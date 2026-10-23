#!/usr/bin/env bash
set -euo pipefail

bat_config_dir="${XDG_CONFIG_HOME:-$HOME/.config}/bat"
themes_dir="$bat_config_dir/themes"
config_file="$bat_config_dir/config"
theme_file="$themes_dir/ptm.tmTheme"
theme_line="--theme=ptm"

mkdir -p "$themes_dir"
touch "$config_file" "$theme_file"

write_if_changed() {
    local target="$1" tmp="$2"
    if ! cmp -s "$target" "$tmp"; then
        cat "$tmp" >"$target"
    fi
    rm -f "$tmp"
}

if ! grep -Fxq -- "$theme_line" "$config_file"; then
    tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
    sed '/^--theme=/d' "$config_file" >"$tmp_file"
    [ -s "$tmp_file" ] && [ -n "$(tail -c1 "$tmp_file")" ] && echo >>"$tmp_file"
    printf '%s\n' "$theme_line" >>"$tmp_file"
    write_if_changed "$config_file" "$tmp_file"
fi

if command -v bat >/dev/null 2>&1; then
    bat cache --build
elif command -v batcat >/dev/null 2>&1; then
    batcat cache --build
else
    echo "Warning: 'bat' executable not found. Please run 'bat cache --build' manually once it is installed."
fi
