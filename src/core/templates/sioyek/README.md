# sioyek

Generate PTM colors for Sioyek's background, highlights, interface, and status bar.

## Setup

1. Install Sioyek and ensure `$XDG_CONFIG_HOME/sioyek/prefs_user.config` exists. The manifest requires `$XDG_CONFIG_HOME/sioyek`, and the hook fails if the preferences file is missing.
2. Enable **sioyek** in **PTM Settings → Templates**.

The template writes `$XDG_CONFIG_HOME/sioyek/themes/ptm.config`. Its Bash hook converts generated hex colors to Sioyek's normalized RGB values, then appends a `source` line for the generated file to `prefs_user.config` unless a matching line is already present.

The hook requires `awk`, `grep`, and `mv`. Back up your preferences before enabling: no backup is created. The generated theme is replaced on regeneration. The hook uses `~/.config` if `XDG_CONFIG_HOME` is unset and does not issue a reload or restart command.
