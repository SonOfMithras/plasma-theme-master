#!/usr/bin/env bash
set -euo pipefail

config_home="${XDG_CONFIG_HOME:-$HOME/.config}"
config_file="${HERDR_CONFIG_PATH:-$config_home/herdr/config.toml}"
colors_file="$config_home/herdr/ptm-colors.toml"

if [ ! -f "$config_file" ]; then
    echo "Error: Herdr config file not found at $config_file" >&2
    exit 1
fi

if [ ! -f "$colors_file" ]; then
    echo "Error: rendered Herdr colors not found at $colors_file" >&2
    exit 1
fi

tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
trap 'rm -f "$tmp_file"' EXIT

awk '
    NR == FNR {
        colors[++color_lines] = $0
        key = $0
        if (key ~ /^[[:space:]]*[[:alnum:]_-]+[[:space:]]*=/) {
            sub(/^[[:space:]]*/, "", key)
            sub(/[[:space:]]*=.*/, "", key)
            managed[key] = 1
        }
        next
    }

    function normalized_header(line, value) {
        value = line
        sub(/[[:space:]]*#.*/, "", value)
        gsub(/[[:space:]]/, "", value)
        return value
    }

    function is_header(line) {
        return substr(normalized_header(line), 1, 1) == "["
    }

    function emit_colors(i) {
        for (i = 1; i <= color_lines; i++) {
            print colors[i]
        }
    }

    {
        config_lines++
        header = normalized_header($0)

        if (header == "[theme.custom]") {
            custom_sections++
            if (custom_sections > 1) {
                print "Error: Herdr config contains multiple [theme.custom] sections" > "/dev/stderr"
                failed = 1
                exit 2
            }

            print
            emit_colors()
            found_custom = 1
            in_custom = 1
            in_theme = 0
            next
        }

        if (is_header($0)) {
            in_custom = 0
            in_theme = (header == "[theme]")
        }

        if (in_theme && $0 ~ /^[[:space:]]*custom[[:space:]]*=/) {
            inline_custom = 1
        }

        if (in_custom) {
            key = $0
            if (key ~ /^[[:space:]]*[[:alnum:]_-]+[[:space:]]*=/) {
                sub(/^[[:space:]]*/, "", key)
                sub(/[[:space:]]*=.*/, "", key)
                if (key in managed) {
                    next
                }
            }
        }

        print
    }

    END {
        if (failed) {
            exit 2
        }

        if (found_custom) {
            exit 0
        }

        if (inline_custom) {
            print "Error: inline theme.custom tables are not supported; use a [theme.custom] section" > "/dev/stderr"
            exit 2
        }

        if (config_lines > 0) {
            print ""
        }
        print "[theme.custom]"
        emit_colors()
    }
' "$colors_file" "$config_file" >"$tmp_file"

changed=0
if ! cmp -s "$config_file" "$tmp_file"; then
    cat "$tmp_file" >"$config_file"
    changed=1
fi

if [ "$changed" -eq 1 ] && command -v herdr >/dev/null 2>&1; then
    herdr server reload-config >/dev/null 2>&1 || true
fi
