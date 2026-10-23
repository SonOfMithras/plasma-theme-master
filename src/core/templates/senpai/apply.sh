#!/usr/bin/env bash
set -euo pipefail

config_dir_xdg="${XDG_CONFIG_HOME:-$HOME/.config}/senpai"
config_file="$config_dir_xdg/senpai.scfg"
theme_file="$config_dir_xdg/themes/ptm.scfg"

if [ ! -f "$config_file" ]; then
    echo "Error: senpai config not found: $config_file (create it first, this hook only edits an existing config)" >&2
    exit 1
fi

if [ ! -f "$theme_file" ]; then
    echo "Error: senpai theme file not found: $theme_file" >&2
    exit 1
fi

theme_block="$(cat "$theme_file")"
tmp_file="$(mktemp)"
trap 'rm -f "$tmp_file"' EXIT

awk -v theme_data="$theme_block" '
BEGIN { in_colors = 0; depth = 0; saw_colors = 0 }
{
    if (!in_colors && $0 ~ /^[[:space:]]*colors[[:space:]]*\{[[:space:]]*$/) {
        in_colors = 1
        saw_colors = 1
        depth = 1
        print theme_data
        next
    }
    if (in_colors) {
        n = gsub(/\{/, "{"); depth += n
        n = gsub(/\}/, "}"); depth -= n
        if (depth <= 0) { in_colors = 0 }
        next
    }
    print
}
END {
    if (!saw_colors) {
        print ""
        print theme_data
    }
}
' "$config_file" >"$tmp_file"

if ! cmp -s "$config_file" "$tmp_file"; then
    cat "$tmp_file" >"$config_file"
fi
