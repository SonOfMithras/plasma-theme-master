#!/usr/bin/env bash
set -euo pipefail

nvim_lua_dir="${XDG_CONFIG_HOME:-$HOME/.config}/nvim/lua"
plugins_dir="$nvim_lua_dir/plugins"
plugin_file="$plugins_dir/base16.lua"
init_lua="${XDG_CONFIG_HOME:-$HOME/.config}/nvim/init.lua"
lazy_dir="${XDG_DATA_HOME:-$HOME/.local/share}/nvim/lazy/lazy.nvim"

if [ -d "$lazy_dir" ]; then
    if [ ! -f "$plugin_file" ] && ! grep -rl "base16-nvim" "${plugins_dir}" 2>/dev/null | grep -q .; then
        mkdir -p "$plugins_dir"
        cat > "$plugin_file" <<'EOF'
return { 'RRethy/base16-nvim',
  config = function()
    local ok, matugen = pcall(require, 'matugen')
    if ok then matugen.setup() end
  end,
}
EOF
    fi
else
    echo "lazy.nvim not installed. Install RRethy/base16-nvim manually." >&2
    if [ -f "$init_lua" ] && ! grep -qF "pcall(require, 'matugen')" "$init_lua"; then
        cat >> "$init_lua" <<'EOF'

local ok, matugen = pcall(require, 'matugen')
if ok then matugen.setup() end
EOF
    fi
fi

pkill -SIGUSR1 nvim >/dev/null 2>&1 || true
