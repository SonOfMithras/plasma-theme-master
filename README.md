# Plasma Theme Master

A powerful, unified tool for managing KDE Plasma themes and Kvantum day/night cycles.

[View Changelog](CHANGELOG.md)

## Features

### 🌙 Kvantum Day/Night Scheduler
- **Solar Scheduling**: Automatically calculates sunrise and sunset times based on your latitude and longitude.
- **Custom Scheduling**: Set your own specific times for day and night modes.
- **Auto-Switching**: Automatically switches your Kvantum theme and Global Theme preference (Day/Night) in the background.

### 🎨 Advanced Global Theme Editor
- **Theme Cloning**: Easily clone existing or system-wide Global Themes to your user directory for customization.
- **User-Friendly Editor**: 
    - Customize individual components: **Colors, Cursors, Icons, Plasma Styles, and Application Styles**.
    - Lists all installed sub-themes in convenient dropdowns.
    - **Window Decorations**: Robust support for **Aurorae** themes, with automatic handling of engine prefixes (e.g., `__aurorae__svg__`).
      > **Warning**: Ensure the **Window Decoration Engine** matches the chosen theme (e.g., use `org.kde.kwin.aurorae` for Aurorae themes). Mismatches may cause display issues.
- **Raw Editor**: Full access to the `contents/defaults` INI file for power users.
- **Restore Defaults**: Safely revert theme changes to their original state using automatic backups.

### 📊 Activity Logging
- **New in v0.1.0**: Track all background actions and app changes.
- View logs via the GUI or CLI (`plasma-theme-master log`).

## Requirements

- **Python 3.6+**
- **PySide6**: `pip install PySide6`
- **Kvantum**: The `kvantummanager` tool must be installed and in your PATH.
- **KDE Plasma**: Tested on Plasma 6.

## Installation

### System-Wide Install (Recommended)
This will install the application to `/opt/plasma-theme-master` and create a desktop entry for tailored integration with KDE Plasma.

1. Clone or download this repository.
2. Run the install script:
   ```bash
   sudo ./install.sh
   ```
3. Launch "Plasma Theme Master" from your application menu or run `plasma-theme-master` from the terminal.

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
- Launch the application.
- **Scheduler Tab**:
    - Enter your Latitude and Longitude (e.g., `40.7128`, `-74.0060`).
    - **Daytime Offset**: Adjusts the sensitivity of the day/night switch.
        > **Note**: To match Plasma's default native delay, set this slider all the way to the right.
    - **Theme Mode**: Choose between **Auto (Follow Schedule)** or **Static Mode** (Light/Dark).
        - **Auto**: Follows the calculated schedule using Plasma's native Day/Night transition.
        - **Static**: Enforces a specific theme regardless of time.
    - Select your preferred Day and Night Kvantum/Global themes.
    - Click "Refresh & Apply Now" to test immediately.
    - Settings are saved automatically.
- **Global Theme Editor Tab**:
    - Select a theme from the list.
    - Click **Clone** to create a mutable copy (appends `-U` by default).
    - Use the **Easy Editor** to change specific theme elements like Colors or Window Decorations.
    - Click **Save Changes** to apply.
    
### GUI Layout & Menus
- **Files Menu**: Quickly open your `~/.local/share/plasma/look-and-feel/` or config folder.
- **Help Menu**: View the Activity Log (in reverse chronological order), see About info, or run the **Uninstaller**.
- **Hamburger (☰)**:
    - **Clear Config**: Reset application settings to defaults.

### CLI
The CLI is perfect for scripting or cron jobs.
- **Set Configuration**:
  ```bash
  plasma-theme-master scheduler --lat 40.7128 --lon -74.0060 --day-theme Graphite --night-theme GraphiteDark
  ```
- **Apply Current State** (Run this in cron/autostart):
  ```bash
  plasma-theme-master scheduler --apply
  ```
- **List Themes**:
  ```bash
  plasma-theme-master theme list
  ```
- **View Log**:
  ```bash
  plasma-theme-master log
  ```

## Automation

### Option 1: Systemd Service (Recommended)
This method runs a lightweight background daemon that checks the schedule every minute.
1. Enable the user service (installed automatically):
   ```bash
   systemctl --user enable --now plasma-theme-master.service
   ```
2. Check status:
   ```bash
   systemctl --user status plasma-theme-master.service
   ```
3. View logs:
   ```bash
   journalctl --user -u plasma-theme-master.service -f
   ```

### Option 2: Cron Job
If you prefer not to run a daemon, you can use cron to check periodically.
1. Open crontab:
   ```bash
   crontab -e
   ```
2. Add a line to run every minute:
   ```bash
   * * * * * /usr/local/bin/plasma-theme-master scheduler --apply
   ```

## Uninstallation
To remove the application from your system:
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
