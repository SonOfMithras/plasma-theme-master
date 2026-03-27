#include "ThemeWriter.h"
#include "Config.h"
#include "FlatpakManager.h"
#include "GlobalThemeManager.h"
#include "Logger.h"
#include "ThemeReader.h"
#include <KConfig>
#include <KConfigGroup>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>

void ThemeWriter::setAutoLookAndFeel(bool enabled) {
  KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("KDE"));
  group.writeEntry("AutomaticLookAndFeel", enabled);
  config.sync();
  Logger::log("System AutomaticLookAndFeel set to " +
                  QString(enabled ? "true" : "false"),
              Logger::Info);
}

void ThemeWriter::enforceKWinNightColorActive() {
  KConfig config(QStringLiteral("kwinrc"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("NightColor"));
  group.writeEntry("Active", true);
  config.sync();

  // Tell KWin to reconfigure
  QDBusMessage message = QDBusMessage::createMethodCall(
      QStringLiteral("org.kde.KWin"), QStringLiteral("/KWin"),
      QStringLiteral("org.kde.KWin"), QStringLiteral("reconfigure"));
  QDBusConnection::sessionBus().send(message);
  Logger::log("Enforced KWin NightColor Active=true and reconfigured KWin.",
              Logger::Info);
}

void ThemeWriter::setKWinTemperatures(int dayTemp, int nightTemp) {
  if (dayTemp < 1000)
    dayTemp = 1000;
  if (dayTemp > 6500)
    dayTemp = 6500;
  if (nightTemp < 1000)
    nightTemp = 1000;
  if (nightTemp > 6500)
    nightTemp = 6500;

  KConfig config(QStringLiteral("kwinrc"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("NightColor"));
  group.writeEntry("DayTemperature", dayTemp);
  group.writeEntry("NightTemperature", nightTemp);
  config.sync();

  // Tell KWin to reconfigure
  QDBusMessage message = QDBusMessage::createMethodCall(
      QStringLiteral("org.kde.KWin"), QStringLiteral("/KWin"),
      QStringLiteral("org.kde.KWin"), QStringLiteral("reconfigure"));
  QDBusConnection::sessionBus().send(message);
  Logger::log(
      QString("Updated KWin NightColor temperatures (Day: %1, Night: %2)")
          .arg(dayTemp)
          .arg(nightTemp),
      Logger::Info);
}

bool ThemeWriter::setKvantumTheme(const QString &themeName, bool force) {
  if (themeName.isEmpty())
    return false;

  if (!force && ThemeReader::currentKvantumTheme() == themeName) {
    Logger::log("Kvantum theme is already \"" + themeName + "\", skipping.",
                Logger::Info);
    return true;
  }

  // Method 1: kvantummanager
  QString kvantumExe =
      QStandardPaths::findExecutable(QStringLiteral("kvantummanager"));
  if (!kvantumExe.isEmpty()) {
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
    process.setProcessEnvironment(env);

    process.start(kvantumExe, QStringList()
                                  << QStringLiteral("--set") << themeName);
    process.waitForFinished();
    if (process.exitCode() == 0) {
      Logger::log("Applied Kvantum theme via kvantummanager: \"" + themeName +
                      "\"",
                  Logger::Info);
      return true;
    } else {
      Logger::log("kvantummanager failed, falling back to config file write.",
                  Logger::Warning);
    }
  }

  // Method 2: Manual Config
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/Kvantum/kvantum.kvconfig");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  group.writeEntry(QStringLiteral("theme"), themeName);
  config.sync();
  Logger::log("Wrote Kvantum theme to config: \"" + themeName + "\"",
              Logger::Info);
  return true;
}

void ThemeWriter::setDefaultDarkTheme(const QString &themeName) {
  KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("KDE"));
  group.writeEntry(QStringLiteral("DefaultDarkLookAndFeel"), themeName);
  config.sync();
  Logger::log("Set DefaultDarkLookAndFeel to \"" + themeName + "\"",
              Logger::Info);
}

void ThemeWriter::setDefaultLightTheme(const QString &themeName) {
  KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("KDE"));
  group.writeEntry(QStringLiteral("DefaultLightLookAndFeel"), themeName);
  config.sync();
  Logger::log("Set DefaultLightLookAndFeel to \"" + themeName + "\"",
              Logger::Info);
}

bool ThemeWriter::applyGlobalTheme(const QString &themeName, bool force) {
  if (themeName.isEmpty())
    return false;

  if (!force && ThemeReader::currentGlobalTheme() == themeName) {
    Logger::log("Global theme is already \"" + themeName + "\", skipping.",
                Logger::Info);
    return true;
  }

  QString tool =
      QStandardPaths::findExecutable(QStringLiteral("lookandfeeltool"));
  if (tool.isEmpty()) {
    Logger::log("lookandfeeltool not found!", Logger::Warning);
    return false;
  }

  // Force reset widget style to Breeze to ensure Kvantum themes reload
  // correctly when the target theme (which likely uses Kvantum) is applied.
  KConfig kdeglobals(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
  KConfigGroup kdeGroup = kdeglobals.group(QStringLiteral("KDE"));
  kdeGroup.writeEntry(QStringLiteral("widgetStyle"), QStringLiteral("Breeze"));
  kdeglobals.sync();

  QProcess process;
  process.start(tool, QStringList() << QStringLiteral("-a") << themeName);
  process.waitForFinished();

  QString stdoutStr =
      QString::fromUtf8(process.readAllStandardOutput()).trimmed();
  QString stderrStr =
      QString::fromUtf8(process.readAllStandardError()).trimmed();

  if (!stdoutStr.isEmpty()) {
    Logger::log("lookandfeeltool output: " + stdoutStr, Logger::Info);
  }
  if (!stderrStr.isEmpty()) {
    Logger::log("lookandfeeltool error: " + stderrStr, Logger::Warning);
  }

  if (process.exitCode() == 0) {
    Logger::log("Applied global theme: \"" + themeName + "\"", Logger::Info);
    return true;
  } else {
    Logger::log("Failed to apply global theme: \"" + themeName + "\"",
                Logger::Error);
    return false;
  }
}

bool ThemeWriter::applyColorScheme(const QString &schemeName, bool force) {
  if (schemeName.isEmpty())
    return false;

  if (!force && ThemeReader::currentColorScheme() == schemeName) {
    Logger::log("Color scheme is already \"" + schemeName + "\", skipping.",
                Logger::Info);
    return true;
  }

  QString tool = QStandardPaths::findExecutable(
      QStringLiteral("plasma-apply-colorscheme"));
  if (tool.isEmpty()) {
    Logger::log("plasma-apply-colorscheme not found!", Logger::Warning);
    return false;
  }

  QProcess process;
  process.start(tool, QStringList() << schemeName);
  process.waitForFinished();

  QString stdoutStr =
      QString::fromUtf8(process.readAllStandardOutput()).trimmed();
  QString stderrStr =
      QString::fromUtf8(process.readAllStandardError()).trimmed();

  if (!stdoutStr.isEmpty()) {
    Logger::log("plasma-apply-colorscheme output: " + stdoutStr, Logger::Info);
  }
  if (!stderrStr.isEmpty()) {
    Logger::log("plasma-apply-colorscheme error: " + stderrStr,
                Logger::Warning);
  }

  if (process.exitCode() == 0) {
    Logger::log("Applied color scheme: \"" + schemeName + "\"", Logger::Info);
    return true;
  } else {
    Logger::log("Failed to apply color scheme: \"" + schemeName + "\"",
                Logger::Error);
    return false;
  }
}

void ThemeWriter::setSolarPadding(int minutes) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  group.writeEntry(QStringLiteral("SolarPadding"), minutes);
  config.sync();
  Logger::log("Set SolarPadding to " + QString::number(minutes), Logger::Info);
}

void ThemeWriter::setDayKvantumTheme(const QString &themeName) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  group.writeEntry(QStringLiteral("DayKvantumTheme"), themeName);
  config.sync();
  Logger::log("Set DayKvantumTheme to \"" + themeName + "\"", Logger::Info);
}

void ThemeWriter::setNightKvantumTheme(const QString &themeName) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  group.writeEntry(QStringLiteral("NightKvantumTheme"), themeName);
  config.sync();
  Logger::log("Set NightKvantumTheme to \"" + themeName + "\"", Logger::Info);
}

bool ThemeWriter::setGtkTheme(const QString &themeName, bool force) {
  if (themeName.isEmpty())
    return false;

  if (!force && ThemeReader::currentGtkTheme() == themeName) {
    Logger::log("GTK theme is already \"" + themeName + "\", skipping.",
                Logger::Info);
    return true;
  }

  // Path 1: GTK 3
  QString gtk3Path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/gtk-3.0/settings.ini");
  QSettings gtk3Settings(gtk3Path, QSettings::IniFormat);
  gtk3Settings.setValue(QStringLiteral("Settings/gtk-theme-name"), themeName);

  // Path 2: GTK 4
  QString gtk4Path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/gtk-4.0/settings.ini");
  QSettings gtk4Settings(gtk4Path, QSettings::IniFormat);
  gtk4Settings.setValue(QStringLiteral("Settings/gtk-theme-name"), themeName);

  // Path 3: GSettings (for running apps)
  QString gsettingsExe =
      QStandardPaths::findExecutable(QStringLiteral("gsettings"));
  if (!gsettingsExe.isEmpty()) {
    QProcess process;
    process.start(gsettingsExe,
                  QStringList() << QStringLiteral("set")
                                << QStringLiteral("org.gnome.desktop.interface")
                                << QStringLiteral("gtk-theme") << themeName);
    process.waitForFinished();
    if (process.exitCode() == 0) {
      Logger::log("Applied GTK theme via gsettings: \"" + themeName + "\"",
                  Logger::Info);
    } else {
      Logger::log("Failed to set gsettings GTK theme.", Logger::Warning);
    }
  }

  Logger::log("Applied GTK theme: \"" + themeName + "\"", Logger::Info);

  // Auto-Sync Flatpak if enabled
  QString confPath =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(confPath, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  if (group.readEntry("FlatpakFollowsGtk", false)) {
    FlatpakManager::setFlatpakGtkTheme(themeName);
  }

  return true;
}

void ThemeWriter::setDayFlatpakTheme(const QString &themeName) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  group.writeEntry(QStringLiteral("DayFlatpakTheme"), themeName);
  config.sync();
  Logger::log("Set DayFlatpakTheme to \"" + themeName + "\"", Logger::Info);
}

void ThemeWriter::setNightFlatpakTheme(const QString &themeName) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  group.writeEntry(QStringLiteral("NightFlatpakTheme"), themeName);
  config.sync();
  Logger::log("Set NightFlatpakTheme to \"" + themeName + "\"", Logger::Info);
}

void ThemeWriter::setFlatpakFollowsGtk(bool follow) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  group.writeEntry(QStringLiteral("FlatpakFollowsGtk"), follow);
  config.sync();
  Logger::log(QString("Set FlatpakFollowsGtk to: %1").arg(follow),
              Logger::Info);
}

void ThemeWriter::setDayGtkTheme(const QString &themeName) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  group.writeEntry(QStringLiteral("DayGtkTheme"), themeName);
  config.sync();
  Logger::log("Set DayGtkTheme to \"" + themeName + "\"", Logger::Info);
}

void ThemeWriter::setNightGtkTheme(const QString &themeName) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      QStringLiteral("/plasma-theme-masterrc");
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group(QStringLiteral("General"));
  group.writeEntry(QStringLiteral("NightGtkTheme"), themeName);
  config.sync();
  qDebug() << "Set NightGtkTheme to" << themeName;
}

// Klassy Day & Night
void ThemeWriter::setKlassyPreset(const QString &presetName, bool force) {
  if (presetName.isEmpty())
    return;

  if (!force && ThemeReader::lastAppliedKlassyPreset() == presetName) {
    Logger::log("Klassy preset is already \"" + presetName + "\", skipping.",
                Logger::Info);
    return;
  }

  QString tool = QStandardPaths::findExecutable("klassy-settings");
  if (tool.isEmpty()) {
    Logger::log("klassy-settings executable not found.", Logger::Warning);
    return;
  }

  QProcess process;
  process.start(tool, QStringList() << "-w" << presetName);
  process.waitForFinished();

  if (process.exitCode() == 0) {
    Logger::log("Applied Klassy preset: \"" + presetName + "\"", Logger::Info);

    // Save state
    QString path =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
        "/plasma-theme-masterrc";
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group("State");
    group.writeEntry("LastAppliedKlassyPreset", presetName);
    config.sync();

  } else {
    QString err = QString::fromUtf8(process.readAllStandardError()).trimmed();
    Logger::log("Failed to set Klassy preset: " + err, Logger::Error);
  }
}

void ThemeWriter::setDayKlassyPreset(const QString &presetName) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      "/plasma-theme-masterrc";
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group("General");
  group.writeEntry("DayKlassyPreset", presetName);
  config.sync();
  Logger::log("Set DayKlassyPreset to \"" + presetName + "\"", Logger::Info);
}

void ThemeWriter::setNightKlassyPreset(const QString &presetName) {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      "/plasma-theme-masterrc";
  KConfig config(path, KConfig::SimpleConfig);
  KConfigGroup group = config.group("General");
  group.writeEntry("NightKlassyPreset", presetName);
  config.sync();
  Logger::log("Set NightKlassyPreset to \"" + presetName + "\"", Logger::Info);
}

void ThemeWriter::syncMaterialYouIcons(bool force) {
  if (!force && !Config::isMaterialYouOverrideEnabled())
    return;

  QString dayTheme = ThemeReader::defaultLightTheme();
  QString nightTheme = ThemeReader::defaultDarkTheme();

  QString dayIcon = GlobalThemeManager::getIconThemeFromGlobal(dayTheme);
  QString nightIcon = GlobalThemeManager::getIconThemeFromGlobal(nightTheme);

  QStringList args;
  if (!dayIcon.isEmpty()) {
    args << "--iconslight" << dayIcon;
  }
  if (!nightIcon.isEmpty()) {
    args << "--iconsdark" << nightIcon;
  }

  args << "--chroma-multiplier" << QString::number(Config::materialYouChroma());
  args << "--tone-multiplier" << QString::number(Config::materialYouTone());
  args << "--scheme-variant"
       << QString::number(Config::materialYouSchemeVariant());
  args << "--ncolor" << QString::number(Config::materialYouNColor());

  QString exe = QStandardPaths::findExecutable("kde-material-you-colors");
  if (exe.isEmpty()) {
    exe = QDir::homePath() + "/.local/bin/kde-material-you-colors";
  }

  // 1. Update autostart silently
  QStringList autostartArgs = args;
  autostartArgs.prepend("-a");
  QProcess::startDetached(exe, autostartArgs);

  // 2. Start the actual background daemon
  QProcess::startDetached(exe, args);

  Logger::log("Synced Material You Icons and started process: day=" + dayIcon +
                  ", night=" + nightIcon,
              Logger::Info);
}