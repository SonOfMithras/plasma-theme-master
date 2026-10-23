# bat

Adds a PTM syntax-highlighting theme for bat.

Enable **bat** in **PTM Settings -> Templates**. The template writes `$XDG_CONFIG_HOME/bat/themes/ptm.tmTheme` (`~/.config/bat/themes/ptm.tmTheme` by default).

The Bash post-hook creates `$XDG_CONFIG_HOME/bat/config` if necessary and selects `--theme=ptm`. Unless that exact line is already present, it removes existing lines beginning with `--theme=` before appending the new selection. Back up your config first; no backup is created.

Install `bat` or `batcat`: the hook runs the available executable with `cache --build` after generation. If neither exists, it prints a warning; run `bat cache --build` (or `batcat cache --build`) yourself after installation.
