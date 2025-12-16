# Plasma Theme Master

A powerful, unified tool for managing KDE Plasma themes, Global Themes (Look and Feel), and Kvantum day/night cycles.

[View Changelog](CHANGELOG.md)

## Features

### 🌙 Robust Day/Night Scheduler
- **Integrated Control**: Manages **Kvantum**, **Global Themes (Plasma)**, and **GTK** themes simultaneously.
- **Solar Scheduling**: Automatically calculates sunrise and sunset times based on your latitude and longitude.
- **Reliable Sync**: The background daemon actively monitors and enforces your chosen themes, correcting any drifts or Plasma sync failures automatically.
- **Smart Auto Mode**: Seamlessly integrates with Plasma's native "Auto" coloring mode while ensuring themes apply instantly upon transition.

### 🖌️ Unified Styling
- **GTK Support**: Syncs your GTK-2/3/4 themes with your Plasma schedule for a cohesive desktop look.
- **Global Theme Editor**: 
    - Clone system themes to create custom variants.
    - Edit specific components: **Colors, Cursors, Icons, Plasma Styles, and Application Styles**.
    - Safely revert changes with built-in backups.

### 🎨 Advanced Global Theme Editor
- **Theme Cloning**: Easily clone existing or system-wide Global Themes to your user directory for customization.
- **User-Friendly Editor**: 
    - Customize individual components: **Colors, Cursors, Icons, Plasma Styles, and Application Styles**.
    - Lists all installed sub-themes in convenient dropdowns.
    - **Window Decorations**: Robust support for **Aurorae** themes, with automatic handling of engine prefixes (e.g., `__aurorae__svg__`).
      > **Warning**: Ensure the **Window Decoration Engine** matches the chosen theme (e.g., use `org.kde.kwin.aurorae` for Aurorae themes). Mismatches may cause display issues.
- **Raw Editor**: Full access to the `contents/defaults` INI file for power users.

### 📊 Activity Logging
- Logs all theme transitions and daemon activities.
- View logs directly in the GUI or via the CLI.

## Requirements

- **Python 3.6+**
- **PySide6**: `pip install PySide6`
- **Kvantum**: The `kvantummanager` tool must be installed.
- **KDE Plasma 6.5**

## Installation

### Option 1: From Release Archive (Recommended)
If you downloaded the `plasma-theme-master-v[version].tar.gz` release:

1. Extract the archive:
   ```bash
   tar -xzf plasma-theme-master-v[version].tar.gz
   cd plasma-theme-master
   ```
2. Run the installer:
   ```bash
   sudo ./install.sh
   ```
3. Follow the prompts to enable the background service.

### Option 2: From Source
1. Clone this repository.
2. Run the installer:
   ```bash
   sudo ./install.sh
   ```
3. Follow the prompts to enable the background service.

### User Install (Manual)
1. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```
2. Run from source:
   ```bash
   python3 src/main.py
   ```

## Usage

### GUI
Launch **Plasma Theme Master** from your application menu.

- **Scheduler Tab**:
    - **Configuration**: Set your Latitude/Longitude for solar calculations or define custom times.
    - **Mode**: Switch between **Auto (Scheduled)**, **Static Light**, or **Static Dark**.
    - **Status**: View current cycle state (Day/Night) and target themes.
    - **Themes**: Select your preferred pairings for Day and Night. 
    - **Refresh**: Click the ⟳ icon to force a status check.
- **Global Theme Editor Tab**:
    - **Clone**: Create editable copies of current themes.
    - **Edit**: Customize theme components (Colors, Window Decorations, etc.).
    - **Save**: Apply your changes immediately.

### CLI
The CLI has been streamlined for efficiency and scripting.

- **Configure & Schedule**:
  ```bash
  plasma-theme-master scheduler --lat 40.7 --lon -74.0 --day-theme Graphite --night-theme GraphiteDark
  ```
- **Force Check & Apply**:
  ```bash
  plasma-theme-master scheduler --apply
  ```
- **Run Daemon** (Foreground):
  ```bash
  plasma-theme-master daemon
  ```
- **List/Clone Themes**:
  ```bash
  plasma-theme-master theme list
  plasma-theme-master theme clone --source "Breeze" --dest "MyBreeze-U"
  ```
- **View Logs**:
  ```bash
  plasma-theme-master log
  ```

## Automation

### Systemd Service (Default)
The installer sets up a user-level systemd service that checks the schedule every minute.

- **Status**: `systemctl --user status plasma-theme-master.service`
- **Logs**: `journalctl --user -u plasma-theme-master.service -f`
- **Restart**: `systemctl --user restart plasma-theme-master.service`

### Option 2: Cron Job (Periodic Check)
If you prefer not to run a daemon, you can use cron to check periodically.
1. Open crontab:
   ```bash
   crontab -e
   ```
2. Add a line to run every minute:
   ```bash
   * * * * * /usr/local/bin/plasma-theme-master scheduler --apply
   ```

### Advanced Scheduling (Weekend Mode)
You can use cron to change **what themes are used** on specific days, while letting the daemon (or the periodic check above) handle the actual day/night switching.

**Command breakdown:**
- `--day-theme / --night-theme`: Sets the **Kvantum** (Application Style) theme.
- `--day-global / --night-global`: Sets the **Global Theme** (Look and Feel).

**Example: Weekend vs. Work Week**
Add these lines to your crontab to switch to a relaxed theme on Friday evening and back to a professional theme on Sunday night:

```bash
# Friday 5:00 PM: Switch to "Nordic" for the weekend
0 17 * * 5 /usr/local/bin/plasma-theme-master scheduler --day-theme "Nordic" --night-theme "Nordic-Dark" --day-global "Nordic" --night-global "Nordic"

# Sunday 8:00 PM: Revert to "Graphite" for the work week
0 20 * * 0 /usr/local/bin/plasma-theme-master scheduler --day-theme "Graphite" --night-theme "GraphiteDark" --day-global "Graphite" --night-global "Graphite-dark"
```

## Uninstallation

To remove the application and service, you can:
1. Open the **Help** menu in the GUI and select **Uninstall**.
2. Or run the removal script from the terminal:
   ```bash
   sudo ./uninstall.sh
   ```

## Configuration
Settings are stored in `~/.config/plasma-theme-master/config.json`.

## Troubleshooting

### Window Decorations Not Displaying Correctly
If your window decorations look broken or revert to default:
- Check that the **Window Decoration Engine** matches the theme you selected.
- For most downloaded themes (Graphite, Catppuccin, etc.), the engine should be `org.kde.kwin.aurorae`.
- For standard themes (Breeze, Oxygen), use their respective engines (e.g., `org.kde.breeze`).
