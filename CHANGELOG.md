# Changelog

All notable changes to this project will be documented in this file.

## [0.2.0] - 2025-12-11

### Added
- **GUI Refinements**:
    - **New Layout**: Standard Menu Bar ("Help", "Files") and simplified Toolbar.
    - **Native Day/Night Toggle**: Switch between "Plasma Native Day/Night" and "Static Theme" mode directly from the toolbar menu.
    - **Files Menu**: Quick access to Global Theme folders and App Config folders.
    - **Native App Identity**: Correctly identifies as "Plasma Theme Master" with the proper icon on Wayland/Task Managers.
    - **UI Polish**:
        - **Borderless Layout**: Optimized window margins and tab layout for a seamless, professional look.
        - **Safe Scrolling**: Added padding to Scheduler tab to prevent accidental clicks on dropdowns while scrolling.
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
    - **Consolidated Controls**: "Theme Mode" (Auto/Static) is now directly in the Scheduler tab, removing the need for a toolbar toggle.
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
