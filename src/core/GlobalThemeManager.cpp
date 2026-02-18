#include "GlobalThemeManager.h"
#include <QStandardPaths>
#include <KConfig>
#include <KConfigGroup>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStyleFactory>
#include "Logger.h"

QList<GlobalThemeInfo> GlobalThemeManager::listInstalledThemes() {
    QList<GlobalThemeInfo> themes;
    QStringList paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "plasma/look-and-feel", QStandardPaths::LocateDirectory);
    
    QStringList seen;

    
    for (const QString &path : paths) {
        QDir dir(path);
        QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            if (seen.contains(entry)) continue;
            
            GlobalThemeInfo info;
            info.name = entry;
            info.path = dir.absoluteFilePath(entry);
            info.isSystem = !info.path.startsWith(QDir::homePath());
            
            themes.append(info);
            seen.append(entry);
        }
    }
    return themes;
}

bool GlobalThemeManager::cloneTheme(const QString &srcName, const QString &newName) {
    QList<GlobalThemeInfo> themes = listInstalledThemes();
    GlobalThemeInfo srcInfo;
    bool found = false;
    for (const auto &t : themes) {
        if (t.name == srcName) {
            srcInfo = t;
            found = true;
            break;
        }
    }
    
    if (!found) {
        Logger::log("Clone failed: Source theme not found: " + srcName, Logger::Error);
        return false;
    }
    
    QString userPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/plasma/look-and-feel/" + newName;
    if (QFile::exists(userPath)) {
        Logger::log("Clone failed: Destination already exists: " + userPath, Logger::Error);
        return false;
    }
    
    QString cmd = QString("cp -r \"%1\" \"%2\"").arg(srcInfo.path).arg(userPath);
    int ret = std::system(qPrintable(cmd));
    
    if (ret == 0) {
        QString metaPath = userPath + "/metadata.json";
        QFile metaFile(metaPath);
        if (metaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString content = QString::fromUtf8(metaFile.readAll());
            metaFile.close();

        }
        
        
        if (metaFile.open(QIODevice::ReadOnly)) {
            QByteArray data = metaFile.readAll();
            metaFile.close();
            
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject root = doc.object();
                
                if (root.contains("KPlugin")) {
                    QJsonObject plugin = root["KPlugin"].toObject();
                    plugin["Id"] = newName;
                    plugin["Name"] = newName;
                    root["KPlugin"] = plugin;
                } else {
                     if (root.contains("Id")) root["Id"] = newName;
                     if (root.contains("Name")) root["Name"] = newName;
                }
                
                doc.setObject(root);
                
                if (metaFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    metaFile.write(doc.toJson());
                    metaFile.close();
                    Logger::log("Updated metadata for cloned theme: " + newName, Logger::Info);
                } else {
                     Logger::log("Failed to write metadata for: " + newName, Logger::Error);
                }
            }
        }

        QString desktopPath = userPath + "/metadata.desktop";
        if (QFile::exists(desktopPath)) {
            KConfig config(desktopPath, KConfig::SimpleConfig);
            
            if (config.hasGroup("KPlugin")) {
                KConfigGroup group = config.group("KPlugin");
                group.writeEntry("Id", newName);
                group.writeEntry("Name", newName);
            } 
            else if (config.hasGroup("Desktop Entry")) {
                KConfigGroup group = config.group("Desktop Entry");
                if (group.hasKey("X-KDE-PluginInfo-Name")) {
                    group.writeEntry("X-KDE-PluginInfo-Name", newName);
                }
                group.writeEntry("Name", newName);
            }
            config.sync();
            Logger::log("Updated metadata.desktop for cloned theme: " + newName, Logger::Info);
        }
    
        Logger::log("Cloned theme " + srcName + " to " + newName, Logger::Info);
        return true;
    } else {
        Logger::log("Clone failed: cp command returned " + QString::number(ret), Logger::Error);
        return false;
    }
}

QString GlobalThemeManager::getDefaultsPath(const QString &themeName) {
    QList<GlobalThemeInfo> themes = listInstalledThemes();
    for (const auto &t : themes) {
        if (t.name == themeName) {
            return t.path + "/contents/defaults";
        }
    }
    return QString();
}

QString GlobalThemeManager::readDefaults(const QString &themeName) {
    QString path = getDefaultsPath(themeName);
    if (path.isEmpty()) return QString();
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    
    QString backupPath = path + ".bak";
    if (!QFile::exists(backupPath) && isUserTheme(themeName)) {
        if (!file.copy(backupPath)) {
             Logger::log("Failed to create backup for " + themeName, Logger::Warning);
        } else {
             Logger::log("Created backup for " + themeName, Logger::Info);
        }
    }

    QTextStream in(&file);
    return in.readAll();
}

QString GlobalThemeManager::restoreDefaults(const QString &themeName) {
    QString path = getDefaultsPath(themeName);
    if (path.isEmpty()) return QString();
    
    QString backupPath = path + ".bak";
    if (!QFile::exists(backupPath)) {
        Logger::log("Restore failed: No backup found for " + themeName, Logger::Warning);
        return QString();
    }
    
    QFile currentFile(path);
    if (currentFile.exists()) {
        if (!currentFile.remove()) {
            Logger::log("Restore failed: Cannot remove current defaults for " + themeName, Logger::Error);
            return QString();
        }
    }
    
    if (QFile::copy(backupPath, path)) {
        Logger::log("Restored defaults for " + themeName, Logger::Info);
        return readDefaults(themeName); // Return formatted content
    } else {
        Logger::log("Restore failed: Copy from backup failed for " + themeName, Logger::Error);
        return QString();
    }
}

bool GlobalThemeManager::writeDefaults(const QString &themeName, const QString &content) {
    QString path = getDefaultsPath(themeName);
    if (path.isEmpty()) return false;
    
    if (!path.startsWith(QDir::homePath())) {
        Logger::log("Cannot write to system theme: " + themeName, Logger::Error);
        return false;
    }
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;
    
    QTextStream out(&file);
    out << content;
    return true;
}

QStringList GlobalThemeManager::listSubThemes(const QString &category) {
    QStringList paths;
    if (category == "colors") {
        paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "color-schemes", QStandardPaths::LocateDirectory);
    } else if (category == "icons") {
        paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "icons", QStandardPaths::LocateDirectory);
    } else if (category == "plasma_style") {
        paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "plasma/desktoptheme", QStandardPaths::LocateDirectory);
    } else if (category == "cursors") {
        paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "icons", QStandardPaths::LocateDirectory); 
    } else if (category == "window_decorations") {
         paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "kwin/decorations", QStandardPaths::LocateDirectory);
         QStringList aurorae = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "aurorae/themes", QStandardPaths::LocateDirectory);
         paths.append(aurorae);
    } else if (category == "window_decoration_engines") {
         return QStringList{"org.kde.kwin.aurorae", "org.kde.breeze", "org.kde.oxygen"};
    } else if (category == "application_styles") {
         QStringList styles = QStyleFactory::keys();
         // Ensure standard KDE ones are present if for some reason not returned (though they should be)
         if (!styles.contains("Breeze") && styles.contains("breeze")) styles.replaceInStrings("breeze", "Breeze");
         return styles;
    }
    
    QStringList results;
    for (const QString &path : paths) {
        QDir dir(path);
        QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &e : entries) {
            if (category == "cursors") {
                 QDir themeDir(dir.absoluteFilePath(e));
                 if (!themeDir.exists("cursors")) continue;
            }
            
            if (!results.contains(e)) results.append(e);
        }
        if (category == "colors") {
             QStringList files = dir.entryList(QStringList() << "*.colors", QDir::Files);
             for (const QString &f : files) {
                 results.append(f.section('.', 0, -2)); 
             }
        }
    }
    
    results.removeDuplicates(); 
    std::sort(results.begin(), results.end()); 
    return results;
}

bool GlobalThemeManager::isUserTheme(const QString &themeName) {
     QString path = getDefaultsPath(themeName);
     return path.startsWith(QDir::homePath());
}
