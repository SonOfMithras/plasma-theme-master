#ifndef FLATPAKMANAGER_H
#define FLATPAKMANAGER_H

#include <QString>
#include <QStringList>

class FlatpakManager {
public:
  // Check if flatpak binary exists
  static bool isFlatpakInstalled();

  // Check if the necessary overrides are active
  static bool hasFilesystemAccess();

  // Run the override commands
  static bool setupFlatpakEnvironment();

  // Get a status string (for GUI/CLI)
  static QString flatpakStatus();

  // Get list of exposed directories (for CLI status)
  static QStringList getExposedDirectories();

  // Set GTK_THEME environment variable for Flatpaks
  static bool setFlatpakGtkTheme(const QString &themeName);

  // Get current GTK_THEME environment variable from Flatpak overrides
  static QString getFlatpakGtkTheme();
};

#endif // FLATPAKMANAGER_H
