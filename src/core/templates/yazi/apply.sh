#!/usr/bin/env bash
set -euo pipefail

config_file="${XDG_CONFIG_HOME:-$HOME/.config}/yazi/theme.toml"
config_dir="$(dirname "$config_file")"

mkdir -p "$config_dir"

# If the config does not exist yet, create a minimal one and stop here.
if [ ! -f "$config_file" ]; then
    cat >"$config_file" <<'EOF'
[flavor]
dark = "ptm"
light = "ptm"
EOF
    exit 0
fi

# Temporary file used so we rewrite safely, then replace the original at the end.
tmp_file="$(mktemp)"

awk '
BEGIN {
    # Tracks whether we are currently inside the [flavor] section.
    in_flavor = 0

    # Tracks whether a [flavor] section exists anywhere in the file.
    saw_flavor = 0

    # Tracks whether dark/light keys were seen inside the current [flavor] section.
    saw_dark = 0
    saw_light = 0

    # Stores blank lines while inside [flavor] so we can place them neatly.
    blank_buf = ""
}

# Print any missing keys before leaving the [flavor] section.
function print_flavor_defaults() {
    if (!saw_dark)  print "dark = \"ptm\""
    if (!saw_light) print "light = \"ptm\""
}

# Flush buffered blank lines back into the output.
function flush_blank_buf() {
    if (blank_buf != "") {
        printf "%s", blank_buf
        blank_buf = ""
    }
}

{
    # Enter the [flavor] section and reset per-section tracking.
    if ($0 ~ /^\[flavor\][[:space:]]*$/) {
        saw_flavor = 1
        in_flavor = 1
        saw_dark = 0
        saw_light = 0
        blank_buf = ""
        print
        next
    }

    if (in_flavor) {
        # Hold blank lines temporarily so we can decide whether they belong
        # inside [flavor] or should stay as spacing before the next section.
        if ($0 ~ /^[[:space:]]*$/) {
            blank_buf = blank_buf $0 "\n"
            next
        }

        # If we reached the next TOML section, finish [flavor] first:
        # add missing keys, then restore blank lines, then print the new header.
        if ($0 ~ /^\[/) {
            print_flavor_defaults()
            flush_blank_buf()
            in_flavor = 0
            print
            next
        }

        # Real content inside [flavor] means buffered blank lines belong here.
        flush_blank_buf()

        # Replace any existing dark/light assignments with the desired value.
        if ($0 ~ /^[[:space:]]*dark[[:space:]]*=/) {
            print "dark = \"ptm\""
            saw_dark = 1
            next
        }

        if ($0 ~ /^[[:space:]]*light[[:space:]]*=/) {
            print "light = \"ptm\""
            saw_light = 1
            next
        }
    }

    # Print all unrelated lines unchanged.
    print
}

END {
    # If the file ended while still inside [flavor], finish that section now.
    if (in_flavor) {
        print_flavor_defaults()
        flush_blank_buf()

    # If no [flavor] section existed at all, append one at the end.
    } else if (!saw_flavor) {
        if (NR > 0) print ""
        print "[flavor]"
        print "dark = \"ptm\""
        print "light = \"ptm\""
    }
}
' "$config_file" >"$tmp_file"

# Write through a symlink instead of replacing it via mv.
if ! cmp -s "$config_file" "$tmp_file"; then
    cat "$tmp_file" >"$config_file"
fi
rm -f "$tmp_file"
