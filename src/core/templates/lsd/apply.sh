#!/usr/bin/env bash
set -euo pipefail

# Point lsd at the rendered colors.yaml by setting `color.theme: custom` in
# lsd's config file. Every other setting is left unchanged.

if ! command -v lsd >/dev/null 2>&1; then
    echo "Error: lsd is not installed" >&2
    exit 1
fi

# lsd reads the first config.yaml or config.yml it finds, searching
# ~/.config/lsd before $XDG_CONFIG_HOME/lsd.
config_file=""
for dir in "$HOME/.config/lsd" "${XDG_CONFIG_HOME:-$HOME/.config}/lsd"; do
    for name in config.yaml config.yml; do
        if [ -f "$dir/$name" ]; then
            config_file="$dir/$name"
            break 2
        fi
    done
done

# lsd never writes a config file itself, so create a minimal one.
if [ -z "$config_file" ]; then
    config_file="${XDG_CONFIG_HOME:-$HOME/.config}/lsd/config.yaml"
    mkdir -p "$(dirname "$config_file")"
    printf 'color:\n  theme: custom\n' >"$config_file"
    exit 0
fi

tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
trap 'rm -f "$tmp_file"' EXIT

# Idempotent: leave `theme: custom` alone, rewrite any other theme value in
# place, add `theme: custom` to an existing `color:` block, or append a new
# block. Exits 2 for an inline `color:` value, which is not edited.
status=0
awk '
{ line[NR] = $0 }

END {
    for (i = 1; i <= NR; i++) {
        if (line[i] ~ /^color[[:space:]]*:/) {
            start = i
            break
        }
    }

    if (!start) {
        for (i = 1; i <= NR; i++) print line[i]
        print "color:"
        print "  theme: custom"
        exit 0
    }

    if (line[start] ~ /^color[[:space:]]*:[[:space:]]*[^[:space:]#]/) exit 2

    # The block runs until the next top-level line.
    end = NR
    for (i = start + 1; i <= NR; i++) {
        if (line[i] ~ /^[^[:space:]#]/) {
            end = i - 1
            break
        }
    }

    # Match the indentation of the first child key, defaulting to two spaces.
    indent = "  "
    for (i = start + 1; i <= end; i++) {
        if (line[i] ~ /^[[:space:]]+[^[:space:]#]/) {
            match(line[i], /^[[:space:]]+/)
            indent = substr(line[i], 1, RLENGTH)
            break
        }
    }

    found = 0
    for (i = start + 1; i <= end; i++) {
        rest = substr(line[i], length(indent) + 1)
        if (substr(line[i], 1, length(indent)) == indent && rest ~ /^theme[[:space:]]*:/) {
            found = 1
            if (rest !~ /^theme[[:space:]]*:[[:space:]]*"?custom"?[[:space:]]*(#.*)?$/) {
                line[i] = indent "theme: custom"
            }
        }
    }

    for (i = 1; i <= NR; i++) {
        print line[i]
        if (i == start && !found) print indent "theme: custom"
    }
}
' "$config_file" >"$tmp_file" || status=$?

if [ "$status" -eq 2 ]; then
    echo "Error: $config_file sets color: inline; change it to a block with 'theme: custom' by hand" >&2
    exit 1
elif [ "$status" -ne 0 ]; then
    echo "Error: failed to update $config_file" >&2
    exit 1
fi

# Write through a symlink instead of replacing it via mv.
if ! cmp -s "$config_file" "$tmp_file"; then
    cat "$tmp_file" >"$config_file"
fi
