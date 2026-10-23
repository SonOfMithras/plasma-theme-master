#!/usr/bin/env bash
set -euo pipefail

config_dir="${XDG_CONFIG_HOME:-$HOME/.config}/velo"
config_file="$config_dir/config"
palette_name="current-ptm-override"

darkmode="true"
[ "${1:-}" = "light" ] && darkmode="false"

mkdir -p "$config_dir"

write_if_changed() {
    local target="$1" tmp="$2"
    if [ ! -e "$target" ] && [ ! -L "$target" ]; then
        mv "$tmp" "$target"
        return
    fi
    if ! cmp -s "$target" "$tmp"; then
        cat "$tmp" >"$target"
    fi
    rm -f "$tmp"
}

if [ ! -f "$config_file" ]; then
    printf 'palette = "%s"\ndarkmode = %s\n' "$palette_name" "$darkmode" >"$config_file"
    exit 0
fi

tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
awk -v palette="$palette_name" -v darkmode="$darkmode" '
    /^palette[[:space:]]*=/ {
        print "palette = \"" palette "\""
        saw_palette = 1
        next
    }
    /^darkmode[[:space:]]*=/ {
        print "darkmode = " darkmode
        saw_darkmode = 1
        next
    }
    { print }
    END {
        if (!saw_palette) {
            print "palette = \"" palette "\""
        }
        if (!saw_darkmode) {
            print "darkmode = " darkmode
        }
    }
' "$config_file" >"$tmp_file"
write_if_changed "$config_file" "$tmp_file"
