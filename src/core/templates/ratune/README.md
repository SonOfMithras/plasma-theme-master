# Ratune

Generate a PTM color theme for Ratune's interface.

## Setup

1. Install Ratune and create its `$XDG_CONFIG_HOME/ratune/config.toml` before enabling this template. The hook fails if that file is missing.
2. Enable **Ratune** in **PTM Settings → Templates**.

The template writes `$XDG_CONFIG_HOME/ratune/theme.generated.toml`. The Bash post-hook replaces the `[theme]` section in `config.toml`, or appends one if absent, with the generated colors and `preset = "static"`. Other sections and comments outside `[theme]` are retained. Back up your configuration first: no backup is created, and custom theme settings are replaced.

The hook requires `awk` and standard command-line tools (`grep`, `mktemp`, `cmp`, `cat`, `rm`). It writes only when content changes and writes through existing symlinks, preserving the config file's permissions. It does not send a reload command or restart Ratune.

The hook uses `~/.config` when `XDG_CONFIG_HOME` is unset.
