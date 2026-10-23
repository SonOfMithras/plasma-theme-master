#!/bin/sh
COLORS_FILE="${XDG_CONFIG_HOME:-$HOME/.config}/qutebrowser/ptm/colors.py"

# Skip if colors.py hasn't changed
[ "$COLORS_FILE" -nt "$0" ] || exit 0
touch "$0"

pgrep -f qutebrowser >/dev/null && qutebrowser :config-source
