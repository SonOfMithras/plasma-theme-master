# Prism Launcher

Generates a **Matugen** color theme using the Fusion widget style for Prism Launcher.

Enable **Prism Launcher** in **PTM Settings -> Templates**. The manifest writes both installation paths:

- Native: `$XDG_DATA_HOME/PrismLauncher/themes/Matugen/theme.json`
- Flatpak: `~/.var/app/org.prismlauncher.PrismLauncher/data/PrismLauncher/themes/Matugen/theme.json`

Install Prism Launcher and select **Matugen** in its theme settings manually. Both outputs are unconditional; the manifest does not check which installation exists.

Back up any existing files at these paths before enabling. No hook changes launcher settings or triggers a reload.
