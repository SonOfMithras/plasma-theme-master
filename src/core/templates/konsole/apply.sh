#!/usr/bin/env bash
# Post-hook for Konsole PTM template:
#
# 1. Restores user customizations (opacity, blur, wallpaper) saved by pre-apply.sh,
#    ensuring Description=PTM is always present so Konsole names the scheme properly.
#
# 2. Performs a live reload of running Konsole sessions via OSC 50 escape sequences,
#    evicting Konsole's in-memory ColorScheme cache and re-reading the updated colors
#    from disk without requiring an application restart.

set -euo pipefail

output="${XDG_DATA_HOME:-$HOME/.local/share}/konsole/ptm.colorscheme"

if [ ! -f "$output" ]; then
    echo >&2 "konsole: rendered colorscheme not found at $output"
    exit 1
fi

backup="${output}.general.bak"

if [ -f "$backup" ]; then
    # Sanitize any legacy Noctalia description to PTM
    sed -i 's/^Description=Noctalia$/Description=PTM/' "$backup"

    # Ensure Description is present so the scheme never appears as "sin nombre"
    if ! grep -q '^Description=' "$backup"; then
        echo "Description=PTM" >> "$backup"
    fi

    # Remove the default [General] section rendered by the template
    sed -i --follow-symlinks '/^\[General\]/,$ d' "$output"

    # Append the user's saved [General] preferences and clean up backup
    cat "$backup" >> "$output"
    rm -f "$backup"
fi

# Ensure Description=PTM is set if not already present
if grep -q '^Description=Noctalia' "$output"; then
    sed -i 's/^Description=Noctalia/Description=PTM/' "$output"
fi

# ── 1b. Konsole profile & legacy cleanup ──────────────────────────────
konsole_dir="$(dirname "$output")"
if [ -d "$konsole_dir" ]; then
    # Clean legacy duplicate schemes so user sees only one single PTM option
    rm -f "$konsole_dir/PlasmaMaster.colorscheme" \
          "$konsole_dir/PlasmaMaster.profile" \
          "$konsole_dir/PlasmaMaster.colorscheme.bak" \
          "$konsole_dir/noctalia.colorscheme"

    # Migrate konsolerc to PTM.profile if it pointed to PlasmaMaster
    konsolerc="${XDG_CONFIG_HOME:-$HOME/.config}/konsolerc"
    if [ -f "$konsolerc" ]; then
        sed -i 's/DefaultProfile=PlasmaMaster\.profile/DefaultProfile=PTM.profile/' "$konsolerc"
    fi

    # Ensure PTM profile exists
    if [ ! -f "$konsole_dir/PTM.profile" ]; then
        cat > "$konsole_dir/PTM.profile" << 'EOF'
[Appearance]
ColorScheme=ptm

[General]
Name=PTM
Parent=FALLBACK/
EOF
    fi
fi

# ── 2. Live-reload active Konsole sessions (OSC 50) ────────────────────
# Collect all active Konsole terminal pts devices (held directly by Konsole or child shells)
ttys=()

for konsole_pid in $(pgrep -x konsole 2>/dev/null || true); do
    # Check open file descriptors of the Konsole process
    for fd in /proc/"$konsole_pid"/fd/*; do
        target="$(readlink "$fd" 2>/dev/null || true)"
        if [[ "$target" =~ ^/dev/pts/[0-9]+$ ]]; then
            ttys+=("$target")
        fi
    done

    # Check controlling terminals of child processes (shells running in tabs/splits)
    for child in $(pgrep -P "$konsole_pid" 2>/dev/null || true); do
        child_tty="$(ps -o tty= -p "$child" 2>/dev/null | tr -d ' ' || true)"
        if [ -n "$child_tty" ] && [ "$child_tty" != "?" ]; then
            ttys+=("/dev/$child_tty")
        fi
    done
done

# If active terminals were found, cycle the scheme to trigger live reload
if [ ${#ttys[@]} -gt 0 ]; then
    unique_ttys=($(printf '%s\n' "${ttys[@]}" | sort -u))

    # Step A: Switch briefly to an alternate scheme name to release Konsole's
    # in-memory cached ColorScheme pointer (no physical file needed on disk)
    for tty_path in "${unique_ttys[@]}"; do
        if [ -w "$tty_path" ]; then
            printf '\033]50;ColorScheme=ptm-transition\a' > "$tty_path" 2>/dev/null || true
        fi
    done

    # Step B: Brief pause for Konsole's event loop to release the cached scheme
    sleep 0.08

    # Step C: Switch back to ptm, forcing Konsole to read updated colors from disk
    for tty_path in "${unique_ttys[@]}"; do
        if [ -w "$tty_path" ]; then
            printf '\033]50;ColorScheme=ptm\a' > "$tty_path" 2>/dev/null || true
        fi
    done
fi
