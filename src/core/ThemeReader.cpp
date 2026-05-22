#include "ThemeReader.h"
#include "Solar.h"
#include <KConfig>
#include <KConfigGroup>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QStandardPaths>

QStringList ThemeReader::listKvantumThemes() {
  QStringList themes;
  QStringList paths = {
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
          QStringLiteral("/Kvantum"),
      QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
          QStringLiteral("/.themes"),
      QStringLiteral("/usr/share/Kvantum"),
      QStringLiteral("/usr/share/themes")};

  for (const QString &path : paths) {
    if (!QDir(path).exists())
      continue;

    QDirIterator it(path, QStringList() << QStringLiteral("*.kvconfig"),
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
      it.next();
      QString name = it.fileInfo().baseName();
      if (name != QStringLiteral("kvantum") && !themes.contains(name)) {
        themes.append(name);
      }
    }
  }
  themes.sort();
  return themes;
}

QString ThemeReader::currentGlobalTheme() {
  KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("KDE"));
  return group.readEntry(QStringLiteral("LookAndFeelPackage"), QString());
}

QString ThemeReader::currentColorScheme() {
  KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry(QStringLiteral("ColorScheme"), QString());
}

QString ThemeReader::currentKvantumTheme() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/Kvantum/kvantum.kvconfig");
  if (!QFile::exists(path))
    return QString();

  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry(QStringLiteral("theme"), QString());
}

QString ThemeReader::currentGtkTheme() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/gtk-3.0/settings.ini");
  if (!QFile::exists(path))
    return QString();

  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("Settings"));
  return group.readEntry(QStringLiteral("gtk-theme-name"), QString());
}

bool ThemeReader::isAutoLookAndFeel() {
  KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("KDE"));
  return group.readEntry(QStringLiteral("AutomaticLookAndFeel"), false);
}

double ThemeReader::nativeLatitude() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/knighttimerc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("Location"));
  return group.readEntry(QStringLiteral("Latitude"), 0.0);
}

bool ThemeReader::isKWinDaytime() {
  QDBusInterface nightLight(QStringLiteral("org.kde.KWin"),
                            QStringLiteral("/org/kde/KWin/NightLight"),
                            QStringLiteral("org.kde.KWin.NightLight"),
                            QDBusConnection::sessionBus());
  if (nightLight.isValid()) {
    QVariant daylightVar = nightLight.property("daylight");
    if (daylightVar.isValid()) {
      return daylightVar.toBool();
    }
  }
  // Fallback if DBus fails
  double lat = nativeLatitude();
  double lon = nativeLongitude();
  int dayOffset = solarDayOffset();
  int nightOffset = solarNightOffset();
  return Solar::isDaytime(lat, lon, dayOffset, nightOffset);
}

int ThemeReader::kwinDayTemperature() {
  KConfig config(QStringLiteral("kwinrc"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("NightColor"));
  return group.readEntry(QStringLiteral("DayTemperature"), 6500);
}

int ThemeReader::kwinNightTemperature() {
  KConfig config(QStringLiteral("kwinrc"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("NightColor"));
  return group.readEntry(QStringLiteral("NightTemperature"), 4500);
}

double ThemeReader::nativeLongitude() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/knighttimerc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("Location"));
  return group.readEntry(QStringLiteral("Longitude"), 0.0);
}

QStringList ThemeReader::listGlobalThemes() {
  QStringList themes;
  QStringList searchPaths = {
      QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
          QStringLiteral("/plasma/look-and-feel"),
      QStringLiteral("/usr/share/plasma/look-and-feel")};

  for (const QString &path : searchPaths) {
    if (!QDir(path).exists())
      continue;

    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
      it.next();
      QString name = it.fileName();
      if (!themes.contains(name)) {
        themes.append(name);
      }
    }
  }
  themes.sort();
  return themes;
}

QString ThemeReader::defaultDarkTheme() {
  KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("KDE"));
  return group.readEntry(QStringLiteral("DefaultDarkLookAndFeel"), QString());
}

QString ThemeReader::defaultLightTheme() {
  KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("KDE"));
  return group.readEntry(QStringLiteral("DefaultLightLookAndFeel"), QString());
}

int ThemeReader::solarDayOffset() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry(QStringLiteral("SolarDayOffset"), 0);
}

int ThemeReader::solarNightOffset() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry(QStringLiteral("SolarNightOffset"), 0);
}

bool ThemeReader::reenableAutoOnScheduledChange() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry(QStringLiteral("ReenableAutoOnScheduledChange"), false);
}

QString ThemeReader::temporaryOverride() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("State"));
  return group.readEntry(QStringLiteral("TemporaryOverride"), QString());
}

QString ThemeReader::overrideScheduledState() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("State"));
  return group.readEntry(QStringLiteral("OverrideScheduledState"), QString());
}

QString ThemeReader::dayKvantumTheme() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry(QStringLiteral("DayKvantumTheme"), QString());
}

QString ThemeReader::nightKvantumTheme() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry(QStringLiteral("NightKvantumTheme"), QString());
}

QStringList ThemeReader::listGtkThemes() {
  QStringList themes;
  QStringList paths = {
      QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
          QStringLiteral("/.themes"),
      QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
          QStringLiteral("/themes"),
      QStringLiteral("/usr/share/themes")};

  for (const QString &path : paths) {
    if (!QDir(path).exists())
      continue;

    QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
      it.next();
      QString name = it.fileName();
      // Validation: Check if it looks like a theme
      bool isValid = false;

      if (QFile::exists(it.filePath() + "/index.theme"))
        isValid = true;
      else if (QFile::exists(it.filePath() + "/gtk-3.0/gtk.css"))
        isValid = true;
      else if (QFile::exists(it.filePath() + "/gtk-4.0/gtk.css"))
        isValid = true;
      else if (QFile::exists(it.filePath() + "/gtk-2.0/gtkrc"))
        isValid = true;

      if (isValid) {
        if (!themes.contains(name)) {
          themes.append(name);
        }
      }
    }
  }
  themes.sort();
  return themes;
}

QString ThemeReader::dayGtkTheme() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry(QStringLiteral("DayGtkTheme"), QString());
}

QString ThemeReader::nightGtkTheme() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry(QStringLiteral("NightGtkTheme"), QString());
}

QString ThemeReader::dayFlatpakTheme() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry("DayFlatpakTheme", QString());
}

QString ThemeReader::nightFlatpakTheme() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry("NightFlatpakTheme", QString());
}

bool ThemeReader::flatpakFollowsGtk() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry("FlatpakFollowsGtk", false);
}

QStringList ThemeReader::listKlassyPresets() {
  QStringList presets;
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      "/klassy/windecopresetsrc";
  QFile file(path);

  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      // Look for [Windeco Preset <Name>]
      if (line.startsWith("[Windeco Preset ") && line.endsWith("]")) {
        int start = 16; // Length of "[Windeco Preset "
        int end = line.length() - 1;
        QString name = line.mid(start, end - start);
        if (!name.isEmpty()) {
          presets.append(name);
        }
      }
    }
    file.close();
  }
  presets.sort();
  return presets;
}

QString ThemeReader::dayKlassyPreset() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry("DayKlassyPreset", QString());
}

QString ThemeReader::nightKlassyPreset() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  return group.readEntry("NightKlassyPreset", QString());
}

QString ThemeReader::lastAppliedKlassyPreset() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("State"));
  return group.readEntry("LastAppliedKlassyPreset", QString());
}