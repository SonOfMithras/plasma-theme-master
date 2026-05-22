#ifndef THEME_READER_H
#define THEME_READER_H

#include <QString>
#include <QStringList> // Add include for QStringList

class ThemeReader {
public:
  static QString currentGlobalTheme();
  static QString currentColorScheme();
  static QString currentKvantumTheme();
  static QString currentGtkTheme();
  static bool isAutoLookAndFeel();
  
  // KWin Night Color DBus & Config
  static bool isKWinDaytime();
  static int kwinDayTemperature();
  static int kwinNightTemperature();

  // Native Day-Night Location
  static double nativeLatitude();
  static double nativeLongitude();

  // Listings
  static QStringList listKvantumThemes();
  static QStringList listGlobalThemes();
  static QStringList listGtkThemes();

  // Configured Defaults
  static QString defaultDarkTheme();
  static QString defaultLightTheme();

  // App Config (plasma-theme-masterrc)
  static int solarDayOffset();
  static int solarNightOffset();
  static bool reenableAutoOnScheduledChange();
  static QString temporaryOverride();
  static QString overrideScheduledState();
  static QString dayKvantumTheme();
  static QString nightKvantumTheme();
  static QString dayGtkTheme();
  static QString nightGtkTheme();

  // Flatpak Config
  static QString dayFlatpakTheme();
  static QString nightFlatpakTheme();
  static bool flatpakFollowsGtk();

  // Klassy
  static QStringList listKlassyPresets();
  static QString dayKlassyPreset();
  static QString nightKlassyPreset();
  static QString lastAppliedKlassyPreset();
};

#endif // THEME_READER_H
