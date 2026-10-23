#!/usr/bin/env bash
set -euo pipefail

# Match the previous lookup (XDG_CACHE_HOME with a .config fallback).
config_dir="${XDG_CACHE_HOME:-$HOME/.config}"

if [[ ! -f "$config_dir/snappy-switcher/config.ini" ]]; then
    snappy-install-config
fi

config_file="$config_dir/snappy-switcher/config.ini"
tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
sed 's/name = snappy-slate.ini/name = ptm.ini/' "$config_file" >"$tmp_file"
if ! cmp -s "$config_file" "$tmp_file"; then
    cat "$tmp_file" >"$config_file"
fi
rm -f "$tmp_file"

snappy-switcher quit
snappy-switcher --daemon & disown
echo done!
