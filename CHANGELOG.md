# Changelog

All notable changes to this project will be documented in this file.

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
