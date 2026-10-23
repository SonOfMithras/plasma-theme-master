#!/usr/bin/env bash
set -euo pipefail

config_file="${XDG_CONFIG_HOME:-$HOME/.config}/walker/config.toml"

if [ ! -f "$config_file" ]; then
    echo "Error: walker config file not found at $config_file" >&2
    exit 1
fi

write_if_changed() {
    local target="$1" tmp="$2"
    if ! cmp -s "$target" "$tmp"; then
        cat "$tmp" >"$target"
    fi
    rm -f "$tmp"
}

if grep -qE '^theme\s*=\s*"ptm"' "$config_file"; then
    :
elif grep -qE '^theme\s*=' "$config_file"; then
    tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
    sed -E 's/^theme\s*=.*/theme = "ptm"/' "$config_file" >"$tmp_file"
    write_if_changed "$config_file" "$tmp_file"
else
    [ -s "$config_file" ] && [ -n "$(tail -c1 "$config_file")" ] && echo >>"$config_file"
    echo 'theme = "ptm"' >>"$config_file"
fi
