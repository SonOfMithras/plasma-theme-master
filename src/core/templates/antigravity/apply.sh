#!/usr/bin/env bash
set -euo pipefail

config_file="${HOME}/.gemini/config/config.json"
theme_file="${HOME}/.gemini/config/theme.json"

if [ -f "$config_file" ]; then
    tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
    jq -s '
        .[0].userSettings.customThemeSeedsDark = .[1].userSettings.customThemeSeedsDark
        | .[0].userSettings.customThemeSeedsLight = .[1].userSettings.customThemeSeedsLight
        | .[0]
    ' "$config_file" "$theme_file" >"$tmp_file"
    if ! cmp -s "$config_file" "$tmp_file"; then
        cat "$tmp_file" >"$config_file"
    fi
    rm -f "$tmp_file"
else
    cp "$theme_file" "$config_file"
fi
