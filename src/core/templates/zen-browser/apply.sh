#!/usr/bin/env bash
set -euo pipefail

cache_dir="${XDG_CACHE_HOME:-$HOME/.cache}"
css_chrome="$cache_dir/ptm/zen-browser/zen-userChrome.css"
css_content="$cache_dir/ptm/zen-browser/zen-userContent.css"
line_chrome="@import \"$css_chrome\";"
line_content="@import \"$css_content\";"

write_if_changed() {
    local target="$1" tmp="$2"
    if ! cmp -s "$target" "$tmp"; then
        cat "$tmp" >"$target"
    fi
    rm -f "$tmp"
}

zen_dirs=()
for d in "${XDG_CONFIG_HOME:-$HOME/.config}/zen" "$HOME/.zen" \
         "$HOME/.var/app/app.zen_browser.zen/.zen" \
         "$HOME/.var/app/io.github.zen_browser.zen/.zen"; do
    [ -d "$d" ] && zen_dirs+=("$d")
done
[ "${#zen_dirs[@]}" -eq 0 ] && exit 0

# Clean up vestigial legacy directory if present
rm -rf "${XDG_CONFIG_HOME:-$HOME/.config}/plasma-theme-master/zen-browser" 2>/dev/null || true

find "${zen_dirs[@]}" -mindepth 2 -maxdepth 2 -type f -name "prefs.js" -print0 |
    while IFS= read -r -d '' prefs_file; do
        profile_dir=$(dirname "$prefs_file")
        chrome_dir="$profile_dir/chrome"
        user_chrome="$chrome_dir/userChrome.css"
        user_content="$chrome_dir/userContent.css"
        user_js="$profile_dir/user.js"

        mkdir -p "$chrome_dir"
        touch "$user_chrome" "$user_content" "$user_js"

        # @import must come before any other CSS rule, so prepend it
        tmp_chrome="$(mktemp "${user_chrome}.tmp.XXXXXX")"
        printf '%s\n' "$line_chrome" >"$tmp_chrome"
        sed -E '/(zen-browser\/(zen-)?userChrome\.css|plasma-theme-master\/zen-browser|plasma-colors\.css|ptm-userChrome|noctalia)/d' "$user_chrome" >>"$tmp_chrome"
        write_if_changed "$user_chrome" "$tmp_chrome"

        tmp_content="$(mktemp "${user_content}.tmp.XXXXXX")"
        printf '%s\n' "$line_content" >"$tmp_content"
        sed -E '/(zen-browser\/(zen-)?userContent\.css|plasma-theme-master\/zen-browser|plasma-colors\.css|ptm-userContent|noctalia)/d' "$user_content" >>"$tmp_content"
        write_if_changed "$user_content" "$tmp_content"

        tmp_js="$(mktemp "${user_js}.tmp.XXXXXX")"
        sed \
            -e '/toolkit\.legacyUserProfileCustomizations\.stylesheets/d' \
            -e '/devtools\.chrome\.enabled/d' \
            "$user_js" >"$tmp_js"
        [ -s "$tmp_js" ] && [ -n "$(tail -c1 "$tmp_js")" ] && echo >>"$tmp_js"
        printf '%s\n' \
            'user_pref("devtools.chrome.enabled", true);' \
            'user_pref("toolkit.legacyUserProfileCustomizations.stylesheets", true);' >>"$tmp_js"
        write_if_changed "$user_js" "$tmp_js"
    done

