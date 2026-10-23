# rio

Generate a PTM terminal palette for rio, including ANSI colors, cursor, selection, search, and hints.

Enable **rio** in **PTM Settings → Templates**. The generated theme is written to `$XDG_CONFIG_HOME/rio/themes/ptm.toml`.

The post-hook sets `theme = "ptm"` in `$XDG_CONFIG_HOME/rio/config.toml`: it replaces an existing theme assignment, prepends one if absent, or creates the config if missing. Back up any existing configuration first; the hook does not create backups. It requires Bash, `grep`, and `sed` and uses `~/.config` if `XDG_CONFIG_HOME` is unset.

No restart or reload command is issued by the hook.
