# GTK 3 & GTK 4 (Adwaita / Libadwaita / adw-gtk3) Theme Sync

Synchronizes custom Plasma Theme Master and Noctalia color palettes to GTK 3 and GTK 4 applications.

## Overview
When using KDE Plasma's default **Breeze** GTK theme, Plasma provides color syncing through `~/.config/gtk-3.0/colors.css` and `~/.config/gtk-4.0/colors.css`. However, when using **adw-gtk3**, **Adwaita**, or modern **Libadwaita** applications, standard Breeze-suffixed variables are ignored.

This template renders `ptm.css` containing Material Design 3 and standard GTK theme color variables (`@define-color accent_color`, `@define-color window_bg_color`, `@define-color view_bg_color`, etc., plus CSS `:root` variables) and imports it into `~/.config/gtk-3.0/gtk.css` and `~/.config/gtk-4.0/gtk.css`.

It also synchronizes the GNOME desktop color-scheme preference (`"default"` for light mode to prevent applications from being stuck in dark mode, and `"prefer-dark"` for dark mode), and automatically selects between `adw-gtk3` and `adw-gtk3-dark` if an Adwaita-based GTK theme is active.
