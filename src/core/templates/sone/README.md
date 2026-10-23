# Sone

Themes [Sone](https://github.com/lullabyX/sone) — the native Linux desktop
client for TIDAL — with the active PTM palette.

Sone stores its theme in a single `theme.json` file. PTM writes the file
with the current palette colors; Sone's built-in file watcher picks the change
up while running, so a theme change updates the player without a restart.

## Setup

1. Launch Sone once so its configuration directory exists, then enable the
   Sone template in PTM.
2. In Sone, open **Settings → Themes** and select **Custom**. Sone will then
   follow whatever PTM wrote into `theme.json`.

No reload command is needed: Sone reads `theme.json` at startup and watches
the file for the rest of the session. If you are running a Sone that was
already on the *Custom* theme, every PTM theme change updates it live.

## Output paths

Sone's config directory differs depending on how it was installed, so this
template ships one entry per install method. Each entry has a `requires_path`
guard, so the ones that do not apply to your installation are skipped:

- Native (AUR, `.deb`, `.rpm`, AppImage):
  `$XDG_CONFIG_HOME/sone/theme.json`
- Flatpak (`io.github.lullabyX.sone`):
  `~/.var/app/io.github.lullabyX.sone/config/sone/theme.json`
- Snap: `~/snap/sone/current/.config/sone/theme.json`

The exact same `theme.json` is written into each path; any of them can be
absent on systems that did not install Sone via that method.

## Theme schema

Only `accent` and `background` are configurable; Sone derives its full light
and dark palettes from those two colors. The PTM template writes `accent`
from `colors.primary` and `background` from `colors.background`, both of which
are mode-aware, so the same file works for light and dark PTM themes.
