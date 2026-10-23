#!/usr/bin/env bash
set -euo pipefail

# Point superfile at the rendered ptm theme. The TOML file is dropped into
# ~/.config/superfile/theme/ptm.toml by the renderer; this hook only
# rewrites the top-level `theme = "..."` line in the user's config.toml.

config_file="${XDG_CONFIG_HOME:-$HOME/.config}/superfile/config.toml"

if [ ! -f "$config_file" ]; then
    echo "Error: superfile config not found at $config_file" >&2
    echo "Run 'spf' once to let superfile generate its config, then reapply the theme." >&2
    exit 1
fi

write_if_changed() {
    local target="$1" tmp="$2"
    if ! cmp -s "$target" "$tmp"; then
        cat "$tmp" >"$target"
    fi
    rm -f "$tmp"
}

# Idempotent: leave the existing value alone if it already selects ptm,
# rewrite it in place if it points somewhere else, append it if it is missing.
if grep -qE '^theme[[:space:]]*=[[:space:]]*"ptm"' "$config_file"; then
    :
elif grep -qE '^theme[[:space:]]*=' "$config_file"; then
    tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
    sed -E 's/^theme[[:space:]]*=.*/theme = "ptm"/' "$config_file" >"$tmp_file"
    write_if_changed "$config_file" "$tmp_file"
else
    [ -s "$config_file" ] && [ -n "$(tail -c1 "$config_file")" ] && echo >>"$config_file"
    echo 'theme = "ptm"' >>"$config_file"
fi
