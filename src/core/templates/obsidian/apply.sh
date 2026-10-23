#!/usr/bin/bash
set -euo pipefail

# Discover Obsidian vaults and configure snippet + baseTheme.
snippet_name="ptm"

find_vaults() {
    # 1. Fast discovery from Obsidian's official config
    local conf="$HOME/.config/obsidian/obsidian.json"
    if [ -f "$conf" ]; then
        python3 -c "
import json, os
try:
    with open('$conf', 'r', encoding='utf-8') as f:
        data = json.load(f)
    for v in data.get('vaults', {}).values():
        p = v.get('path')
        if p and os.path.isdir(os.path.join(p, '.obsidian')):
            print(os.path.join(p, '.obsidian'))
except Exception:
    pass
" 2>/dev/null && return 0
    fi

    # 2. Fallback: Search common locations (Documents, Home) without recursing deeply
    local search_dirs=("$HOME/Documents" "$HOME")
    for dir in "${search_dirs[@]}"; do
        [ -d "$dir" ] || continue
        find "$dir" -maxdepth 3 -name ".obsidian" -type d 2>/dev/null || true
    done | sort -u
}

case "${1:-}" in
    output)
        # Called by ptm to determine output paths.
        find_vaults | while read -r obsidian_dir; do
            snippets_dir="$obsidian_dir/snippets"
            mkdir -p "$snippets_dir"
            echo "$snippets_dir/$snippet_name.css"
        done
        ;;
    apply)
        # Called after rendering. Enable snippet and sync baseTheme in each vault's appearance.json.
        target_mode="${2:-dark}"
        if [ "$target_mode" != "light" ] && [ "$target_mode" != "dark" ]; then
            target_mode="dark"
        fi

        find_vaults | while read -r obsidian_dir; do
            [ -d "$obsidian_dir" ] || continue
            appearance_file="$obsidian_dir/appearance.json"

            python3 -c "
import json, os

app_file = '$appearance_file'
mode = '$target_mode'
snippet = '$snippet_name'

data = {}
if os.path.exists(app_file):
    try:
        with open(app_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception:
        data = {}

# Ensure ptm snippet is in enabledCssSnippets
snippets = data.get('enabledCssSnippets', [])
if snippet not in snippets:
    snippets.append(snippet)
data['enabledCssSnippets'] = snippets

# Explicitly synchronize baseTheme with PTM mode so Obsidian never gets stuck
data['baseTheme'] = mode

try:
    with open(app_file, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2)
        f.write('\n')
except Exception:
    pass
" 2>/dev/null || true
        done
        ;;
    *)
        echo "Usage: $0 {output|apply [dark|light]}" >&2
        exit 1
        ;;
esac
