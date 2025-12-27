# Project Implementation Log

This document tracks the step-by-step implementation of the Plasma Theme Master application.

## 1. CLI Status Reporting [DONE]
   a. Create CMake project structure.
   b. Implement `install.sh` and `uninstall.sh`.
   c. Implement `ThemeReader` core class to read `kdeglobals`, `kvantum.kvconfig`, and `gtk-3.0/settings.ini`.
   d. Implement `main.cpp` with `status` command.
   e. Sync Latitude/Longitude with native KDE configuration (`knighttimerc`).

## 2. Simple GUI Viewer [DONE]
   a. Update CMake to include `Qt6::Widgets`.
   b. Create `MainWindow` class with `QFormLayout`.
   c. Display read-only status information from `ThemeReader`.
   d. Update `main.cpp` to launch GUI by default or via flags.

## 3. CLI Auto LookAndFeel Switch [DONE]
   a. Create `ThemeWriter` core class for config modification.
   b. Implement `setAutoLookAndFeel(bool)` to update `kdeglobals`.
   c. Add `set-auto <true|false>` command to `main.cpp`.
   d. Update `--help` documentation.

## 4. CLI Kvantum Theme Setter [DONE]
   a. Add `setKvantumTheme(QString)` to `ThemeWriter`.
   b. Implement logic to run `kvantummanager --set` (with fallback).
   c. Add `set-kvantum <theme>` command to CLI.
   d. Verify theme exists before setting.

## 5. CLI Global Theme Setter [DONE]
   a. Add `setGlobalTheme(QString)` to `ThemeWriter`.
   b. Use `lookandfeeltool -a` to apply global theme.
   c. Add `set-global <theme>` command to CLI.

## 6. CLI Global Day/Night Theme Setter [DONE]
   a. Implement logic to store Day/Night specific Global themes (app config).
   b. Add `set-day-global` and `set-night-global` commands.
   c. Implement `apply-cycle` command to apply correct theme based on time/sun.

## 7. Advanced CLI Logic & Kvantum Support [DONE]
   a. Implement App Config (plasma-theme-masterrc).
   b. Add `set-offset` and `status` updates (Solar Padding).
   c. Add `set-kvantum-day` & `set-kvantum-night`.
   d. Update `set-auto` and `set-static-*` to apply Kvantum themes.

## 8. GUI Integration [DONE]
   a. Update MainWindow to show Solar Status.
   b. Add Controls for Offset and Kvantum Preferences.
   c. Add Controls for Global Themes.
   d. Integrate `ThemeWriter` setters into GUI.

## 9. CLI GTK Theme Setter [DONE]
   a. Add `set-gtk-theme(QString)` to `ThemeWriter`.
   b. Implement logic to update `gtk-3.0/settings.ini`. and `gtk-4.0/settings.ini`.
   c. Add `set-gtk <theme>` command to CLI.
   d. use the same logic as `set-kvantum` to verify theme exists before setting as well as to save dark and light configs.
   e. Update `--help` documentation.

## 10. GUI GTK Theme Setter [DONE]
   a. Integrate GTK Theme selection into GUI.

## 11. Log integration [DONE]
   a. set up a logging system to log all actions and errors with a corresponding timestamp.
   b. ensure the log file is stored in a consistent location that users would expect.
   c. ensure the cli log command works with a clear log command as well as a modifier to only show the last n lines or filter by errors or a specific type of action.

## 12. GUI Refinement [DONE]
   a. add a seperate gui tab for the log with most recent logs at the top
   b. verify that the gui log tab is updated in real time and that it is scrollable with a clear log button
   c. refine the gui to be more user friendly and intuitive (Add padding to the gui as well as subheadings for each section within a tab)

## 13. Integrate app into system [DONE]
   a. add a desktop entry for the app
   b. add a menu entry for the app
   c. add a system service to run in the background doing the theme switching for kvantum and gtk while ensuring that the autolookandfeel status returns to the set value if the it was messed up.
   d. verify robustness of the uninstall script
   e. integrate the uninstall script into the CLI

## 14. New tab - global theme editor [DONE]
   a. add a new tab for global theme editor
   b. CLI commands to clone the global theme to the user dir and dropdowns to edit compondents of that theme
   c. Similar to the global theme editor in the python equivalent

## 15. GUI Refinement 2.0 [DONE] 
   a. Add a menu bar with (files - help - about)
   b. Add info to the about popup - version, author, license, and a link to the github repo
   c. The help section should have a clear log button as well as a clear config button as well as an uninstall button
   d. the files section should have options to view local theme files (global themes folder, kvantum themes folder, gtk themes folder - in the userspace) as well as an option to view the local app config files

## 16. Daemon fix and codebase cleanup [DONE]
   a. fix the daemon
   b. improve installer logic 
   c. cleanup the codebase

