#!/usr/bin/env bash
# Merge the PTM-generated [theme] section into Ratune's config.toml.
# Everything else in the config is left untouched.
set -euo pipefail

dir="${XDG_CONFIG_HOME:-$HOME/.config}/ratune"
config="$dir/config.toml"
generated="$dir/theme.generated.toml"

if [[ ! -f "$config" ]]; then
    echo "Ratune config not found: $config" >&2
    exit 1
fi

if [[ ! -f "$generated" ]]; then
    echo "Generated PTM theme not found: $generated" >&2
    exit 1
fi

if ! grep -qE '^\[theme\][[:space:]]*$' "$generated"; then
    echo "Generated theme has no [theme] section: $generated" >&2
    exit 1
fi

tmp="$(mktemp "$config.XXXXXX")"
trap 'rm -f "$tmp"' EXIT

# Swap the existing [theme] section for the generated one, or append it if
# the config has none. Other sections and comments outside [theme] are kept.
THEME="$(<"$generated")" awk '
    function emit() { print ENVIRON["THEME"]; print ""; done = 1 }
    /^\[theme\][ \t\r]*$/ { if (!done) emit(); skip = 1; next }
    /^\[/                 { skip = 0 }
    !skip                 { print }
    END                   { if (!done) { print ""; emit() } }
' "$config" > "$tmp"

# Nothing changed: leave the file alone so ratune does not reload for nothing.
if cmp -s "$tmp" "$config"; then
    exit 0
fi

# Write through the existing file (not mv) so a symlinked config, e.g. one
# managed by a dotfiles repo, stays a symlink and keeps its permissions.
cat "$tmp" > "$config"
