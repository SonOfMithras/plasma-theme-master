# velo

Generate a PTM palette for velo, with separate dark and light colors.

Enable **velo** in **PTM Settings → Templates**. The palette is written to `$XDG_CONFIG_HOME/velo/palettes/current-ptm-override.json`.

The Bash post-hook creates or updates `$XDG_CONFIG_HOME/velo/config` with:

```text
palette = "current-ptm-override"
```

It also sets `darkmode = false` for PTM's light mode and `darkmode = true` otherwise. Existing unindented `palette` and `darkmode` assignments are replaced; missing assignments are appended, while other lines are retained. The hook uses `~/.config` if `XDG_CONFIG_HOME` is unset.

The hook requires `awk`, `mktemp`, `cmp`, `mv`, `cat`, `rm`, and `mkdir`. It avoids rewriting unchanged existing configs and writes through existing symlinks. Back up your configuration first, since the hook makes no backup. No reload or restart command is issued.
