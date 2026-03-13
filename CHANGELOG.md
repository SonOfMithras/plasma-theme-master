# Changelog

All notable changes to this project will be documented in this file.

## [1.1.7] - 2026-03-11

### Added
- **Material you tab**: Added a new tab to the dashboard for Material You integration - with Chroma, Tone, and Color Scheme (Scheme Variant) settings directly from the KDE Material You Colors plasmoid.
- **Nascent Fedora Integration**: Added support for Fedora Linux.

### Fixed
- **Theme Day/Night Sync**: Fixed an issue where the theme would not sync to the correct day or night theme despite Plasma being correct. ThanX Daylight Savings Time for the confusion. (Considering a method of increased integration witht KDE's internal day/night sync)
- **Added new Deps to install.sh**:
  - Added `python-pipx` to the Arch Linux dependencies.
  - Added `python-pipx` to the Debian/Ubuntu dependencies.

## [1.1.6] - 2026-03-05
### Added
- **Zed Code Editor Integration**:
  - Automatically generates and applies a "Plasma Master" theme to the Zed editor (`~/.config/zed/themes/plasma-master.json`).
  - Sets Zed to use this theme in its `settings.json`.
  - Added GUI and CLI controls (`sync-enable zed`) for this feature.

## [1.1.5] - 2026-03-03
### Changed
- Bugfixes and improved material you integration.
- Transformed the daemon into an event-driven service utilizing `QTimer` and `QFileSystemWatcher` instead of a continuous blocking `while` loop, improving efficiency.

### Added
- **Material You Icons Integration**:
  - Automatically extracts the icon theme assigned in the current Day or Night Global Theme and passes it natively to `kde-material-you-colors` via the `--iconslight` and `--iconsdark` arguments.
  - Vencord, Btop and Vicinae Universal theme Integration

### Fixed
- **Universal Sync Race Condition**:
  - Resolved an issue where statically forcing Day or Night mode would sync outdated Universal App colors (VS Code, Discord, Kitty) because the Material You generation script had not finished generating the new colors yet.
  - The background daemon now acts as a definitive failsafe, always triggering a universal sync regardless of the `Auto LookAndFeel` toggle whenever it detects that `kde-material-you-colors` has successfully updated the active KDE color scheme.
  - Defeated the `KSharedConfig` memory cache that was causing `UniversalThemeExporter` to reuse stale hex values if it queried the `.colors` file multiple times within the same execution cycle.

## [1.1.4] - 2026-02-27
### Added
- **Material You Colors Integration**:
  - Automatically generate and apply Material You based color schemes (`MaterialYouLight` and `MaterialYouDark`) when the theme switches.
  - New "Material You Colors" sub-menu in the Help menu to install/upgrade the `kde-material-you-colors` script via `pipx`.
  - Toggle autostart for the Material You generator directly from the Help menu.
  - Checkbox in Dashboard to "Override color scheme with Material You" without needing to edit your Global Theme.
  - Universal Theme Sync now correctly reads from generated Material You colors if the override is enabled.

### Changed
- **Universal Theme Sync**:
  - Removed old "Generic CSS" (universal.css) export and UI elements to declutter the application.
- **Kitty Integration**:
  - Improved theme synchronization logic to generate separate `light-theme.auto.conf` and `dark-theme.auto.conf` files based on the global theme settings.
  - Removed the `include plasma-colors.conf` directive from `kitty.conf` to prevent conflicts, allowing Kitty to manage theme switching natively or via its own configuration.
  - `exportToKitty` now sources colors directly from the configured Day and Night Global Themes rather than the current system state, or from Material You generated schemes if that feature is enabled.

### Fixed
- **Klassy Window Decoration Integration**:
  - Resolved issue where Klassy Window Decorations were not updating automatically with the day/night cycle. Implemented state tracking for Klassy to detect mismatches and trigger updates.
  - Ensured Universal Theme Sync (CSS injection) is triggered when Klassy window decorations are updated.
- **CLI**: Added Klassy status fields to the `status` CLI command for better debugging.

## [1.1.3] - 2026-02-17

### Added
- **Klassy Window Decoration Integration**:
  - Full support for setting Day/Night **Klassy presets** via the Dashboard.
  - Automatically applies presets using `klassy-settings` when theme changes.
  - Reads presets from dynamic config path (`~/.config/klassy/windecopresetsrc`).
- **Global Theme Editor Enhancements**:
  - **Dynamic Engine Discovery**: Now scans all installed global themes to populate the "Window Decoration Engine" dropdown (supports Klassy, Lightly, etc.).
  - **Dynamic Application Styles**: Now lists all available system widget styles (via `QStyleFactory`) instead of a hardcoded list.

### Fixed
- **Global Theme Editor**: Correctly detects `org.kde.klassy` and other installed window decorations regardless of them appearing in Global themes.

## [1.1.2] - 2026-02-10

### Added
- **Universal Theme Sync**: CSS based theme sync for multiple applications.
  - Automatically syncs Plasma colors to **VS Code** (Code, OSS, VSCodium), **Firefox** (incl. Zen Browser), **BetterDiscord**, **Kitty**, and **Obsidian**.
  - **Backup System**: Automatically backs up configuration files before modification.
  - **Restore System**: `sync-restore` command to revert changes.
  - **CLI Management**: `sync-enable`, `sync-disable`, `sync-list`, `sync-restore`, and `sync-universal` commands.
- **Specific App Support**:
  - **Code - OSS** and **VSCodium** support added.
  - **Zen Browser** support (via Firefox sync).
- **Config**:
  - All sync apps are **Disabled by Default** to prevent unwanted changes. Users must enable them via CLI.

### Changed
- **CLI Help**: Updated with new sync commands.
- **Daemon**: Now triggers universal sync automatically when the theme changes (if apps are enabled).
- **BetterDiscord Integration**:
  - Updated import logic: User custom imports now take precedence over default addons.
  - "Base Material Theme" is now an optional toggle that loads *before* user customization.
  - Renamed checkbox to "Use Material UI (recommended for cohesive use of colors)" for clarity.
- **Config Reloading**: The color extractor now forces a reload of the KDE configuration from disk (`reparseConfiguration`) to prevent stale data usage.

## [1.1.1] - 2026-01-31

### Added
- **Arch Linux Dependency Checking**: `install.sh` now fully supports Arch Linux, including:
  - Checking for standard build dependencies.
  - Detecting and offering to install `kvantum` and `libadwaita-without-adwaita`.
  - Handling AUR helpers (`yay` or `paru`).
- **Restart Plasma Shell**: New option in the Help menu to safely restart `plasmashell` (`--replace`) to fix visual glitches in the shell.
- **Background Service Control**: New toggle in the Help menu to easily Enable/Disable the background daemon.

### Changed
- **Install Script**:
  - Significantly improved visual presentation with colors and clear status messages.
  - Now explicitly restarts the user service (`systemctl --user restart`) during installation to ensure the latest version is running.
- **Config Folder Access**: "Open App Config Folder" now correctly opens `~/.config/plasma-theme-master/` instead of the parent directory.
- **Global Theme Editor**: Added `kvantum-dark` to the list of available application styles.

### Fixed
- **Kvantum Theme Refresh**: Solved an issue where Kvantum themes would not visually update without a session restart. The application now forces a style reload (via `kdeglobals`) before applying global themes.

## [1.1.0] - 2026-01-25

### Added
- **Flatpak Integration**:
  - Full support for managing Flatpak GTK themes via `flatpak override`.
  - Checking and setting up environment permissions automatically.
  - Independent configuration for Flatpak Day/Night themes.
  - Option to sync Flatpak theme with System GTK theme.
- **CLI Commands**:
  - `flatpak-status`, `flatpak-setup`
  - `set-flatpak`, `set-flatpak-day`, `set-flatpak-night`, `set-flatpak-follow`
- **GUI Features**:
  - New "Flatpak Settings" dialog in Help menu.
  - "Check for Updates" button integrated into the About dialog.

### Changed
- **About Dialog**: Updated version information and features list.

## [1.0.5] - 2025-12-27

### Fixed
- **Daemon Crash**: Solved an issue where the daemon would crash on startup due to missing display environment variables. The service now waits for `graphical-session.target`.
- **Environment Handling**: Added logic to ensure `lookandfeeltool` receives the correct `DISPLAY` and `WAYLAND_DISPLAY` variables.

### Changed
- **Code Refactoring**: Moved CLI argument handling logic from `main.cpp` to a dedicated `CLIHandler` class for better maintainability.
- **Cleanup**: Removed legacy `reference_python` directory and unused `requirements.txt`.
- **Build System**: Updated `install.sh` to support incremental builds, speeding up reinstallations.

### Added
- **Command Help**: Improved help output organization.

## [1.0.0] - 2025-12-17

**Major Rewrite**: This release marks the transition from the legacy Python codebase to a native C++ implementation using Qt6 and KDE Frameworks 6.

### Added
- **Native C++ Core**: Completely rewritten backend for improved performance, stability, and lower resource usage.
- **Modern Qt6 GUI**: Redesigned functional interface with tabs for Dashboard, Logs, and Editors.
- **Global Theme Editor**:
  - Detailed component editing (Plasma Style, Window Decorations, Icons, Cursors, etc.).
  - "Easy Editor" with dropdowns and "Raw Editor" for direct file modification.
  - **Backup & Restore**: Automatically creates backups of theme defaults and allows restoration.
  - **Theme Cloning**: Easily clone existing global themes to create custom variants.
  - **Aurorae Support**: Specific handling for Aurorae window decoration themes.
- **Menu Bar**: Native menu bar for quick access to theme folders, config, and help.
- **Robust Sync**: Improved synchronization logic for Kvantum and GTK themes.
- **CLI Enhancements**:
  - Unified command structure (`set-*-day`/`night` naming).
  - New `log` command with filtering.
  - New `clone-global` command.
  - `uninstall` command for easy removal.
- **Systemd Integration**: Automatic user service installation for background monitoring.

### Changed
- **CLI Commands**: Renamed `light`/`dark` commands to `day`/`night` for consistency with the UI.
- **Configuration**: Now uses KDE's native `KConfig` system for robust settings management.
- **Logging**: Centralized, rotating log file with GUI viewer.

### Removed
- Python dependencies (no longer requires Python runtime or python-libs).
