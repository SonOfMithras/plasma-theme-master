#!/usr/bin/env bash
# Undo hook for Zen Browser: strips PTM @import rules from userChrome.css & userContent.css
set -euo pipefail

zen_dirs=()
for d in "${XDG_CONFIG_HOME:-$HOME/.config}/zen" "$HOME/.zen" \
         "$HOME/.var/app/app.zen_browser.zen/.zen" \
         "$HOME/.var/app/io.github.zen_browser.zen/.zen"; do
    [ -d "$d" ] && zen_dirs+=("$d")
done
[ "${#zen_dirs[@]}" -eq 0 ] && exit 0

rm -rf "${XDG_CONFIG_HOME:-$HOME/.config}/plasma-theme-master/zen-browser" 2>/dev/null || true

find "${zen_dirs[@]}" -mindepth 2 -maxdepth 2 -type f -name "prefs.js" -print0 |
    while IFS= read -r -d '' prefs_file; do
        profile_dir=$(dirname "$prefs_file")
        chrome_dir="$profile_dir/chrome"
        user_chrome="$chrome_dir/userChrome.css"
        user_content="$chrome_dir/userContent.css"

        if [ -f "$user_chrome" ]; then
            sed -i -E '/(zen-browser\/(zen-)?userChrome\.css|plasma-theme-master\/zen-browser|plasma-colors\.css|ptm-userChrome|noctalia)/d' "$user_chrome"
            [ ! -s "$user_chrome" ] && rm -f "$user_chrome"
        fi

        if [ -f "$user_content" ]; then
            sed -i -E '/(zen-browser\/(zen-)?userContent\.css|plasma-theme-master\/zen-browser|plasma-colors\.css|ptm-userContent|noctalia)/d' "$user_content"
            [ ! -s "$user_content" ] && rm -f "$user_content"
        fi
    done
