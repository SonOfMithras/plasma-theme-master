#!/usr/bin/env bash
set -euo pipefail

tmux_dir="$(cd "$(dirname "$0")" && pwd)"
theme_file="$(mktemp)"
socket_name="ptm-theme-test-$$"

cleanup() {
  tmux -L "$socket_name" kill-server 2>/dev/null || true
  rm -f "$theme_file"
}
trap cleanup EXIT

sed \
  -e 's/{{ colors.surface_container_low.default.hex }}/#111111/g' \
  -e 's/{{ colors.surface_container.default.hex }}/#222222/g' \
  -e 's/{{[^}]*}}/#333333/g' \
  "$tmux_dir/tmux.conf" >"$theme_file"

tmux -L "$socket_name" -f /dev/null new-session -d
tmux -L "$socket_name" set -g @ptm_inactive_tabs_use_background off
tmux -L "$socket_name" source-file "$theme_file"
[[ "$(tmux -L "$socket_name" show-options -gv window-status-style)" != *bg=* ]]

tmux -L "$socket_name" set -g @ptm_inactive_tabs_use_background on
tmux -L "$socket_name" source-file "$theme_file"
[[ "$(tmux -L "$socket_name" show-options -gv window-status-style)" == *bg=#222222* ]]
