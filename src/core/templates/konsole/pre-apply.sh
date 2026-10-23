#!/usr/bin/env bash
# Pre-hook for Konsole PTM template:
# Preserves user customizations (opacity, background blur, wallpaper, etc.)
# by backing up the existing [General] section before PTM writes the new colors.

set -euo pipefail

output="${XDG_DATA_HOME:-$HOME/.local/share}/konsole/ptm.colorscheme"
backup="${output}.general.bak"

# Back up [General] section if the colorscheme exists and contains it
if [ -f "$output" ] && grep -qF '[General]' "$output"; then
    awk '/^\[General\]/{flag=1; print; next} /^\[/{flag=0} flag' "$output" > "$backup"
fi
