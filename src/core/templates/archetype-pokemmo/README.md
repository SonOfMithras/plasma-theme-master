# Archetype (PokeMMO)

Synchronizes the **Archetype** PokeMMO theme with PTM's active color palette.

When the PTM palette changes, this community template renders matching colors and updates only Archetype's exposed color constants in `CHOOSE_YOUR_COLORS.xml`.

## Requirements

- PTM v5+
- PokeMMO desktop client
- Archetype Theme
- Python 3

**PokeManager is supported but not required.**

## Color ownership

While **Archetype (PokeMMO)** is enabled:

- PTM owns Archetype colors.
- PokeManager can still install or update Archetype, manage other Archetype settings, and manage PokeMMO mods.
- If you want to manually customize Archetype colors, disable this PTM template first.

Disabling the community template stops future color synchronization. PTM's current community-template manifest does not provide an undo hook, so Archetype keeps the last applied colors until you change them again.

## What changes

The template renders a small palette file into PTM's cache. Its hook then patches only existing named `<constantDef>` color values in Archetype's:

`theme/CHOOSE_YOUR_COLORS.xml`

The complete Archetype XML file is not replaced.

Normal UI roles follow PTM's resolved Material palette. HP and warning colors keep semantic green, yellow, and red values so they remain recognizable in both dark and light mode.

If PokeMMO is already running, press **Ctrl+F5** in PokeMMO after a palette change to reload Archetype.

## Linux install discovery

The hook checks:

1. `POKEMMO_ARCHETYPE_COLORS_FILE`, when explicitly set
2. the standard PokeMMO Flatpak data location
3. `~/PokeMMO/`
4. `~/.local/share/PokeMMO/`

The official portable Linux client may be extracted anywhere. For a non-standard portable location, set `POKEMMO_ARCHETYPE_COLORS_FILE` to the exact `CHOOSE_YOUR_COLORS.xml` path in the environment that launches PTM.

## Tested

- PTM v5.0.1
- Archetype Theme v1.8, theme revision 8
- PokeMMO Linux Flatpak
- CachyOS
- Hyprland / Wayland

Verified with dark mode, light mode, wallpaper-derived `m3-content`, a second wallpaper palette, and repeated template application.

## AI note

**This template was fully vibecoded using AI assistance.**

## Upstream

- PTM: <https://github.com/ptm-dev/ptm>
- Archetype: <https://github.com/ssjshields/archetype>
- PokeMMO: <https://pokemmo.com/>
- PokeManager, optional: <https://github.com/Ryukotsuki/Poke-Manager>

This community template is not an official component of PokeMMO, Archetype, or PokeManager.
