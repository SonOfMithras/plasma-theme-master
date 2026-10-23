# Pywalfox

Writes a Pywal-style color palette for Firefox theming through Pywalfox.

## Setup

1. Install the [Pywalfox](https://addons.mozilla.org/en-US/firefox/addon/pywalfox/) Firefox extension. The `python-pywalfox` package and its native host are not needed: PTM is the native host.
2. Enable **Pywalfox** in **PTM Settings -> Templates**.
3. Apply a theme (change the palette or re-apply templates). PTM writes the palette and installs its native-messaging manifest at `~/.mozilla/native-messaging-hosts/pywalfox.json`.
4. Restart Firefox once so it picks up the host.

See [Firefox](https://docs.ptm.dev/v5/theming/app-theming/#firefox) in the PTM docs for the host, its `ptm firefox-theme` commands, multiple profiles, and migrating from the Python host.

## Output

The output is `$XDG_CACHE_HOME/wal/colors.json`. Back up an existing file first: this shared Pywal cache file is replaced, which can affect other tools that consume it. The palette includes the wallpaper path and selected color slots; several slots are intentionally empty, so it is not a complete general-purpose Pywal palette.

After rendering, the manifest requests PTM's built-in `firefox-theme` post-action, which pushes the colors to every running Firefox profile. There is no shipped shell hook.
