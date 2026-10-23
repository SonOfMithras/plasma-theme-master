# Halloy

Themes Halloy's interface, message buffers, text, and buttons with PTM colors.

Enable **halloy** in **PTM Settings -> Templates**. The theme is written to `$XDG_CONFIG_HOME/halloy/themes/ptm.toml` (`~/.config/halloy/themes/ptm.toml` by default).

## Setup and hook

Install Halloy and create `$XDG_CONFIG_HOME/halloy/config.toml` before enabling; the Bash hook fails if it is missing. The hook uses `awk` to set the root-level `theme = "ptm"` before any TOML sections, replacing an existing root theme selection or inserting one. Section-local settings are left alone.

Back up your config first; the hook creates no backup. It then attempts `pkill -SIGUSR1 halloy` to notify running Halloy processes, ignoring failures. This requires `pkill` for the reload signal; no application restart is performed.
