#!/usr/bin/env bash
# Apply the rendered PTM Brave theme and sync browser.theme.color_scheme.
# Never writes Brave Preferences while Brave is running.
set -euo pipefail
IFS=$'\n\t'

log() {
    printf '[%s] ptm-brave: %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*" >&2
}

require_cmd() {
    command -v "$1" >/dev/null 2>&1 || {
        log "ERROR: $1 not found"
        exit 1
    }
}

brave_is_running() {
    pgrep -u "$(id -u)" -x brave >/dev/null 2>&1 \
        || pgrep -u "$(id -u)" -x brave-browser >/dev/null 2>&1
}

xdg_config="${XDG_CONFIG_HOME:-$HOME/.config}"
xdg_cache="${XDG_CACHE_HOME:-$HOME/.cache}"
xdg_data="${XDG_DATA_HOME:-$HOME/.local/share}"

brave_user_data="${xdg_config}/BraveSoftware/Brave-Browser"
brave_profile="${brave_user_data}/Default"
prefs_path="${brave_profile}/Preferences"
secure_prefs_path="${brave_profile}/Secure Preferences"

theme_dir="${xdg_data}/ptm/brave-theme"
backup_dir="${theme_dir}/backups"
prefs_original="${backup_dir}/Preferences.original"
secure_original="${backup_dir}/Secure Preferences.original"
rendered_manifest="${xdg_cache}/ptm/brave-theme/manifest.json"
installed_manifest="${theme_dir}/manifest.json"

usage() {
    cat <<'EOF'
Usage: apply.sh <dark|light>
       apply.sh backup
       apply.sh restore

  dark|light  Install rendered theme files; sync color_scheme only if Brave is stopped
  backup      Snapshot Preferences (and Secure Preferences) once; never overwrites
  restore     Copy the original Preferences snapshot back (Brave must be stopped)
EOF
}

ensure_brave_profile() {
    if [ ! -d "$brave_user_data" ]; then
        log "ERROR: Brave user data not found: $brave_user_data"
        exit 1
    fi
    if [ ! -f "$prefs_path" ]; then
        log "ERROR: Brave Preferences not found: $prefs_path"
        exit 1
    fi
}

# One-time snapshot so the pre-PTM profile can be restored.
backup_prefs() {
    ensure_brave_profile
    mkdir -p "$backup_dir"

    if [ -f "$prefs_original" ]; then
        log "Original Preferences backup already exists: $prefs_original"
    else
        cp -a -- "$prefs_path" "$prefs_original"
        log "Saved original Preferences to $prefs_original"
        if brave_is_running; then
            log "NOTE: Brave is running; this is an on-disk snapshot and may lag in-memory prefs"
        fi
    fi

    if [ -f "$secure_prefs_path" ] && [ ! -f "$secure_original" ]; then
        cp -a -- "$secure_prefs_path" "$secure_original"
        log "Saved original Secure Preferences to $secure_original"
    fi
}

# Color-only Chrome themes often leave frame/toolbar/NTP unpainted on
# Linux. Write solid PNGs next to the manifest so those surfaces fill.
install_theme_files() {
    local mode="$1"

    if [ ! -f "$rendered_manifest" ]; then
        log "ERROR: rendered manifest missing: $rendered_manifest"
        log "Re-apply the PTM theme (or run: ptm theme --list-templates)"
        exit 1
    fi
    require_cmd python3
    python3 -c 'import json,sys; json.load(open(sys.argv[1]))' "$rendered_manifest"

    mkdir -p "$theme_dir/images"
    PTM_THEME_MODE="$mode" \
        RENDERED_MANIFEST="$rendered_manifest" \
        INSTALLED_MANIFEST="$installed_manifest" \
        THEME_DIR="$theme_dir" \
        python3 - <<'PY'
import json
import os
from datetime import datetime, timezone
from pathlib import Path

from PIL import Image

rendered = Path(os.environ["RENDERED_MANIFEST"])
installed = Path(os.environ["INSTALLED_MANIFEST"])
theme_dir = Path(os.environ["THEME_DIR"])
mode = os.environ["PTM_THEME_MODE"]

manifest = json.loads(rendered.read_text())
colors = manifest["theme"]["colors"]
theme = manifest.setdefault("theme", {})
props = theme.setdefault("properties", {})
props["ntp_logo_alternate"] = 1 if mode == "dark" else 0
# Bump so brave://extensions → Update reloads the unpacked theme.
manifest["version"] = datetime.now(timezone.utc).strftime("1.%Y%m%d.%H%M%S")

theme["images"] = {
    "theme_frame": "images/theme_frame.png",
    "theme_frame_inactive": "images/theme_frame_inactive.png",
    "theme_frame_incognito": "images/theme_frame_incognito.png",
    "theme_toolbar": "images/theme_toolbar.png",
    "theme_ntp_background": "images/theme_ntp_background.png",
    "theme_tab_background": "images/theme_tab_background.png",
}

img_dir = theme_dir / "images"
img_dir.mkdir(parents=True, exist_ok=True)


def rgb(name):
    value = colors[name]
    return tuple(int(c) for c in value)


def write_png(name, color, size):
    path = img_dir / name
    Image.new("RGB", size, color).save(path, "PNG")


write_png("theme_frame.png", rgb("frame"), (80, 60))
write_png("theme_frame_inactive.png", rgb("frame_inactive"), (80, 60))
write_png("theme_frame_incognito.png", rgb("frame_incognito"), (80, 60))
write_png("theme_toolbar.png", rgb("toolbar"), (320, 120))
write_png("theme_tab_background.png", rgb("frame"), (64, 64))
write_png("theme_ntp_background.png", rgb("ntp_background"), (1920, 1080))

installed.write_text(json.dumps(manifest, indent=2) + "\n")
print(f"wrote {installed} version={manifest['version']}")
PY
    log "Installed theme + background images in $theme_dir"
}

print_load_unpacked_help() {
    cat <<EOF >&2
ptm-brave: first-time install (click through in Brave):
  1. Quit extra windows if you want, then open: brave://extensions
  2. Enable Developer mode
  3. Load unpacked → ${theme_dir}
  4. Confirm Appearance uses the "PTM" theme
EOF
}

theme_already_loaded() {
    [ -f "$prefs_path" ] || return 1
    python3 - "$prefs_path" "$theme_dir" <<'PY'
import json, sys
from pathlib import Path

data = json.loads(Path(sys.argv[1]).read_text())
theme_dir = str(Path(sys.argv[2]).resolve())
ext_theme = data.get("extensions", {}).get("theme") or {}
pack = str(Path(ext_theme.get("pack", "")).resolve()) if ext_theme.get("pack") else ""
if pack == theme_dir:
    sys.exit(0)
settings = data.get("extensions", {}).get("settings", {})
if isinstance(settings, dict):
    for ext in settings.values():
        if not isinstance(ext, dict):
            continue
        path = ext.get("path") or ""
        if path and Path(path).resolve() == Path(theme_dir):
            sys.exit(0)
        manifest = ext.get("manifest") or {}
        if manifest.get("name") == "PTM" and "theme" in manifest:
            sys.exit(0)
sys.exit(1)
PY
}

# Chromium theme JSON cannot set Brave Settings/Leo accents. The only
# in-profile lever is browser.theme.user_color (Material You seed).
# Written only when Brave is stopped.
sync_color_scheme() {
    local mode="$1"
    require_cmd python3

    if brave_is_running; then
        log "Brave is running: skipped Preferences write"
        log "Settings checkboxes/buttons use Brave Leo purple, not the theme pack"
        log "Quit Brave completely, then re-run: $0 $mode"
        log "That writes color_scheme + user_color (PTM primary seed)"
        return 0
    fi

    local scheme
    if [ "$mode" = "light" ]; then
        scheme=1
    else
        scheme=2
    fi

    local rc=0
    BRAVE_PREFS="$prefs_path" \
        BRAVE_COLOR_SCHEME="$scheme" \
        INSTALLED_MANIFEST="$installed_manifest" \
        python3 - <<'PY' || rc=$?
import json
import os
import stat
from pathlib import Path

prefs = Path(os.environ["BRAVE_PREFS"])
scheme = int(os.environ["BRAVE_COLOR_SCHEME"])
manifest_path = Path(os.environ["INSTALLED_MANIFEST"])
data = json.loads(prefs.read_text())
theme = data.setdefault("browser", {}).setdefault("theme", {})

user_color = theme.get("user_color")
if manifest_path.is_file():
    colors = json.loads(manifest_path.read_text())["theme"]["colors"]
    rgb = colors.get("ntp_link") or colors.get("toolbar_button_icon")
    if isinstance(rgb, list) and len(rgb) >= 3:
        r, g, b = (int(rgb[0]), int(rgb[1]), int(rgb[2]))
        sk = 0xFF000000 | (r << 16) | (g << 8) | b
        if sk >= 2**31:
            sk -= 2**32
        user_color = sk

changed = False
if theme.get("color_scheme") != scheme or theme.get("color_scheme2") != scheme:
    theme["color_scheme"] = scheme
    theme["color_scheme2"] = scheme
    changed = True
if user_color is not None and (
    theme.get("user_color") != user_color or theme.get("user_color2") != user_color
):
    theme["user_color"] = user_color
    theme["user_color2"] = user_color
    changed = True
if not changed:
    raise SystemExit(0)

mode = prefs.stat().st_mode
tmp = prefs.with_name("Preferences.ptm-tmp")
tmp.write_text(json.dumps(data, separators=(",", ":"), ensure_ascii=False))
os.chmod(tmp, stat.S_IMODE(mode))
tmp.replace(prefs)
raise SystemExit(2)
PY
    if [ "$rc" -eq 0 ]; then
        log "color_scheme and user_color already match mode=${mode}"
    elif [ "$rc" -eq 2 ]; then
        log "Set color_scheme=${scheme} and user_color from PTM primary"
    else
        log "ERROR: failed to update Brave theme prefs"
        exit 1
    fi
}

restore_prefs() {
    ensure_brave_profile
    if [ ! -f "$prefs_original" ]; then
        log "ERROR: no original backup at $prefs_original"
        exit 1
    fi
    if brave_is_running; then
        log "ERROR: Brave is running; will not restore Preferences"
        exit 1
    fi

    cp -a -- "$prefs_original" "$prefs_path"
    log "Restored Preferences from $prefs_original"
    if [ -f "$secure_original" ]; then
        cp -a -- "$secure_original" "$secure_prefs_path"
        log "Restored Secure Preferences from $secure_original"
    fi
}

cmd="${1:-}"
case "$cmd" in
    backup)
        backup_prefs
        ;;
    restore)
        restore_prefs
        ;;
    dark | light)
        ensure_brave_profile
        backup_prefs
        install_theme_files "$cmd"
        if theme_already_loaded; then
            log "PTM theme already loaded"
            log "Reload it: brave://extensions → Developer mode → Update"
        else
            print_load_unpacked_help
        fi
        sync_color_scheme "$cmd"
        ;;
    -h | --help | help)
        usage
        ;;
    *)
        usage >&2
        exit 1
        ;;
esac
