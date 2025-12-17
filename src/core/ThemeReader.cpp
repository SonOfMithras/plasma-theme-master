#include "ThemeReader.h"
#include <KConfig>
#include <KConfigGroup>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QDirIterator>

QStringList ThemeReader::listKvantumThemes() {
    QStringList themes;
    QStringList paths = {
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/Kvantum"),
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QStringLiteral("/.themes"),
        QStringLiteral("/usr/share/Kvantum"),
        QStringLiteral("/usr/share/themes")
    };

    for (const QString &path : paths) {
        if (!QDir(path).exists()) continue;
        
        QDirIterator it(path, QStringList() << QStringLiteral("*.kvconfig"), QDir::Files, QDirIterator::Subdirectories);
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

QString ThemeReader::currentKvantumTheme() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/Kvantum/kvantum.kvconfig");
    if (!QFile::exists(path)) return QString();
    
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    return group.readEntry(QStringLiteral("theme"), QString());
}

QString ThemeReader::currentGtkTheme() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/gtk-3.0/settings.ini");
    if (!QFile::exists(path)) return QString();
    
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
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/knighttimerc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("Location"));
    return group.readEntry(QStringLiteral("Latitude"), 0.0);
}

double ThemeReader::nativeLongitude() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/knighttimerc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("Location"));
    return group.readEntry(QStringLiteral("Longitude"), 0.0);
}

QStringList ThemeReader::listGlobalThemes() {
    QStringList themes;
    QStringList searchPaths = {
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/plasma/look-and-feel"),
        QStringLiteral("/usr/share/plasma/look-and-feel")
    };

    for (const QString &path : searchPaths) {
        if (!QDir(path).exists()) continue;
        
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

int ThemeReader::solarPadding() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    return group.readEntry(QStringLiteral("SolarPadding"), 0);
}

QString ThemeReader::dayKvantumTheme() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    return group.readEntry(QStringLiteral("DayKvantumTheme"), QString());
}

QString ThemeReader::nightKvantumTheme() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    return group.readEntry(QStringLiteral("NightKvantumTheme"), QString());
}

QStringList ThemeReader::listGtkThemes() {
    QStringList themes;
    QStringList paths = {
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QStringLiteral("/.themes"),
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/themes"),
        QStringLiteral("/usr/share/themes")
    };

    for (const QString &path : paths) {
        if (!QDir(path).exists()) continue;
        
        QDirIterator it(path, QDir::Dirs | QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            it.next();
            QString name = it.fileName();
            // Validation: Check if it looks like a theme
            bool isValid = false;
            
            if (QFile::exists(it.filePath() + "/index.theme")) isValid = true;
            else if (QFile::exists(it.filePath() + "/gtk-3.0/gtk.css")) isValid = true;
            else if (QFile::exists(it.filePath() + "/gtk-4.0/gtk.css")) isValid = true;
            else if (QFile::exists(it.filePath() + "/gtk-2.0/gtkrc")) isValid = true;
            
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
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    return group.readEntry(QStringLiteral("DayGtkTheme"), QString());
}

QString ThemeReader::nightGtkTheme() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/plasma-theme-masterrc");
    KConfig config(path, KConfig::SimpleConfig);
    KConfigGroup group = config.group(QStringLiteral("General"));
    return group.readEntry(QStringLiteral("NightGtkTheme"), QString());
}
