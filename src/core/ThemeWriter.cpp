#include "ThemeWriter.h"
#include "Logger.h"
#include <KConfig>
#include <KConfigGroup>
#include <QSettings>
#include <QDebug>
#include <QProcess>
#include <QStandardPaths>
#include <QFileInfo>

void ThemeWriter::setAutoLookAndFeel(bool enabled) {
    KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("KDE"));
    group.writeEntry(QStringLiteral("AutomaticLookAndFeel"), enabled);
    config.sync();
    
    qDebug() << "Set AutomaticLookAndFeel to" << enabled;
}

bool ThemeWriter::setKvantumTheme(const QString &themeName) {
    if (themeName.isEmpty()) return false;
    
    // Method 1: kvantummanager
    QString kvantumExe = QStandardPaths::findExecutable(QStringLiteral("kvantummanager"));
    if (!kvantumExe.isEmpty()) {
        QProcess process;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
        process.setProcessEnvironment(env);
        
        process.start(kvantumExe, QStringList() << QStringLiteral("--set") << themeName);
        process.waitForFinished();
        if (process.exitCode() == 0) {
            Logger::log("Applied Kvantum theme via kvantummanager: \"" + themeName + "\"", Logger::Info);
            return true;
        } else {
           Logger::log("kvantummanager failed, falling back to config file write.", Logger::Warning);
        }
    }
    
    // Method 2: Manual Config
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/Kvantum/kvantum.kvconfig");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    group.writeEntry(QStringLiteral("theme"), themeName);
    config.sync();
    Logger::log("Wrote Kvantum theme to config: \"" + themeName + "\"", Logger::Info);
    return true;
}

void ThemeWriter::setDefaultDarkTheme(const QString &themeName) {
    KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("KDE"));
    group.writeEntry(QStringLiteral("DefaultDarkLookAndFeel"), themeName);
    config.sync();
    Logger::log("Set DefaultDarkLookAndFeel to \"" + themeName + "\"", Logger::Info);
}

void ThemeWriter::setDefaultLightTheme(const QString &themeName) {
    KConfig config(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("KDE"));
    group.writeEntry(QStringLiteral("DefaultLightLookAndFeel"), themeName);
    config.sync();
    Logger::log("Set DefaultLightLookAndFeel to \"" + themeName + "\"", Logger::Info);
}

bool ThemeWriter::applyGlobalTheme(const QString &themeName) {
    if (themeName.isEmpty()) return false;
    
    QString tool = QStandardPaths::findExecutable(QStringLiteral("lookandfeeltool"));
    if (tool.isEmpty()) {
        Logger::log("lookandfeeltool not found!", Logger::Warning);
        return false;
    }
    
    QProcess process;
    process.start(tool, QStringList() << QStringLiteral("-a") << themeName);
    process.waitForFinished();
    
    if (process.exitCode() == 0) {
        Logger::log("Applied global theme: \"" + themeName + "\"", Logger::Info);
        return true;
    } else {
        Logger::log("Failed to apply global theme: \"" + themeName + "\"", Logger::Error);
        return false;
    }
}

void ThemeWriter::setSolarPadding(int minutes) {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    group.writeEntry(QStringLiteral("SolarPadding"), minutes);
    config.sync();
    Logger::log("Set SolarPadding to " + QString::number(minutes), Logger::Info);
}

void ThemeWriter::setDayKvantumTheme(const QString &themeName) {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    group.writeEntry(QStringLiteral("DayKvantumTheme"), themeName);
    config.sync();
    Logger::log("Set DayKvantumTheme to \"" + themeName + "\"", Logger::Info);
}

void ThemeWriter::setNightKvantumTheme(const QString &themeName) {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    group.writeEntry(QStringLiteral("NightKvantumTheme"), themeName);
    config.sync();
    Logger::log("Set NightKvantumTheme to \"" + themeName + "\"", Logger::Info);
}

bool ThemeWriter::setGtkTheme(const QString &themeName) {
    if (themeName.isEmpty()) return false;
    
    // Path 1: GTK 3
    QString gtk3Path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/gtk-3.0/settings.ini");
    QSettings gtk3Settings(gtk3Path, QSettings::IniFormat);
    gtk3Settings.setValue(QStringLiteral("Settings/gtk-theme-name"), themeName);
    
    // Path 2: GTK 4
    QString gtk4Path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/gtk-4.0/settings.ini");
    QSettings gtk4Settings(gtk4Path, QSettings::IniFormat);
    gtk4Settings.setValue(QStringLiteral("Settings/gtk-theme-name"), themeName);
    
    Logger::log("Applied GTK theme: \"" + themeName + "\"", Logger::Info);
    return true;
}

void ThemeWriter::setDayGtkTheme(const QString &themeName) {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    group.writeEntry(QStringLiteral("DayGtkTheme"), themeName);
    config.sync();
    Logger::log("Set DayGtkTheme to \"" + themeName + "\"", Logger::Info);
}

void ThemeWriter::setNightGtkTheme(const QString &themeName) {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    group.writeEntry(QStringLiteral("NightGtkTheme"), themeName);
    config.sync();
    qDebug() << "Set NightGtkTheme to" << themeName;
}
