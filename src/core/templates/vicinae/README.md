# Vicinae

Generates a PTM theme for Vicinae using the active palette and light/dark mode.

## Setup

Install Vicinae and make its `vicinae` command available on `PATH`, then enable **Vicinae** in **PTM Settings -> Templates**. The Bash post-hook runs `vicinae theme set ptm` after generation to select the theme; there is no separate restart command.

## Output

- `$XDG_DATA_HOME/vicinae/themes/ptm.toml`

The theme references `ptm.svg` in the same directory. The hook can copy an icon when explicitly given a source file as its first argument, without overwriting an existing icon, but the configured hook passes no argument and does not install one. Supply that icon manually if desired. The hook does not edit Vicinae configuration files directly or create backups.
