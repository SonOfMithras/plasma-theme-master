# Pear Desktop

Themes Pear Desktop's YouTube Music interface with PTM colors.

## Setup

Install Pear Desktop and launch it once to create its configuration, then enable **Pear Desktop** in **PTM Settings -> Templates**. The CSS is written to `$XDG_CONFIG_HOME/YouTube Music/ptm.css`.

The Bash post-hook requires `jq` and standard shell utilities. If `$XDG_CONFIG_HOME/YouTube Music/config.json` exists, it sets `options.themes` to an array containing only the absolute path to the generated CSS. This replaces the entire existing theme list while retaining other JSON settings. If the configuration does not exist, the hook exits without configuring the theme; reapply after the app has created it.

Back up `config.json` and any existing `ptm.css` first. No backup is created; configuration writes preserve symlinks. The hook does not send a reload command to the app.
