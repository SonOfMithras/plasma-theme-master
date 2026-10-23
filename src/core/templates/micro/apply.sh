#!/usr/bin/env bash
set -euo pipefail

config_home="${XDG_CONFIG_HOME:-$HOME/.config}"
json_file="$config_home/micro/settings.json"

mkdir -p "$(dirname "$json_file")"

if [ ! -f "$json_file" ] || [ ! -s "$json_file" ]; then
    echo '{"colorscheme": "ptm"}' > "$json_file"
    exit 0
fi

tmp_file="$(mktemp "${json_file}.tmp.XXXXXX")"

if jq '.colorscheme = "ptm"' "$json_file" >"$tmp_file" 2>/dev/null; then
    if ! cmp -s "$json_file" "$tmp_file"; then
        cat "$tmp_file" > "$json_file"
    fi
else
    # Backup corrupt JSON and create fresh
    mv "$json_file" "$json_file.bak.$(date +%s)"
    echo '{"colorscheme": "ptm"}' > "$json_file"
fi

rm -f "$tmp_file"
