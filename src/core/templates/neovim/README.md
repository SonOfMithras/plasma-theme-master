# Neovim

Generates a Base16 palette plus Telescope and mini.pick highlights for Neovim.

## Setup

Enable **Neovim** in **PTM Settings -> Templates**. The generated Lua module is written to `$XDG_CONFIG_HOME/nvim/lua/matugen.lua`. It requires Neovim with `vim.uv` support and the `RRethy/base16-nvim` plugin.

The Bash hook checks for lazy.nvim at `$XDG_DATA_HOME/nvim/lazy/lazy.nvim`. If that directory exists, it creates `$XDG_CONFIG_HOME/nvim/lua/plugins/base16.lua` only when that file is absent and no file under `lua/plugins` mentions `base16-nvim`. The new plugin specification calls `matugen.setup()`; your lazy.nvim configuration must load that plugins directory. If you already configure base16-nvim, arrange to call the generated module yourself.

Without that lazy.nvim directory, install `RRethy/base16-nvim` manually. The hook appends the following to an existing `$XDG_CONFIG_HOME/nvim/init.lua` unless it already contains `pcall(require, 'matugen')`; it does not create `init.lua`:

```lua
local ok, matugen = pcall(require, 'matugen')
if ok then matugen.setup() end
```

Run this after the plugin is available. Back up your configuration first: no backup is created, and the generated module is overwritten on palette updates.

## Updates

The module registers a SIGUSR1 handler that reloads its palette. After rendering, the hook uses `pkill -SIGUSR1 nvim` to signal matching Neovim processes; it needs `pkill` (usually provided by procps). The module must already be loaded for its handler to receive the update. The hook does not filter processes by configuration or install plugins itself.
