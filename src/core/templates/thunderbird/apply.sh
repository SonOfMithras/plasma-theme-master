#!/usr/bin/env bash
set -euo pipefail

# Wires the rendered PTM palette into every Thunderbird profile:
#   - prepends @import "<css_file>"; to chrome/userChrome.css
#   - enables toolkit.legacyUserProfileCustomizations.stylesheets in user.js
# Both edits are idempotent. Anything that cannot be written is reported and
# skipped instead of aborting the run.

css_file="${XDG_CACHE_HOME:-$HOME/.cache}/ptm/thunderbird/ptm.css"
import_line="@import \"$css_file\";"
pref_key='toolkit.legacyUserProfileCustomizations.stylesheets'
pref_line="user_pref(\"$pref_key\", true);"
marker="ptm/thunderbird/ptm.css"

warn() { echo "thunderbird: $*" >&2; }

roots=()
for root in \
    "${XDG_CONFIG_HOME:-$HOME/.config}/thunderbird" \
    "$HOME/.thunderbird" \
    "$HOME/.var/app/org.mozilla.Thunderbird/.thunderbird" \
    "$HOME/snap/thunderbird/common/.thunderbird"; do
    [ -d "$root" ] && roots+=("$root")
done

if [ "${#roots[@]}" -eq 0 ]; then
    # Thunderbird is not installed here, nothing to wire.
    exit 0
fi

# -L so profiles managed as symlinks are found too.
profiles=()
while IFS= read -r -d '' prefs; do
    profiles+=("$(dirname "$prefs")")
done < <(find -L "${roots[@]}" -mindepth 2 -maxdepth 2 -type f -name prefs.js -print0)

if [ "${#profiles[@]}" -eq 0 ]; then
    warn "no profile with prefs.js found under ${roots[*]}"
    exit 1
fi

for profile in "${profiles[@]}"; do
    chrome_dir="$profile/chrome"
    user_chrome="$chrome_dir/userChrome.css"
    user_js="$profile/user.js"

    if [ ! -d "$chrome_dir" ] && ! mkdir -p "$chrome_dir" 2>/dev/null; then
        warn "$chrome_dir cannot be created, skipping profile"
        continue
    fi

    # userChrome.css: @import has to be the very first line.
    if [ ! -e "$user_chrome" ]; then
        if [ -w "$chrome_dir" ]; then
            printf '%s\n' "$import_line" >"$user_chrome"
        else
            warn "$user_chrome cannot be created, skipping import"
        fi
    elif grep -qF "$marker" "$user_chrome"; then
        : # already wired
    elif [ -w "$user_chrome" ]; then
        tmp="$(mktemp "${user_chrome}.tmp.XXXXXX")"
        printf '%s\n' "$import_line" >"$tmp"
        cat "$user_chrome" >>"$tmp"
        cat "$tmp" >"$user_chrome"
        rm -f "$tmp"
    else
        warn "$user_chrome is not writable, skipping import"
    fi

    # user.js: keep the pref, never duplicate it, correct a stale false.
    if [ ! -e "$user_js" ]; then
        printf '%s\n' "$pref_line" >>"$user_js"
    elif ! grep -qF "$pref_key" "$user_js"; then
        if [ -w "$user_js" ]; then
            printf '%s\n' "$pref_line" >>"$user_js"
        else
            warn "$user_js is not writable, set $pref_key manually"
        fi
    elif grep -qE "$pref_key\",[[:space:]]*false" "$user_js"; then
        if [ -w "$user_js" ]; then
            tmp="$(mktemp "${user_js}.tmp.XXXXXX")"
            sed -E "s/($pref_key\",[[:space:]]*)false/\1true/" "$user_js" >"$tmp"
            cat "$tmp" >"$user_js"
            rm -f "$tmp"
        else
            warn "$user_js sets $pref_key to false and is not writable"
        fi
    fi
done
