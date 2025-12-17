# Plasma Theme Master

**Version 1.0 (C++ Edition)**
*Re-engineered from the ground up for performance and reliability.*

Plasma Theme Master is a powerful utility for KDE Plasma recently introduced users to a native Day/Night cycle that automatically switches your desktop theme between Day and Night modes.

This application allows you to set custom kvantum themes for day or night mode as well as custom gtk themes for day or night mode so that you can maintain a cohesive aesthetic across your system. It also features a robust Global Theme Editor for customizing and backing up global themes in case you like some parts of a global theme but not others. (example: I want the Breeze Global theme but I would like it to always use the tela icon theme and the breeze dark window decorations)

## Features

- **Automatic Day/Night Switching**: seamless transition of Global, Kvantum, and GTK themes.
- **Solar Calculation**: Automatically calculates sunrise and sunset times based on the long and lat provided to vial plasma's integrated day/night cycle.
- **Solar Offset**: I noticed that plasma switched my global theme 30 mins after sunset so I added an offset to allow manual adjustment to sync with the time plasma acctually changes the theme.
- **Global Theme Editor**: Customize theme components (Plasma Style, Window Decorations, Icons, etc.) with ease.
- **Backup & Restore**: Automatically backs up theme defaults and allows one-click restoration.
- **Theme Sync**: Keeps Kvantum and GTK themes in sync with your Global Theme.
- **Daemon Mode**: Runs efficiently in the background to monitor time changes and swap to the correct themes. Lightweight and resource efficient. ~2mb Memory Usage.
- **Logging**: Centralized, log file with GUI viewer.

## Installation

### Prerequisites
- KDE Plasma 6
- Qt 6
- CMake
- KConfig, KCoreAddons

### Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/SonOfMithras/plasma-theme-master.git
   cd plasma-theme-master
   ```

2. Run the installation script:
   ```bash
   ./install.sh
   ```
   This script will build the application, install it to `/usr/bin`, and register a systemd user service.

## Usage

### GUI
Launch the application from your application menu or terminal:
```bash
plasma-theme-master
```
- **Dashboard**: View system status, sun times, and manually override themes.
- **Global Theme Editor**: Select a global theme, edit its components, and save changes.
- **Logs**: View application logs for debugging.

### CLI Commands
The application provides a comprehensive Command Line Interface (CLI) for scripting and advanced usage.

| Command | Description |
|---|---|
| `status` | Show current solar times, mode, and active themes. |
| `day` | Force switch to Day mode (applies configured Day themes). |
| `night` | Force switch to Night mode (applies configured Night themes). |
| `set-global-day <theme>` | Set the Global Theme for Day mode. |
| `set-global-night <theme>` | Set the Global Theme for Night mode. |
| `set-kvantum-day <theme>` | Set the Kvantum Theme for Day mode. |
| `set-kvantum-night <theme>` | Set the Kvantum Theme for Night mode. |
| `set-gtk-day <theme>` | Set the GTK Theme for Day mode. |
| `set-gtk-night <theme>` | Set the GTK Theme for Night mode. |
| `clone-global <src> <dest>` | Clone a Global Theme to a new name. |
| `log [-n lines] [--errors]` | View application logs. |
| `daemon` | Run the background daemon (handled by systemd usually). |
| `uninstall` | Remove the application and service. |

## Tips & Tricks

- **Location Settings**: The application automatically retrieves your Latitude and Longitude from KDE Plasma's System Settings ("Night Light" or "Location" settings). You **must** have your location configured in Plasma for the solar calculation to be accurate.
- **Backup Reset**: If you mess up a theme in the Editor, use the "Restore Original Defaults" button to revert to the state before you first edited it.
- **Service Control**: The background service is managed by systemd. You can control it manually if needed:
  ```bash
  systemctl --user status plasma-theme-master
  systemctl --user restart plasma-theme-master
  ```
- **Config Reset**: If the app misbehaves, you can reset all settings via `Help > Clear Config` in the GUI.

## Credits

**Author**: Ammar Al-Riyamy (SonOfMithras)
**GitHub**: [https://github.com/SonOfMithras](https://github.com/SonOfMithras)

## License
MIT License
