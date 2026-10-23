#!/usr/bin/env bash
set -euo pipefail

config_dir_xdg="${XDG_CONFIG_HOME:-$HOME/.config}/fastfetch"
config_file="$config_dir_xdg/config.jsonc"
theme_file="$config_dir_xdg/themes/ptm.jsonc"

if [ ! -f "$config_file" ]; then
    echo "Error: fastfetch config not found at $config_file -- run fastfetch once to generate a default config first." >&2
    exit 1
fi

if [ ! -f "$theme_file" ]; then
    echo "Warning: fastfetch theme file not found: $theme_file" >&2
    exit 0
fi

# jq only accepts strict JSON. config.jsonc is JSONC (comments/trailing commas allowed), so
# fail loudly with a clear message rather than let a bare jq parse error explain nothing, or
# silently skip and leave the user without any indication why colors never apply.
if ! jq empty "$config_file" 2>/dev/null; then
    echo "Error: $config_file could not be parsed as strict JSON. Comments and trailing commas (valid JSONC, not valid JSON) are not currently supported by this hook -- remove them to use this template." >&2
    exit 1
fi

# fastfetch 2.66.0 has a confirmed real bug in its logo renderer: a
# logo.color value is dropped silently -- the logo falls back to its
# default palette with no error -- whenever its truecolor SGR string
# ("38;2;R;G;B") is exactly 16 characters, i.e. every one of R/G/B is a
# 3-digit number (>=100). That covers a large share of real accent colors
# (most pastel/light theme colors included). display.color.keys/title/
# percent are unaffected -- only logo.color.N goes through the broken path.
# Verified live against 2.66.0 by isolating the exact byte-length trigger:
# any hex where all three channels are >=100 reproduces it, regardless of
# hue; anything with at least one channel <100 (a shorter SGR string)
# renders correctly.
#
# Route around it by nudging the single *smallest* channel down to 99
# whenever all three are >=100 -- that's the minimal change that drops the
# SGR string below 16 chars, and it's always the channel closest to the
# 100 boundary already, so it's the smallest possible perceptual shift.
# (An earlier version of this fix quantized to the nearest ANSI-256 cube
# color instead, but the 216-color cube only has 6 steps per channel, so a
# near-white pastel like #fff59b snapped to #ffffaf -- visually just
# "white" on a small ascii logo. This keeps full 24-bit truecolor.)
nudge_channel() {
    local hex="${1#\#}"
    local r=$((16#${hex:0:2}))
    local g=$((16#${hex:2:2}))
    local b=$((16#${hex:4:2}))
    if [ "$r" -ge 100 ] && [ "$g" -ge 100 ] && [ "$b" -ge 100 ]; then
        if [ "$r" -le "$g" ] && [ "$r" -le "$b" ]; then
            r=99
        elif [ "$g" -le "$r" ] && [ "$g" -le "$b" ]; then
            g=99
        else
            b=99
        fi
    fi
    printf '#%02x%02x%02x' "$r" "$g" "$b"
}

tmp_theme="$(mktemp)"
tmp_file=""
trap 'rm -f "$tmp_theme" "${tmp_file:-}"' EXIT

logo_1="$(jq -r '.logo.color."1" // empty' "$theme_file")"
logo_2="$(jq -r '.logo.color."2" // empty' "$theme_file")"

jq \
    --arg c1 "$([ -n "$logo_1" ] && nudge_channel "$logo_1" || echo "")" \
    --arg c2 "$([ -n "$logo_2" ] && nudge_channel "$logo_2" || echo "")" \
    '(if $c1 != "" then .logo.color."1" = $c1 else . end)
     | (if $c2 != "" then .logo.color."2" = $c2 else . end)' \
    "$theme_file" >"$tmp_theme"

# Compare only the "logo"/"display" sub-objects semantically (jq -S, sorted+canonical),
# not the whole file's raw bytes -- jq's own re-serialization would never byte-match
# hand-formatted JSON, which made a naive cmp-the-whole-file guard fire on every single
# apply regardless of whether colors actually changed.
current_logo="$(jq -S '.logo // {}' "$config_file")"
current_display="$(jq -S '.display // {}' "$config_file")"
new_logo="$(jq -S '.logo // {}' "$tmp_theme")"
new_display="$(jq -S '.display // {}' "$tmp_theme")"

if [ "$current_logo" != "$new_logo" ] || [ "$current_display" != "$new_display" ]; then
    tmp_file="$(mktemp)"
    jq -s '.[0] * .[1]' "$config_file" "$tmp_theme" >"$tmp_file"
    cat "$tmp_file" >"$config_file"
fi
