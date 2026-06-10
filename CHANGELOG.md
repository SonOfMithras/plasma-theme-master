# Changelog

All notable changes to this project will be documented in this file.

## [2.0.7] - 2026-06-09

### Added
- **Libadwaita & Standard GTK Color Scheme Templates**
  - Added new native template files for GTK4/Libadwaita (`gtk4_colors.css.tpl`) and GTK3 (`gtk3_colors.css.tpl`).
  - Automatically exports clean, unsuffixed CSS variables (e.g. `accent_bg_color`, `window_bg_color`, `view_bg_color`) to standard locations (`~/.config/gtk-4.0/colors.css` and `~/.config/gtk-3.0/colors.css`), bypassing Breeze-specific names.
  - Enables all Libadwaita and standard GTK applications to dynamically respect and live-refresh custom system color palettes.
- **Universal Settings GUI Integration**
  - Added a dedicated checkbox labeled **"GTK & Libadwaita (Non-Breeze Themes only)"** and an associated **Restore** button directly to the **Universal Theming** page in the GUI.
  - Automatically loads, saves, and updates settings dynamically, synchronizing both GTK3 and GTK4 templates.
- **Vesktop Support in Vencord Sync**
  - Updated Vencord sync to automatically copy generated themes to Vesktop directories (`~/.config/vesktop/themes/` and Flatpak location `~/.var/app/dev.vencord.Vesktop/config/vesktop/themes/`) if those directories exist.
  - Cleans up all synchronized Vesktop theme files when Vencord configuration is restored.

### Fixed
- **Breeze Background Daemon Race Condition**
  - Implemented a delayed second-pass execution (using a 2.5-second single-shot timer) during synchronization.
  - Safely overwrites `colors.css` with standard variables *after* KDE Plasma's background daemon finishes regenerates the Breeze-suffixed color variables, preventing the Breeze engine from overriding custom styles.
- **Millennium Theme Template Outline Color Contrast**
  - Updated the Material Design 3 `outline` and `outline-variant` color calculations to interpolate between surface background and high-contrast foreground colors.
  - This resolves contrast issues (e.g. black/invisible text in dark mode and light gray/hard to read text in light mode) in the Millennium Steam theme's top-level menu.

## [2.0.6] - 2026-05-28

### Added
- **Integrated gtk4 live refresh hook**
  - Added a bash-based post-hook script (`gtk4-reload.sh`) to force a live refresh of GTK4/Libadwaita applications.
  - Automatically triggers when the GTK theme settings are changed, toggling the GNOME desktop `color-scheme` via `gsettings` to propagate updates seamlessly.

## [2.0.5] - 2026-05-27

### Fixed
- **Solar Offsets Overriding in KWin NightLight Active State**
  - Resolved an issue where solar offsets (Daytime and Nighttime Offset sliders) were completely ignored when KDE's KWin NightLight DBus property `daylight` was active.
  - Updated theme switching and override logic to bypass DBus and use offset-enabled solar clock calculations (`Solar::isDaytime`) whenever non-zero offsets are set.
  - Ensures manual override auto-restoration ("Re-enable auto-switch at next scheduled cycle transition") triggers exactly at the custom offset boundaries rather than raw system times.

## [2.0.4] - 2026-05-27

### Added
- **Zen Browser Theme Template**
  - Added support for custom Zen Browser stylesheet templates (`userChrome.css.tpl` and `userContent.css.tpl`).
  - Implemented a post-hook script (`zen-browser-apply.sh`) to automatically inject and manage these themes into Zen profiles (supporting both native and Flatpak installations) without duplicating styles.
  - Added an intuitive parent/child checkbox configuration in the **Universal Theming** GUI: a primary "Zen Browser" parent checkbox and an indented sub-option for web settings and content pages (`userContent.css`).
  - Added CLI support allowing the template to be enabled or disabled via `plasma-theme-master sync-enable/disable zen`.
  - Added instructions to set `toolkit.legacyUserProfileCustomizations.stylesheets` to `true` in `about:config` and a note that browser restarts are required for color scheme changes to take full effect.

## [2.0.3] - 2026-05-22

### Added
 **Independent Day/Night Solar Offsets**
  - Replaced the single legacy `SolarPadding` logic with separate `SolarDayOffset` (shifts sunrise time) and `SolarNightOffset` (shifts sunset time) configuration keys.
  - Allows daytime and nighttime cycles to be shifted earlier or later independently, including negative offsets (e.g. starting night mode earlier than sunset).
  - Cleaned up all vestigial `SolarPadding` code from `ThemeReader` and command-line parser.
- **Improved Solar Settings GUI Controls**
  - Flipped the Daytime Offset slider mapping visually so that pulling left represents negative values (sunrise earlier) and pulling right represents positive values (sunrise later), aligning with visual and label intuition.
  - Added a dedicated "Save Offsets" button to persist slider updates, preventing disk thrashing and improving UI responsiveness.
- **Auto-Re-enable Automatic Theme Switching**
  - Added a new configuration option and GUI checkbox: "Re-enable auto-switch at next scheduled cycle transition".
  - If enabled, manually forcing static Day/Night mode (e.g. during a daytime storm) will temporarily override the theme but automatically restore automatic mode at the next scheduled sunrise/sunset transition.
- **CLI Enhancements & Aliases**
  - Added `day` and `night` aliases to manually trigger static light and dark overrides from the command line.
  - Added CLI options `set-offset-day <minutes>` and `set-offset-night <minutes>` to set separate offsets.
  - Added `set-reenable-auto <true/false>` to configure auto-restoration behavior from the terminal.
  - Removed deprecated `set-offset` command in favor of independent day and night offset controls.
- **Millennium Steam (Material Theme) Support**
  - Added support for the Millennium Steam client modding framework (specifically the Material-Theme).
  - Automatically exports space-separated dual-scheme CSS variables directly to `blue.css` (`~/.steam/steam/millennium/themes/Material-Theme/css/main/colors/blue.css`).
  - Overwriting `blue.css` directly bypasses dynamic registration limits, enabling custom color integration when the "Blue" theme option is active inside Steam.
  - Documented setup instructions and manual reload notes in the README.

### Fixed

- **Cleaned up VSCode helper** - Removed the unused `applyWorkspaceSettings()` function and all associated build flags and headers.

## [2.0.2] - 2026-05-01

### Fixed

- **Static Global Theme Enforcement on Boot**
  - Daemon now detects if it's running on boot (via absence of `plasma-theme-masterrc` or `TemplateConfig.toml`).
  - If detected, it applies the static Day/Night global theme configured in Settings, overriding any transient session default.
  - Prevents Plasma from automatically snapping back to Breeze Light on boot when Night mode is configured.
- **Material You Daemon Integration**
  - Configured Material You colors are now applied *after* the global theme is set, ensuring the correct seed is used.
  - Prevents a race condition where Material You colors might be applied to the wrong base theme (e.g. Light colors on a Dark global theme).

## [2.0.1] - 2026-03-27

### Added

- **Material You: Dominant Seed Color Picker** — The Material You tab now shows a visual row of colored swatches representing the dominant colors extracted from the current wallpaper (read from the kde-material-you-colors daemon JSON). Clicking a swatch selects it as the seed color and passes `--ncolor N` to the daemon on apply.
- **`plasma-theme-master-helper-kitty`** — New dedicated C++ helper for Kitty terminal that:
  - Copies the correct day/night auto theme conf to `~/.config/kitty/current-theme.conf`
  - Independently adds `include ./current-theme.conf` to `kitty.conf` on first run
  - Sends `SIGUSR1` to all running kitty instances via `/proc` enumeration so colours reload live

### Fixed

- **Kitty hook name mismatch** — Built-in hook now correctly triggers for template names `kitty`, `kitty_dark`, and `kitty_light` (previously only `kitty` was registered, so the hook was silently skipped for the default `kitty_dark` / `kitty_light` config entries).

## [2.0.0] - 2026-03-26

### Added

- **Template-Based Universal Theming** (matugen-style):
  - All theme export logic replaced with a data-driven `.tpl` file system. Templates live in `/usr/share/plasma-theme-master/templates/` and can be freely customized.
  - New `config.toml` at `~/.config/plasma-theme-master/config.toml` is the single source of truth for which apps are enabled, their output paths, and palette variant (`day` / `night` / `current`).
  - `post_hook` field available in `config.toml` for user-defined custom template entries. Built-in apps run their hooks automatically — no manual configuration required.
- **New Template Files** — shipped for all supported apps:
  - `kitty_light.tpl`, `kitty_dark.tpl` (separate day/night palette resolution)
  - `btop.tpl`, `vicinae.tpl`, `firefox.css.tpl`, `obsidian.css.tpl`
  - `betterdiscord/theme.css.tpl`, `vencord/theme.css.tpl`
  - `zed/theme.json.tpl` (full light + dark theme in one file)
  - `konsole/colorscheme.tpl`
- **Helper Binaries** — small compiled C++ utilities for apps requiring structural logic beyond simple substitution:
  - `plasma-theme-master-helper-vscode` — merges `workbench.colorCustomizations` into VS Code / VSCodium / Antigravity `settings.json`
  - `plasma-theme-master-helper-zed` — patches `~/.config/zed/settings.json` to reference the generated theme
  - `plasma-theme-master-helper-firefox` — discovers all Firefox / Zen profile directories and injects `plasma-colors.css` + `userChrome.css` import
  - `plasma-theme-master-helper-konsole` — generates `PlasmaMaster.profile` and sets it as the default Konsole profile
- **TemplateEngine** — header-only `{{VARIABLE}}` substitution engine. Provides hex and RGB variants of all ANSI colors, semantic palette colors, and derived shades from a single `UniversalPalette`.
- **TemplateConfig** — reads and writes `config.toml` using the bundled `toml++` v3.4.0 (header-only, no new runtime dependency).
- **Separate `[discord.*]` Sections** — BetterDiscord and Vencord import configuration (Midnight toggle, custom import URLs) live in their own `[discord.betterdiscord]` / `[discord.vencord]` sections, separate from the template entries. GUI dialogs write directly to these sections.
- **Custom Template Support** — add any `[templates.myapp]` entry to `config.toml` and it will be rendered and written automatically, with an optional `post_hook` for reload commands.

### Changed

- **Universal Theme Sync completely rewritten**. The monolithic `UniversalThemeExporter.cpp` (1700+ lines of hardcoded string literals) has been replaced by `syncTemplates()` which iterates `config.toml` entries.
- **GUI toggles write directly to `config.toml`** via `TemplateConfig::setEnabled()` instead of `KConfig`. The legacy `plasma-theme-masterrc` sync toggle keys are removed.
- **CLI `sync-enable` / `sync-disable` / `sync-list` / `sync-restore`** updated to operate on `config.toml` template entries. The app list now includes `konsole`, `btop`, `vicinae`, and `vencord` in addition to the previous set.
- **Built-in post-hooks are hardcoded in C++** for all known apps. Users do not need to know or configure hook commands — the app always knows what each built-in app needs.
- **install.sh**: Copies `config.toml.default` → `~/.config/plasma-theme-master/config.toml` on first install. Skips copy if an existing config is present (preserving user settings on upgrades).
- **uninstall.sh**: Now also removes helper binaries, the system data directory (`/usr/share/plasma-theme-master/`), and optionally the user `config.toml`.

### Removed

- All individual `exportToX()` / `restoreX()` static methods from `UniversalThemeExporter` (VSCode, Firefox, BetterDiscord, Kitty, Konsole, Btop, Vicinae, Zed, Obsidian, Vencord) — the GUI restore buttons remain and now call `restoreFile()` using the output path from `config.toml`.
- All universal-theme sync methods from `Config` (`isVSCodeSyncEnabled()`, `setKittySyncEnabled()`, etc.) — replaced by `TemplateConfig`.

## [1.1.8] - 2026-03-18

### Added
- **Native KWin Night Light Integration**:
  - The application now natively reads KDE Plasma's Night Color settings directly via D-Bus instead of relying exclusively on calculating sunrise/sunset times using location coordinates.
  - Location calculation acts as a smooth fallback system.
- **Improved Day/Night Cycle Configuration**:
  - Respect Native Plasma Day-Night Cycle setting (Currently works for Custom Long & Lat as well as setting Custom Times)
  - Added new intuitive sliders for configuring global Day and Night Color Temperatures. 
  - Added a button to natively launch KDE's System Settings for Day-Night Cycle directly from the application.

### Fixed
- **Universal Sync Path Resolution**: Fixed an issue where exporting themes to standalone IDEs like Antigravity failed because their local sync path was removed during recent configuration abstraction.
- **KDE AutomaticLookAndFeel Desynchronization**: Resolved a significant defect where pressing "Apply Target" or letting the daemon re-apply themes would permanently sever KDE's native `AutomaticLookAndFeel` property, preventing further time-based transitions.
- **Double Pass Stability**: Refactored the delayed second-pass execution pipeline into a stable single-pass loop, improving system response stability and preventing unintended race conditions.

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
