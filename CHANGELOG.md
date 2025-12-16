# Changelog

All notable changes to this project will be documented in this file.
## [0.2.2] - 2025-12-15

### Fixed
- **Kvantum Service Crash**:
    - Resolved an issue where `kvantummanager` would crash (core dump) when run by the background service due to missing display access.
    - Implemented a robust fallback that writes directly to `kvantum.kvconfig` if the graphical tool fails.
**Plasma Auto Mode Interference**:
    - Fixed a logic error where enabling "Auto Mode" would inadvertently block Plasma's native scheduler by forcing a manual theme application.
    - The application now delegates Global Theme switching to Plasma when Auto Mode is active.
    
- **GTK Logging**:
    - Fixed duplicate logging for GTK theme application.
    - Prevented the daemon from repeatedly applying/logging the GTK theme when it is already active.
- **Auto Mode Sync**:
    - Improved transition to "Auto Mode" by forcing an immediate theme sync if the current theme is incorrect, ensuring Plasma updates visuals instantly while keeping the native scheduler active.
- **UI Refinements**:
    - Simplifed the Scheduler interface by removing the redundant "Apply Static Theme" button and moving the "Refresh" action to a compact icon in the status header.
- **Robust Scheduler**:
    - Fixed a flaw where the background daemon would ignore Global Theme updates.
    - The daemon now actively syncs the Global Theme if needed and re-enables the native scheduler to maintain state.
- **Code Cleanup**:
    - Refactored the CLI and Daemon logic for better readability and performance.
- **Installer Improvements**:
    - The `install.sh` script now automatically stops any running instances of the daemon before installation to ensure the new version is loaded correctly.

## [0.2.1] - 2025-12-13

### Added
- **GTK Theme Support**:
    - Manage legacy GTK-2/3/4 themes via `~/.config/gtk-3.0/settings.ini` and `~/.config/gtk-4.0/settings.ini`.
    - Dedicated GTK theme dropdowns in Scheduler Tab.
    - CLI support: `--day-gtk` and `--night-gtk` arguments.
- **UI Refinements**:
    - **Refresh Logic**: Reduced auto-refresh interval to 5 seconds for better responsiveness.
    - **Immediate Feedback**: "Apply Static Theme" and "Set Mode" buttons now instantly update the "Active" status labels.
    - **Status Box**: Improved visual hierarchy with larger headers and better padding.
    - **Set Mode Button**: Explicit "Set Mode" button replaces dynamic dropdown updates to prevent settings conflicts.

### Changed
- Refined "Current Status" display to show System Mode (Auto/Static).

## [0.2.0] - 2025-12-11

### Added
- **GUI Refinements**:
    - **New Layout**: Standard Menu Bar ("Help", "Files") and simplified Toolbar.
    - **Native Day/Night Toggle**: Switch between "Plasma Native Day/Night" and "Static Theme" mode directly from the toolbar menu.
    - **Files Menu**: Quick access to Global Theme folders and App Config folders.
    - **Native App Identity**: Correctly identifies as "Plasma Theme Master" with the proper icon on Wayland/Task Managers.
    - **UI Polish**:
        - **Borderless Layout**: Optimized window margins and tab layout for a seamless, professional look.
        - **Safe Scrolling**: Added padding to Scheduler tab to prevent accidental changes to dropdowns while scrolling.
        - **Status Grid**: Improved "Current Status" display with clear Target vs Active theme indicators.
- **Uninstall Feature**:
    - **GUI Uninstaller**: "Uninstall..." option in the Help menu with a clean-up confirmation dialog.
    - `uninstall.sh` CLI flags (`--clean`/`--keep`) for scripted removal.
- **Activity Logging System**:
    - Backend logging to `~/.local/share/plasma-theme-master/activity.log`.
    - CLI `log` command to view recent activity.
    - GUI "View Activity Log" dialog with **reverse chronological order** (newest first).
- **Improved Installation**:
    - `install.sh` ensures `uninstall.sh` is strictly packaged.
    - `install.sh` now performs a clean installation by removing old files first.

### Changed
- **Scheduler Tab**:
    - **Consolidated Controls**: "Theme Mode" (Auto/Static) is now directly in the Scheduler tab.
    - **Real-Time Updates**: Status updates immediately upon configuration changes.
- **Theme Management**:
    - Improved error messages for system theme backup permissions (prompts to Clone instead of crashing).
    - Standardized "Reapply Current Theme" to work alongside the new Mode Toggle.


### Changed
- Standardized file paths and service management.
- Improved CLI `daemon` robustness and error logging.

## [0.0.1] - Initial Release

### Features
- **Solar Scheduler**: Automatically calculate day/night times based on geolocation.
- **Kvantum Switching**: Switch Kvantum themes based on schedule.
- **Global Theme Editor**: Clone and edit Plasma Global Themes.
- **GUI**: PySide6-based interface for easy configuration.
- **CLI**: Command-line interface for scripting and background automation.
- **Systemd Integration**: Background service support.
