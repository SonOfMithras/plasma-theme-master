#!/usr/bin/env python3
"""Import an OMP layout once; recolor its copy with PTM's terminal palette."""

import argparse
import colorsys
import copy
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile


ANSI = (
    "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white",
    "darkGray", "lightRed", "lightGreen", "lightYellow", "lightBlue",
    "lightMagenta", "lightCyan", "lightWhite",
)
ROLES = (*ANSI, "foreground", "background")
NEUTRALS = {"black", "white", "darkGray", "lightWhite", "foreground", "background"}
HEX = re.compile(r"#(?:[0-9a-fA-F]{6}|[0-9a-fA-F]{3})\Z")
FUNCTION = re.compile(r"(linear-gradient|dark-gradient|light-gradient|darken|lighten)\((.*)\)\Z", re.DOTALL)
SPECIAL = {"default", "transparent", "parentBackground", "parentForeground", "background", "foreground", "accent"}
COLOR_FIELDS = {"foreground", "background", "accent_color", "color"}
MARKUP_FIELDS = {"template", "templates", "filler", "leading_diamond", "trailing_diamond", "powerline_symbol"}


def split_go(text):
    """Yield literal output and Go actions in one forward pass."""
    index = 0
    while index < len(text):
        start = text.find("{{", index)
        if start < 0:
            yield text[index:]
            return
        yield text[index:start]
        index = start + 2
        quote = None
        while index < len(text):
            char = text[index]
            # Closing delimiters inside Go strings and comments are still code.
            if quote:
                if char == "\\" and quote != "`":
                    index += 2
                    continue
                if char == quote:
                    quote = None
            elif char in "\"'`":
                quote = char
            elif text.startswith("/*", index):
                end = text.find("*/", index + 2)
                if end < 0:
                    index = len(text)
                    break
                index = end + 2
                continue
            elif text.startswith("}}", index):
                index += 2
                break
            index += 1
        # Preserve unfinished actions through EOF without retrying later openers.
        yield text[start:index]


def rgb(value):
    if not isinstance(value, str) or not HEX.fullmatch(value):
        return None
    value = value[1:]
    if len(value) == 3:
        value = "".join(char * 2 for char in value)
    return tuple(int(value[i:i + 2], 16) / 255 for i in (0, 2, 4))


def color_role(value):
    """Classify source colors independently of the current target palette."""
    if value in ANSI:
        return value
    if value.isdecimal() and 0 <= int(value) <= 255:
        index = int(value)
        if index < 16:
            return ANSI[index]
        if index >= 232:
            channels = ((8 + (index - 232) * 10) / 255,) * 3
        else:
            index -= 16
            levels = (0, 95, 135, 175, 215, 255)
            channels = tuple(levels[n] / 255 for n in (index // 36, index // 6 % 6, index % 6))
    else:
        channels = rgb(value)
    if channels is None:
        return None
    hue, lightness, _ = colorsys.rgb_to_hls(*channels)
    if max(channels) - min(channels) <= 0.16:
        if lightness < 0.25:
            return "background"
        if lightness > 0.8:
            return "foreground"
        return "darkGray" if lightness < 0.6 else "white"
    family = ("red", "yellow", "green", "cyan", "blue", "magenta")[int(hue * 6 + 0.5) % 6]
    return "light" + family.title() if lightness > 0.6 else family


def split_arguments(text):
    parts, start, depth = [], 0, 0
    for index, char in enumerate(text):
        if char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
        elif char == "," and depth == 0:
            parts.append(text[start:index])
            start = index + 1
    return [*parts, text[start:]]


def luminance(color):
    channels = rgb(color)
    linear = [c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4 for c in channels]
    return sum(c * weight for c, weight in zip(linear, (0.2126, 0.7152, 0.0722)))


def contrast(first, second):
    low, high = sorted((luminance(first), luminance(second)))
    return (high + 0.05) / (low + 0.05)


def resolve_color(value, palette):
    seen = set()
    while isinstance(value, str) and value.startswith("p:"):
        key = value[2:]
        if key in seen:
            return None
        seen.add(key)
        value = palette.get(key)
    return value if isinstance(value, str) else None


class Recolor:
    def __init__(self, palette, overrides=None):
        self.palette = palette
        self.overrides = {key.lower(): value for key, value in (overrides or {}).items()}
        self.mapping = {}
        self.contrast_fixes = 0

    def role(self, value):
        return self.overrides.get(value.lower()) or color_role(value)

    def literal(self, value):
        stripped = value.strip()
        role = self.role(stripped)
        if role:
            self.mapping[stripped] = role
            return value.replace(stripped, self.palette[role], 1)
        match = FUNCTION.fullmatch(stripped)
        if match:
            name, arguments = match.groups()
            parts = split_arguments(arguments)
            # Shade percentages are numbers, not indexed ANSI colors.
            count = 1 if name in {"darken", "lighten"} else len(parts)
            parts[:count] = [self.literal(part) for part in parts[:count]]
            return value.replace(stripped, f"{name}({','.join(parts)})", 1)
        return value

    def color(self, value):
        if isinstance(value, list):
            return [self.color(item) for item in value]
        if not isinstance(value, str):
            return value
        # Go actions may contain comparisons, printf, or arbitrary string data.
        # Recolor only their literal output; do not rewrite executable expressions.
        return "".join(part if part.startswith("{{") else self.literal(part) for part in split_go(value))

    def readable(self, original, foreground, background):
        if not original or self.role(original) not in NEUTRALS:
            return foreground
        if rgb(foreground) is None or rgb(background) is None or contrast(foreground, background) >= 4.5:
            return foreground
        best = max((self.palette[role] for role in sorted(NEUTRALS)), key=lambda color: contrast(color, background))
        if contrast(best, background) > contrast(foreground, background):
            self.contrast_fixes += 1
            return best
        return foreground

    def markup(self, value):
        if isinstance(value, list):
            return [self.markup(item) for item in value]
        if not isinstance(value, str):
            return value

        def replace(match):
            colors = split_arguments(match.group(1))
            if len(colors) > 2 or not any(colors):
                return match.group(0)
            if not all(not c or c in SPECIAL or c.startswith("p:") or self.role(c) or FUNCTION.fullmatch(c) for c in colors):
                return match.group(0)
            mapped = [self.literal(c) for c in colors]
            if len(colors) == 2:
                mapped[0] = self.readable(colors[0], mapped[0], mapped[1])
            return f"<{','.join(mapped)}>"

        return "".join(part if part.startswith("{{") else re.sub(r"<([^<>]*)>", replace, part) for part in split_go(value))

    def walk(self, value):
        if isinstance(value, list):
            return [self.walk(item) for item in value]
        if not isinstance(value, dict):
            return value
        result = {}
        for key, item in value.items():
            if key == "palette" and isinstance(item, dict):
                result[key] = {name: self.color(color) for name, color in item.items()}
            elif key == "palettes" and isinstance(item, dict):
                result[key] = copy.deepcopy(item)
                result[key]["list"] = {
                    name: {slot: self.color(color) for slot, color in colors.items()}
                    for name, colors in item.get("list", {}).items()
                }
            elif key == "terminal_background":
                result[key] = self.palette["background"]
            elif key in COLOR_FIELDS or key.endswith("_color") or key in {"foreground_templates", "background_templates"}:
                result[key] = self.color(item)
            elif key in MARKUP_FIELDS or key.endswith("_template"):
                result[key] = self.markup(item)
            else:
                result[key] = self.walk(item)
        return result

    def convert(self, source):
        result = self.walk(source)
        # Runtime-selected palettes cannot be resolved statically. Only correct
        # neutral text on known backgrounds; colored status indicators keep their hue.
        if not source.get("palettes"):
            original_palette = source.get("palette", {})
            mapped_palette = result.get("palette", {})

            def correct(original, mapped):
                if isinstance(original, list):
                    for old, new in zip(original, mapped):
                        correct(old, new)
                elif isinstance(original, dict):
                    if "foreground" in original and not any(original.get(key) for key in ("foreground_templates", "background_templates")):
                        old_fg = resolve_color(original["foreground"], original_palette)
                        new_fg = resolve_color(mapped["foreground"], mapped_palette)
                        new_bg = resolve_color(mapped.get("background", "transparent"), mapped_palette)
                        if new_bg in {"transparent", "default"}:
                            new_bg = self.palette["background"]
                        corrected = self.readable(old_fg, new_fg, new_bg)
                        if corrected != new_fg:
                            mapped["foreground"] = corrected
                    for key, item in original.items():
                        correct(item, mapped[key])

            correct(source, result)
        return result


def json_bytes(value):
    return (json.dumps(value, ensure_ascii=True, indent=2) + "\n").encode("utf-8")


def write_json(path, value, *, exclusive=False):
    content = json_bytes(value)
    if not exclusive and path.is_file() and path.read_bytes() == content:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(mode="wb", dir=path.parent, delete=False) as handle:
            temporary = Path(handle.name)
            handle.write(content)
            handle.flush()
            os.fsync(handle.fileno())
        if exclusive:
            os.link(temporary, path)  # Publish atomically without replacing a new arrival.
        else:
            temporary.replace(path)
    finally:
        if temporary is not None and temporary.exists():
            temporary.unlink()
    return True


def load_json(path):
    return json.loads(path.read_text(encoding="utf-8"))


def write_prompt(path, value):
    """Replace only an unchanged output previously generated by this hook."""
    state_path = path.with_name("ptm-output-state.json")
    # Lock the directory itself so concurrent hooks need no additional lock file.
    lock = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY)
    try:
        fcntl.flock(lock, fcntl.LOCK_EX)
        for candidate in (path, state_path):
            if candidate.is_symlink() or (candidate.exists() and not candidate.is_file()):
                raise ValueError(f"Refusing to replace {candidate}: expected a regular file. Move it aside first.")
        state_exists = state_path.exists()
        state = load_json(state_path) if state_exists else {}
        hashes = state.get("sha256") if isinstance(state, dict) else None
        if state_exists and (not isinstance(state, dict) or state.get("generator") != "ptm-oh-my-posh-v1"
                             or not isinstance(hashes, list) or not 1 <= len(hashes) <= 2
                             or any(not isinstance(h, str) or not re.fullmatch(r"[0-9a-f]{64}", h) for h in hashes)):
            raise ValueError(f"Unrecognized ownership state: {state_path}. Move it and the output aside first.")
        previous = path.read_bytes() if path.exists() else None
        previous_hash = hashlib.sha256(previous).hexdigest() if previous is not None else None
        if previous is not None and previous_hash not in (hashes or []):
            raise ValueError(f"Refusing to overwrite {path}: it is unmanaged or was edited. "
                             "Keep edits in ptm-source.omp.json, then move the output aside and apply again.")
        digest = hashlib.sha256(json_bytes(value)).hexdigest()
        final_state = {"generator": "ptm-oh-my-posh-v1", "sha256": [digest]}
        # Record both versions before replacing the prompt. An interrupted update
        # can resume safely whether it left the old or the new output on disk.
        pending = dict(final_state, sha256=list(dict.fromkeys(h for h in (previous_hash, digest) if h)))
        write_json(state_path, pending, exclusive=not state_exists)
        changed = write_json(path, value, exclusive=previous is None)
        write_json(state_path, final_state)
        return changed
    finally:
        os.close(lock)


def import_config(directory, config, replace):
    if not config:
        raise ValueError("Run import from an initialized terminal, or pass --config /path/to/theme.omp.json.")
    source = Path(os.path.expandvars(config)).expanduser().resolve()
    if not source.is_file():
        raise ValueError(f"OMP config is not a local file: {source}")
    if source == (directory / "ptm.omp.json").resolve():
        raise ValueError("Import the original theme, not PTM's generated output.")
    target = directory / "ptm-source.omp.json"
    if (target.exists() or target.is_symlink()) and not replace:
        raise ValueError(f"{target} already exists; use --replace to import a different theme.")
    exported = subprocess.run(
        ["oh-my-posh", "config", "export", "--config", str(source), "--format", "json"],
        check=True, capture_output=True, text=True, timeout=30,
    )
    normalized = json.loads(exported.stdout)
    if not isinstance(normalized, dict) or not normalized.get("blocks") or normalized.get("extends"):
        raise ValueError("Expected an exported OMP config with blocks and fully resolved inheritance.")
    # OMP can exit successfully after substituting its fallback error prompt.
    errors = {"CONFIG ERROR", "CONFIG PARSE ERROR", "CONFIG LOAD ERROR"}
    if any(segment.get("template", "").strip() in errors
           for block in normalized["blocks"] for segment in block.get("segments", [])):
        raise ValueError(f"Oh My Posh could not load {source}; the imported source was not changed.")
    write_json(target, normalized, exclusive=not replace)
    print(f"Imported {source} into {target}")


def apply(directory, dry_run):
    source_path = directory / "ptm-source.omp.json"
    if not source_path.is_file():
        raise ValueError("Import your OMP config first: python3 apply.py import --config /path/to/theme.omp.json")
    source = load_json(source_path)
    if not isinstance(source, dict) or not source.get("blocks") or source.get("extends"):
        raise ValueError("Re-import the source config: expected blocks with resolved inheritance.")
    palette = load_json(directory / "ptm-palette.json")
    if not isinstance(palette, dict) or any(rgb(palette.get(role)) is None for role in ROLES):
        raise ValueError("PTM has not rendered all 18 terminal colors; apply the template first.")
    overrides_path = directory / "ptm-map.json"
    overrides = load_json(overrides_path) if overrides_path.exists() else {}
    if not isinstance(overrides, dict) or any(not isinstance(key, str) or value not in ROLES for key, value in overrides.items()):
        raise ValueError("ptm-map.json must map source color strings to terminal color roles.")
    converter = Recolor(palette, overrides)
    output = converter.convert(source)
    if dry_run:
        print(json.dumps({"colors": converter.mapping, "contrast_adjustments": converter.contrast_fixes}, indent=2))
    else:
        target = directory / "ptm.omp.json"
        changed = write_prompt(target, output)
        print(f"{'Updated' if changed else 'Unchanged'} {target}; {len(converter.mapping)} colors mapped, {converter.contrast_fixes} contrast adjustments")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    default_config = Path(os.environ.get("XDG_CONFIG_HOME") or Path.home() / ".config")
    parser.add_argument("--directory", type=Path, default=default_config / "oh-my-posh", help="OMP integration directory (default: $XDG_CONFIG_HOME/oh-my-posh)")
    commands = parser.add_subparsers(dest="command", required=True)
    importer = commands.add_parser("import", help="Capture the current OMP config as an editable source copy")
    importer.add_argument("--config", default=os.environ.get("POSH_THEME"), help="Original OMP config (default: $POSH_THEME)")
    importer.add_argument("--replace", action="store_true", help="Replace the previously imported source copy")
    applier = commands.add_parser("apply", help="Recolor the imported source using PTM's rendered palette")
    applier.add_argument("--dry-run", action="store_true", help="Show inferred color roles without writing the prompt")
    args = parser.parse_args()
    try:
        if args.command == "import":
            import_config(args.directory.expanduser(), args.config, args.replace)
        else:
            apply(args.directory.expanduser(), args.dry_run)
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f"oh-my-posh template: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
