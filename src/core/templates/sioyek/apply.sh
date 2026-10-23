#!/usr/bin/env bash

set -euo pipefail

config_dir="${XDG_CONFIG_HOME:-$HOME/.config}/sioyek"
file="$config_dir/themes/ptm.config"
prefs="$config_dir/prefs_user.config"
tmp="${file}.tmp"

[[ -f "$prefs" ]] || {
    echo "Sioyek config not found: $prefs" >&2
    exit 1
}

# Convert PTM hex colors to Sioyek's 0-1 RGB format.
while IFS= read -r line || [[ -n "$line" ]]; do
    if [[ "$line" =~ ^([a-z_]+)[[:space:]]+#([0-9A-Fa-f]{6})$ ]]; then
        name="${BASH_REMATCH[1]}"
        hex="${BASH_REMATCH[2]}"

        r=$((16#${hex:0:2}))
        g=$((16#${hex:2:2}))
        b=$((16#${hex:4:2}))

        rf=$(awk "BEGIN { printf \"%.6f\", $r / 255 }")
        gf=$(awk "BEGIN { printf \"%.6f\", $g / 255 }")
        bf=$(awk "BEGIN { printf \"%.6f\", $b / 255 }")

        printf '%s %s %s %s\n' "$name" "$rf" "$gf" "$bf"
    else
        printf '%s\n' "$line"
    fi
done < "$file" > "$tmp"

mv "$tmp" "$file"

# Make sure Sioyek loads the generated theme.
grep -Eq '^source[[:space:]]+.*sioyek/themes/ptm\.config$' "$prefs" || \
    printf '\nsource %s\n' "$file" >> "$prefs"
