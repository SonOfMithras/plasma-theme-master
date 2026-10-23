#!/usr/bin/env bash
set -euo pipefail

icon_source="${1:-}"
icon_target="${XDG_DATA_HOME:-$HOME/.local/share}/vicinae/themes/ptm.svg"

if [ -n "$icon_source" ] && [ -f "$icon_source" ]; then
    mkdir -p "$(dirname "$icon_target")"
    cp --update=none "$icon_source" "$icon_target"
fi

vicinae theme set ptm
