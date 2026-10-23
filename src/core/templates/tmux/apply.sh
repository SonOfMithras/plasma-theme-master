#!/usr/bin/env bash
set -euo pipefail

theme_file="${XDG_CONFIG_HOME:-$HOME/.config}/tmux/themes/ptm.conf"

if ! command -v tmux >/dev/null 2>&1; then
  echo "Error: tmux is not installed" >&2
  exit 1
fi

if [ ! -f "$theme_file" ]; then
  echo "Error: rendered tmux theme not found at $theme_file" >&2
  exit 1
fi

if [ -f "$HOME/.tmux.conf" ]; then
  config_file="$HOME/.tmux.conf"
elif [ -n "${XDG_CONFIG_HOME:-}" ] && [ -f "$XDG_CONFIG_HOME/tmux/tmux.conf" ]; then
  config_file="$XDG_CONFIG_HOME/tmux/tmux.conf"
elif [ -f "$HOME/.config/tmux/tmux.conf" ]; then
  config_file="$HOME/.config/tmux/tmux.conf"
else
  echo "Error: tmux config file not found" >&2
  exit 1
fi

if [ -n "${XDG_CONFIG_HOME:-}" ]; then
  source_line='source-file -q "$XDG_CONFIG_HOME/tmux/themes/ptm.conf"'
else
  source_line='source-file -q "$HOME/.config/tmux/themes/ptm.conf"'
fi

tmp_file="$(mktemp "${config_file}.tmp.XXXXXX")"
trap 'rm -f "$tmp_file"' EXIT
awk '
  /^[[:space:]]*source(-file)?[[:space:]]/ && index($0, "tmux/themes/ptm.conf") { next }
  { print }
' "$config_file" >"$tmp_file"
printf '%s\n' "$source_line" >>"$tmp_file"

if ! cmp -s "$config_file" "$tmp_file"; then
  cat "$tmp_file" >"$config_file"
fi

socket_dir="${TMUX_TMPDIR:-/tmp}/tmux-$(id -u)"

for socket_path in "$socket_dir"/*; do
  [ -S "$socket_path" ] || continue

  config_files="$(
    tmux -N -S "$socket_path" display-message -p '#{config_files}' 2>/dev/null
  )" || continue

  case ",$config_files," in
    *",$config_file,"*)
      tmux -N -S "$socket_path" source-file "$theme_file"
      ;;
  esac
done
