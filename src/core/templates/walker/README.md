# Walker

Styles Walker's launcher window, search field, results and previews with the PTM palette.

## Setup

Create your Walker configuration at `$XDG_CONFIG_HOME/walker/config.toml` (normally `~/.config/walker/config.toml`) before enabling **Walker** in **PTM Settings -> Templates**. The hook fails if this file is missing.

## Output and hook

The stylesheet is written to `$XDG_CONFIG_HOME/walker/themes/ptm/style.css`.

The Bash hook changes an existing unindented `theme = ...` assignment in `config.toml` to `theme = "ptm"`, or appends that assignment if none is found. Back up your configuration first: the hook writes through existing symlinks and does not create a backup. It uses standard shell utilities, including `grep`, `sed`, `cmp` and `mktemp`.

The hook does not restart Walker or issue a reload command.
