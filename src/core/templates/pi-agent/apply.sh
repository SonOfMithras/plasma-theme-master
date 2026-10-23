#!/usr/bin/env bash
set -euo pipefail

config_file="$HOME/.pi/agent/settings.json"

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
    echo '{"theme": "ptm"}' >"$config_file"
    exit 0
fi

tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
if grep -q '"theme"' "$config_file"; then
    sed 's/"theme"[[:space:]]*:[[:space:]]*"[^"]*"/"theme": "ptm"/' "$config_file" >"$tmp_file"
else
    sed '1s/{/{\n  "theme": "ptm",/' "$config_file" >"$tmp_file"
fi
write_if_changed "$config_file" "$tmp_file"
