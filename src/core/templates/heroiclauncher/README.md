# Heroic Games Launcher

This template renders `matugen.css` into Heroic's custom themes folder. Heroic does not pick
up a rendered theme automatically, it has to be selected once, for either install:

1. Open Heroic, go to **Settings > Accessibility > Custom Themes Path**, and set it to the
   folder the template renders into (see below, differs for native vs Flatpak).
2. In the theme dropdown at the top of that same settings page, select `matugen.css`.

Heroic remembers the selection after that. Future theme changes update `matugen.css` in place,
no need to reselect it.

## Native install

Custom Themes Path: `~/.config/heroic/themes`

## Flatpak install (`com.heroicgameslauncher.hgl`)

Flatpak's sandbox keeps its own isolated config directory, so the `heroiclauncher-flatpak`
entry in `template.toml` renders a second copy straight into it, no `flatpak override` needed:

Custom Themes Path: `~/.var/app/com.heroicgameslauncher.hgl/config/heroic/themes`

Both entries are skipped automatically if the matching install is not present
(`requires_path`), so enabling this template is safe regardless of which install you use, or
whether you have both.
