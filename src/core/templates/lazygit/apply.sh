#!/usr/bin/env bash
set -euo pipefail

lazygit_config_dir="${XDG_CONFIG_HOME:-$HOME/.config}/lazygit"
themes_dir="$lazygit_config_dir/themes"
theme_file="$themes_dir/ptm.yml"
config_file="$lazygit_config_dir/config.yml"

mkdir -p "$themes_dir"
touch "$config_file"

# 1. Verify theme file exists
if [ ! -f "$theme_file" ]; then
    echo "ptm.yml not found" >&2
    exit 1
fi

# 2. Extract the theme block cleanly from rendered theme file
theme_block=$(awk '
    # Detect the top-level "gui:" block starter.
    /^gui[[:space:]]*:/ {
        in_gui = 1
        next
    }

    # If inside gui, look for "theme:" indented by two spaces.
    in_gui && /^  theme[[:space:]]*:/ {
        in_theme = 1
        next
    }

    # While inside the theme block, collect lines indented by at least four spaces.
    in_theme {
        if ($0 ~ /^    /) {
            print
        } else if ($0 !~ /^[[:space:]]*$/ && $0 !~ /^[[:space:]]*#/) {
            exit
        }
    }
' "$theme_file")

if [ -z "$theme_block" ]; then
    echo "failed to extract theme from ptm.yml" >&2
    exit 1
fi

# Create a safe temporary file for the rewrite
tmp_file="$(mktemp)"

# 3. Rebuild config.yml safely using AWK
awk -v theme_data="$theme_block" '
BEGIN {
    in_gui = 0
    in_theme = 0
    saw_gui = 0
    saw_theme = 0
}

# Print the theme block under gui.
function print_theme() {
    print "  theme:"
    print theme_data
    saw_theme = 1
}

{
    # Check if we hit the top-level "gui:" key.
    if ($0 ~ /^gui[[:space:]]*:/) {
        saw_gui = 1
        in_gui = 1
        print
        next
    }

    if (in_gui) {
        # Check if we hit the "theme:" key inside gui.
        if ($0 ~ /^  theme[[:space:]]*:/) {
            print_theme()
            in_theme = 1
            next
        }

        # If we are inside theme, discard old theme lines, blank lines, and comments.
        if (in_theme) {
            if ($0 ~ /^    / || $0 ~ /^[[:space:]]*$/ || $0 ~ /^[[:space:]]*#/) {
                next
            }

            in_theme = 0
        }

        # A sibling key is still part of gui. Insert theme before the first
        # sibling when theme is missing, but remain in the gui context.
        if ($0 ~ /^  [^[:space:]#]/) {
            if (!saw_theme) {
                print_theme()
            }
        }

        # A non-comment top-level key ends the gui block.
        if ($0 ~ /^[^[:space:]#]/) {
            if (!saw_theme) {
                print_theme()
            }

            in_gui = 0
        }
    }

    # Print unrelated lines unchanged.
    print
}

END {
    # If the file ended while we were inside gui and we never saw theme, print it now.
    if (in_gui && !saw_theme) {
        print_theme()
    } else if (!saw_gui) {
        if (NR > 0) {
            print ""
        }

        print "gui:"
        print_theme()
    }
}
' "$config_file" > "$tmp_file"

# Write through a symlink instead of replacing it via mv.
if ! cmp -s "$config_file" "$tmp_file"; then
    cat "$tmp_file" >"$config_file"
fi
rm -f "$tmp_file"
