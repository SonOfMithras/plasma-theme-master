#!/usr/bin/env python3
"""Apply PTM colors to Archetype's existing color constants."""

from __future__ import annotations

import json
import os
import re
import stat
import sys
import tempfile
from pathlib import Path

PALETTE = (
    Path(os.environ.get("XDG_CACHE_HOME", Path.home() / ".cache"))
    / "ptm/archetype-pokemmo/colors.json"
)

COLOR_RE = re.compile(
    r'(?P<prefix><constantDef\s+name=["\'](?P<name>[^"\']+)["\']\s*>\s*<color>)'
    r'(?P<value>#[0-9A-Fa-f]{6,8})'
    r'(?P<suffix></color>\s*</constantDef>)'
)

REQUIRED = {
    "main-color",
    "sub-color",
    "button-color",
    "accent-color",
    "font-main-color",
}


def fail(message: str) -> int:
    print(f"Archetype (PokeMMO): {message}", file=sys.stderr)
    return 1


def find_target() -> Path | None:
    override = os.environ.get("POKEMMO_ARCHETYPE_COLORS_FILE")
    candidates = []

    if override:
        candidates.append(Path(override).expanduser())

    candidates += [
        Path.home()
        / ".var/app/com.pokemmo.PokeMMO/data/pokemmo-client-live"
        / "data/mods/archetype-theme/theme/CHOOSE_YOUR_COLORS.xml",
        Path.home()
        / "PokeMMO/data/mods/archetype-theme/theme/CHOOSE_YOUR_COLORS.xml",
        Path.home()
        / ".local/share/PokeMMO/data/mods/archetype-theme/theme/CHOOSE_YOUR_COLORS.xml",
    ]

    for path in candidates:
        if path.is_file():
            return path

    return None


def write_atomic(path: Path, text: str) -> None:
    mode = stat.S_IMODE(path.stat().st_mode)
    fd, temp_name = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
        os.chmod(temp_name, mode)
        os.replace(temp_name, path)
    finally:
        try:
            os.unlink(temp_name)
        except FileNotFoundError:
            pass


def main() -> int:
    if not PALETTE.is_file():
        return fail(f"rendered palette is missing: {PALETTE}")

    try:
        colors = json.loads(PALETTE.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return fail(f"could not read rendered palette: {error}")

    if not isinstance(colors, dict):
        return fail("rendered palette is not a JSON object")

    invalid = {
        name: value
        for name, value in colors.items()
        if not isinstance(value, str)
        or re.fullmatch(r"#[0-9A-Fa-f]{6,8}", value) is None
    }
    if invalid:
        return fail(f"rendered palette contains invalid color values: {invalid}")

    target = find_target()
    if target is None:
        return fail(
            "Archetype CHOOSE_YOUR_COLORS.xml was not found. "
            "Install Archetype, or set POKEMMO_ARCHETYPE_COLORS_FILE."
        )

    try:
        source = target.read_text(encoding="utf-8")
    except OSError as error:
        return fail(f"could not read {target}: {error}")

    present = {match.group("name") for match in COLOR_RE.finditer(source)}
    missing = REQUIRED - present
    if missing:
        return fail(
            "Archetype's color file has an unexpected structure; missing: "
            + ", ".join(sorted(missing))
        )

    changed = 0

    def replace(match: re.Match[str]) -> str:
        nonlocal changed
        name = match.group("name")
        value = colors.get(name)
        if value is None:
            return match.group(0)
        if value.lower() != match.group("value").lower():
            changed += 1
        return match.group("prefix") + value + match.group("suffix")

    rendered = COLOR_RE.sub(replace, source)

    if changed:
        try:
            write_atomic(target, rendered)
        except OSError as error:
            return fail(f"could not update {target}: {error}")

    print(f"Archetype (PokeMMO): synced {changed} color value(s).")
    if changed:
        print("If PokeMMO is open, press Ctrl+F5 in PokeMMO to reload Archetype.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
