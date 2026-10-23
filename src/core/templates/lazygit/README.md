# lazygit

Applies PTM colors to lazygit's borders, selection, text, commit markers, and unstaged changes.

Enable **lazygit** in **PTM Settings -> Templates**. The generated theme is `$XDG_CONFIG_HOME/lazygit/themes/ptm.yml` (`~/.config/lazygit/themes/ptm.yml` by default).

## Setup and hook

Install lazygit. The Bash hook creates `$XDG_CONFIG_HOME/lazygit/config.yml` if needed, then uses `awk` to copy the generated `gui.theme` block into it. Existing `gui.theme` content is replaced, while unrelated settings are retained; a missing `gui` or `theme` block is added.

The merge expects block-style YAML with a top-level `gui:` key, a two-space-indented `theme:` key, and theme entries indented at least four spaces. Use that layout in an existing config rather than inline YAML mappings.

Back up your config before enabling: the hook creates no backup and removes comments within the old theme block along with its settings. It writes through config symlinks. Standard tools including `awk`, `mktemp`, and `cmp` are required. No command is provided to reload a running lazygit instance.
