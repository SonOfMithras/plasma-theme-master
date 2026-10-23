#!/usr/bin/env bash
set -euo pipefail

config_home="${XDG_CONFIG_HOME:-$HOME/.config}"
json_file="$config_home/YouTube Music/config.json"
theme_path="$config_home/YouTube Music/ptm.css"

if [ ! -f "$json_file" ]; then
    exit 0
fi

tmp_file="$(mktemp "${json_file}.tmp.XXXXXX")"
jq --arg new_theme "$theme_path" '.options.themes = [$new_theme]' "$json_file" >"$tmp_file"
if ! cmp -s "$json_file" "$tmp_file"; then
    cat "$tmp_file" >"$json_file"
fi
rm -f "$tmp_file"
