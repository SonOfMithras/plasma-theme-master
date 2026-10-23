# LazyVim template for PTM

Generates a self-contained Neovim colourscheme called **`ptm`** that
follows PTM's Material 3 palette, and registers it as LazyVim's
colorscheme.

## What does it theme

- A new file at `~/.config/nvim/colors/ptm.lua`. This is a real
  Neovim colourscheme that fully replaces the active highlight set on
  load — no `base16-nvim` or other plugin is required.
- A plugin spec at `~/.config/nvim/lua/plugins/ptm.lua` that sets
  LazyVim's `colorscheme` option to `"ptm"`, so LazyVim loads it on
  startup. This is the LazyVim-native way of adding a theme.

The colourscheme covers the editor chrome, search/selection, diagnostics,
treesitter capture groups, LSP semantic tokens, inlay hints, the
`:terminal` palette, and the highlight groups the LazyVim-bundled plugins
rely on (Snacks dashboard / picker / notifier, blink.cmp, neo-tree,
flash.nvim, gitsigns, which-key, treesitter-context, aerial, illuminate,
Trouble, lazy.nvim, telescope, …). lualine and bufferline use their
`auto` themes, which derive from these groups.

## How it stays in sync

Every time PTM rewrites the colours file, the hook sends one
`SIGUSR1` to running Neovim sessions. The signal handler at the bottom of
`colors/ptm.lua` runs `:colorscheme ptm` again, so open
sessions pick up the new palette immediately and plugins listening to
`ColorScheme` refresh too. The handler is idempotent: if you have
switched to another colourscheme, the signal is ignored.

The hook never edits or overwrites your existing files:

- If `lua/plugins/ptm.lua` already exists, it is left untouched. If
  it does not select `ptm`, the hook prints a warning on stderr.
- Otherwise, the spec is only written if no file under `lua/plugins/`
  already selects `ptm`. If another spec sets a `colorscheme`, the
  hook prints a warning so you can remove it.

## Contrast guard

Generated palettes do not always give readable pairs (e.g. an amber
`primary` on a beige light surface). On load, every highlight's
foreground is checked against the background it is drawn on using the
WCAG contrast ratio and, if it falls short, is mixed towards white or
black until it meets the minimum, keeping its hue:

- **4.5** for body text (code, menus, messages).
- **3.0** for UI and accents (signs, icons, borders, line numbers,
  comments, titles, dim labels).
- Deliberately faint decoration (whitespace, indent guides, separators,
  backdrops) is left alone.

Selection-style overlays (Visual, CursorLine, diff, references) only set a
background, so they are reported rather than changed. Run
`:PTMContrast` to see every pair that was adjusted or flagged on the
last load.

## Requirements

- A LazyVim config at `$XDG_CONFIG_HOME/nvim` (detected via
  `lazyvim.json`, which LazyVim creates). `requires_path` skips the entry
  otherwise.
- Neovim >= 0.10.

## Notes

- The palette follows the same material roles ptm uses everywhere:
  surfaces for backgrounds, primary/secondary/tertiary for accents,
  error for red. Warnings and git additions/changes use the terminal
  yellow and green so they keep a recognisable hue on any wallpaper.
- `vim.o.background` follows PTM's dark / light mode, and the switch
  is applied live like any other palette change.
- Do not enable this together with the `neovim` template (base16 +
  matugen): both drive the same highlights and will fight each other.
- Tested with Neovim 0.12.5 and LazyVim 16.0.1.
- This template belongs to the **editor** category.
