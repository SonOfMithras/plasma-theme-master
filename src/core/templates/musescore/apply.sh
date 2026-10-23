#!/usr/bin/env bash
set -euo pipefail

musescore_config_dir="${XDG_CONFIG_HOME:-$HOME/.config}/MuseScore"
theme_file="$musescore_config_dir/ptm.ini"
config_file="$musescore_config_dir/MuseScore4.ini"

touch "$config_file"

# Verify theme file exists.
if [ ! -f "$theme_file" ]; then
  echo "ptm.ini not found" >&2
  exit 1
fi

tmp_file="$(mktemp)"
stripped_config_file="$(mktemp)"

# Check [ui] section exists.
if grep -q "^\[ui\]$" "$config_file"; then
  # Remove theme (may not be set). Then insert theme.
  awk '!/^application\\themes=|^canvas\\background*/' "$config_file" >"$stripped_config_file"
  awk -v theme_file="$theme_file" '/^\[ui\]$/{print; while(getline line < theme_file) print line; next}1' "$stripped_config_file" >"$tmp_file"

else
  # Append [ui] section and theme.
  echo [ui] >>"$config_file"
  cat "$config_file" "$theme_file" >"$tmp_file"
fi

# Write through a symlink instead of replacing it via mv.
if ! cmp -s "$config_file" "$tmp_file"; then
  cat "$tmp_file" >"$config_file"
fi
rm -f "$stripped_config_file" "$tmp_file"
