#!/usr/bin/env bash
set -euo pipefail

config_dir="${XDG_CONFIG_HOME:-$HOME/.config}"
config_file="$config_dir/fcitx5/conf/classicui.conf"
theme_name="ptm"

mkdir -p "$(dirname "$config_file")"

write_if_changed() {
    local target="$1" tmp="$2"
    if [ ! -e "$target" ]; then
        mv "$tmp" "$target"
        return
    fi
    if ! cmp -s "$target" "$tmp"; then
        cat "$tmp" >"$target"
    fi
    rm -f "$tmp"
}

if [ ! -f "$config_file" ]; then
    echo "Theme=$theme_name" >"$config_file"
elif grep -qE '^Theme=' "$config_file"; then
    if grep -qE "^Theme=$theme_name\$" "$config_file"; then
        :
    else
        tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
        sed -E "s/^Theme=.*/Theme=$theme_name/" "$config_file" >"$tmp_file"
        write_if_changed "$config_file" "$tmp_file"
    fi
else
    [ -s "$config_file" ] && [ -n "$(tail -c1 "$config_file")" ] && echo >>"$config_file"
    echo "Theme=$theme_name" >>"$config_file"
fi

busctl --user call org.fcitx.Fcitx5 /controller org.fcitx.Fcitx.Controller1 ReloadAddonConfig s classicui \
    >/dev/null 2>&1 || true
