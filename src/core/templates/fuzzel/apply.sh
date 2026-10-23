#!/usr/bin/env bash
set -euo pipefail

config_dir="${XDG_CONFIG_HOME:-$HOME/.config}"
config_file="$config_dir/fuzzel/fuzzel.ini"
include_line="include=$config_dir/fuzzel/themes/ptm"

mkdir -p "$(dirname "$config_file")"

write_if_changed() {
    local target="$1" tmp="$2"
    if [ ! -e "$target" ] && [ ! -L "$target" ]; then
        mv "$tmp" "$target"
        return
    fi
    if ! cmp -s "$target" "$tmp"; then
        cat "$tmp" >"$target"
    fi
    rm -f "$tmp"
}

if [ ! -f "$config_file" ]; then
    echo "$include_line" >"$config_file"
elif grep -q "^$include_line$" "$config_file"; then
    :
elif grep -q '^include=.*themes' "$config_file"; then
    tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
    sed 's|^include=.*themes.*|'"$include_line"'|' "$config_file" >"$tmp_file"
    write_if_changed "$config_file" "$tmp_file"
else
    [ -s "$config_file" ] && [ -n "$(tail -c1 "$config_file")" ] && echo >>"$config_file"
    echo "$include_line" >>"$config_file"
fi
