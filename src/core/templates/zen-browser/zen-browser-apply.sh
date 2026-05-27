#!/usr/bin/env bash
# zen-browser-apply.sh
# Discovers native and Flatpak Zen Browser profiles, and imports userChrome.css/userContent.css.

set -euo pipefail

css_chrome="$HOME/.config/plasma-theme-master/zen-browser/userChrome.css"
css_content="$HOME/.config/plasma-theme-master/zen-browser/userContent.css"
line_chrome="@import \"$css_chrome\";"
line_content="@import \"$css_content\";"

# Common base directories for Zen Browser on Linux
# 1. ~/.zen (Standard native path)
# 2. ~/.config/zen (Alternative native config path)
# 3. ~/.var/app/app.zen_browser.zen/.zen (Flatpak path)
for base_dir in "$HOME/.zen" "$HOME/.config/zen" "$HOME/.var/app/app.zen_browser.zen/.zen"; do
    if [ -d "$base_dir" ]; then
        find "$base_dir" -mindepth 2 -maxdepth 2 -type d -name chrome -print0 2>/dev/null |
            while IFS= read -r -d '' dir; do
                user_chrome="$dir/userChrome.css"
                user_content="$dir/userContent.css"
                mkdir -p "$dir"
                touch "$user_chrome" "$user_content"
                
                # Clean up legacy import lines to keep files tidy and prevent stale themes
                sed -i '/noctalia\/zen-browser/d' "$user_chrome" 2>/dev/null || true
                sed -i '/noctalia\/zen-browser/d' "$user_content" 2>/dev/null || true
                sed -i '/plasma-theme-master\/zen-browser/d' "$user_chrome" 2>/dev/null || true
                sed -i '/plasma-theme-master\/zen-browser/d' "$user_content" 2>/dev/null || true
                
                # Append correct import lines if they are not already there
                if ! grep -Fq "$line_chrome" "$user_chrome" 2>/dev/null; then
                    printf '%s\n' "$line_chrome" >> "$user_chrome"
                fi
                if ! grep -Fq "$line_content" "$user_content" 2>/dev/null; then
                    printf '%s\n' "$line_content" >> "$user_content"
                fi
            done
    fi
done
