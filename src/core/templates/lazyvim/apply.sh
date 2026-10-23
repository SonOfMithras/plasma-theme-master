#!/usr/bin/env bash
#
# apply.sh — wires PTM's ptm colourscheme into LazyVim.
#
# The requires_path gate ($XDG_CONFIG_HOME/nvim/lazyvim.json, which LazyVim
# creates in every starter config) guarantees LazyVim is set up before
# this runs.
#
# Behaviour (existing files are never modified):
#   - If lua/plugins/ptm.lua already exists, it is left untouched. When
#     it does not select ptm, the conflict is reported on stderr.
#   - Otherwise, if another spec already selects ptm, do nothing.
#   - Otherwise, write lua/plugins/ptm.lua with a minimal spec that sets
#     LazyVim's colorscheme to "ptm", warning on stderr if another spec
#     also sets a colorscheme (lazy.nvim merges opts, so the user should
#     pick one).
#   - One SIGUSR1 is sent to running Neovim sessions on every run so they
#     reload the rewritten palette. The handler in colors/ptm.lua is
#     idempotent and ignores the signal when ptm is not active.

set -euo pipefail

xdg_config="${XDG_CONFIG_HOME:-$HOME/.config}"
plugins_dir="$xdg_config/nvim/lua/plugins"
spec_file="$plugins_dir/ptm.lua"
selects_ptm="^[^-]*colorscheme([[:space:]=(]*[\"']ptm[\"']|[[:space:]]+ptm)"

if [ -e "$spec_file" ]; then
  if ! grep -qsE "$selects_ptm" "$spec_file"; then
    echo "lazyvim: $spec_file already exists and does not select ptm; leaving it untouched." >&2
    echo "lazyvim: set colorscheme = \"ptm\" in it (or remove it) to use this theme." >&2
  fi
elif ! grep -rqsE "$selects_ptm" "$plugins_dir"; then
  # Skip specs disabled the LazyVim-starter way (`if true then return {} end`).
  others=""
  while IFS= read -r f; do
    grep -qF 'if true then return {} end' "$f" || others+="$f"$'\n'
  done < <(grep -rlsE '^[^-]*colorscheme[[:space:]]*=' "$plugins_dir" || true)
  if [ -n "$others" ]; then
    echo "lazyvim: another spec also sets a colorscheme; remove it to let ptm win:" >&2
    printf '%s' "$others" >&2
  fi
  mkdir -p "$plugins_dir"
  # noclobber: never overwrite a spec that appeared in the meantime.
  (
    set -C
    cat >"$spec_file" <<'EOF'
return {
  {
    "LazyVim/LazyVim",
    opts = {
      colorscheme = "ptm",
    },
  },
}
EOF
  )
fi

pkill -SIGUSR1 -x nvim >/dev/null 2>&1 || true
