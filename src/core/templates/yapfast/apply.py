#!/usr/bin/env python3
"""Apply PTM's palette to Yapfast as a custom theme.

Yapfast keeps its themes inside ``layout.json`` (there is no themes directory),
so the rendered palette is merged into the existing layout instead of replacing
it. All unrelated settings are preserved.
"""

from __future__ import annotations

import json
import os
import re
import stat
import sys
import tempfile
from pathlib import Path

THEME_NAME = "PTM"
THEME_ID = re.compile(r"custom:\d+\Z")
COLOUR = re.compile(r"#[0-9A-Fa-f]{6}(?:[0-9A-Fa-f]{2})?\Z")

REQUIRED_COLOURS = {
    "rail",
    "sidebar",
    "panel",
    "chat",
    "stage",
    "raised",
    "field",
    "sunken",
    "hover",
    "active",
    "row-hover",
    "line",
    "edge",
    "text",
    "secondary",
    "muted",
    "faint",
    "accent",
    "accent-hover",
    "on-accent",
    "accent-soft",
    "danger",
    "danger-hover",
    "on-danger",
    "live",
    "loud",
    "link",
    "button-default",
    "button-default-hover",
    "overlay",
    "shimmer",
    "reply-spine",
    "tooltip",
    "avatar",
}


def fail(message: str) -> int:
    print(f"Yapfast: {message}", file=sys.stderr)
    return 1


def data_home() -> Path:
    return Path(os.environ.get("XDG_DATA_HOME", Path.home() / ".local/share"))


def cache_home() -> Path:
    return Path(os.environ.get("XDG_CACHE_HOME", Path.home() / ".cache"))


def find_layout() -> Path | None:
    override = os.environ.get("YAPFAST_LAYOUT")
    candidates = [Path(override).expanduser()] if override else []
    candidates.append(data_home() / "yapfast/layout.json")
    for path in candidates:
        if path.is_file():
            return path
    return None


def next_id(themes: list[dict]) -> str:
    highest = 0
    for theme in themes:
        match = THEME_ID.fullmatch(str(theme.get("id", "")))
        if match:
            highest = max(highest, int(match.group(0).split(":", 1)[1]))
    return f"custom:{highest + 1}"


def yapfast_running() -> bool:
    proc = Path("/proc")
    if not proc.is_dir():
        return False
    for entry in proc.iterdir():
        if not entry.name.isdigit():
            continue
        try:
            if (entry / "comm").read_text(encoding="utf-8").strip() == "yapfast":
                return True
        except OSError:
            continue
    return False


def write_atomic(path: Path, text: str) -> None:
    mode = stat.S_IMODE(path.stat().st_mode)
    fd, temporary = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
            handle.write(text)
        os.chmod(temporary, mode)
        os.replace(temporary, path)
    finally:
        try:
            os.unlink(temporary)
        except FileNotFoundError:
            pass


def main() -> int:
    palette_path = cache_home() / "ptm/yapfast/theme.json"
    if not palette_path.is_file():
        return fail(f"rendered palette is missing: {palette_path}")

    try:
        palette = json.loads(palette_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return fail(f"could not read rendered palette: {error}")

    if not isinstance(palette, dict):
        return fail("rendered palette is not a JSON object")

    dark = palette.get("dark")
    colours = palette.get("colours")
    if not isinstance(dark, bool) or not isinstance(colours, dict):
        return fail("rendered palette has no dark flag or colours")

    missing = REQUIRED_COLOURS - colours.keys()
    if missing:
        return fail("rendered palette is missing colours: " + ", ".join(sorted(missing)))

    invalid = {
        name: value
        for name, value in colours.items()
        if not isinstance(value, str) or COLOUR.fullmatch(value) is None
    }
    if invalid:
        return fail(f"rendered palette contains invalid colours: {invalid}")

    layout_path = find_layout()
    if layout_path is None:
        return fail(
            "layout.json was not found. Start Yapfast once, or set YAPFAST_LAYOUT "
            "to its path."
        )

    try:
        layout = json.loads(layout_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        return fail(f"could not read {layout_path}: {error}")

    if not isinstance(layout, dict):
        return fail(f"{layout_path} is not a JSON object")

    themes = layout.get("custom_themes")
    if not isinstance(themes, list):
        themes = []
        layout["custom_themes"] = themes

    if any(not isinstance(theme, dict) for theme in themes):
        return fail(f"{layout_path} has a malformed custom_themes entry")

    used = {
        theme.get("id")
        for theme in themes
        if THEME_ID.fullmatch(str(theme.get("id", ""))) is not None
    }
    entry = next(
        (theme for theme in themes if str(theme.get("name", "")).strip().lower() == "ptm"),
        None,
    )

    if entry is None:
        entry = {}
        themes.append(entry)

    theme_id = entry.get("id")
    if not THEME_ID.fullmatch(str(theme_id or "")) or (
        theme_id in used and sum(theme.get("id") == theme_id for theme in themes) > 1
    ):
        theme_id = next_id(themes)
        while theme_id in used:
            theme_id = next_id([{"id": theme_id}] + [{"id": value} for value in used])

    entry.update(
        {
            "id": theme_id,
            "name": THEME_NAME,
            "base": "yap" if dark else "sakura",
            "dark": dark,
            "colours": dict(sorted(colours.items())),
        }
    )
    layout["theme"] = theme_id

    text = json.dumps(layout, ensure_ascii=False, separators=(",", ":"))
    try:
        write_atomic(layout_path, text)
    except OSError as error:
        return fail(f"could not write {layout_path}: {error}")

    print(f"Yapfast: applied the {THEME_NAME} theme ({'dark' if dark else 'light'})")
    if yapfast_running():
        print(
            "Yapfast: it is running, so restart it to see the new theme "
            "(its next save would otherwise overwrite layout.json).",
            file=sys.stderr,
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
