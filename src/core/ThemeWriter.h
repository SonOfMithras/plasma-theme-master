#ifndef THEME_WRITER_H
#define THEME_WRITER_H

#include <QString>

class ThemeWriter {
public:
  static void setAutoLookAndFeel(bool enabled);
  static bool setKvantumTheme(const QString &themeName, bool force = false);
  static bool setGtkTheme(const QString &themeName, bool force = false);

  // Global Theme Setters
  static void setDefaultDarkTheme(const QString &themeName);
  static void setDefaultLightTheme(const QString &themeName);
  static bool applyGlobalTheme(const QString &themeName, bool force = false);
  static bool applyColorScheme(const QString &schemeName, bool force = false);

  // App Config (plasma-theme-masterrc)
  static void setSolarPadding(int minutes);
  static void setDayKvantumTheme(const QString &themeName);
  static void setNightKvantumTheme(const QString &themeName);
  static void setDayGtkTheme(const QString &themeName);
  static void setNightGtkTheme(const QString &themeName);

  // Flatpak Config
  static void setDayFlatpakTheme(const QString &themeName);
  static void setNightFlatpakTheme(const QString &themeName);
  static void setFlatpakFollowsGtk(bool follow);

  // Klassy
  static void setKlassyPreset(const QString &presetName, bool force = false);
  static void setDayKlassyPreset(const QString &presetName);
  static void setNightKlassyPreset(const QString &presetName);
  
  static void syncMaterialYouIcons(bool force = false);
};

#endif // THEME_WRITER_H
