# Supersonic

Keeps [Supersonic](https://github.com/supersonic-app/supersonic) aligned with the
active PTM palette. Native and Flatpak installations are supported.

## Setup

1. Launch Supersonic once so its configuration directory exists, then enable
   the Supersonic template in PTM.
2. Open **Settings > Appearance** in Supersonic and select **PTM** from the
   Theme dropdown. Restart Supersonic first if the theme is not listed yet.

Supersonic remembers that selection. Later PTM palette changes rewrite the
theme and use Supersonic's `--reload-theme` IPC command to refresh a running
instance without interrupting playback. If Supersonic is closed, the hook does
nothing and the updated theme is loaded on its next start.

Live reload requires Supersonic 0.21.0 or newer. Older releases can still use
the generated theme but need to be restarted, or switched to another theme and
back, after a palette change.

## Output paths

- Native: `$XDG_CONFIG_HOME/supersonic/themes/ptm.toml`
- Flatpak: `~/.var/app/io.github.dweymouth.supersonic/config/supersonic/themes/ptm.toml`

The active PTM palette is written into both Supersonic color variants so
the result stays consistent with Supersonic's Dark, Light, and Auto modes.
