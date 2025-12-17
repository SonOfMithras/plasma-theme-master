#include "GlobalThemeManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "Logger.h"

QList<GlobalThemeInfo> GlobalThemeManager::listInstalledThemes() {
    QList<GlobalThemeInfo> themes;
    QStringList paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "plasma/look-and-feel", QStandardPaths::LocateDirectory);
    
    // Helper to avoid duplicates (User overrrides System)
    QStringList seen;

    // Iterate backwards (User paths usually first in locateAll? No, check docs. 
    // locateAll returns list of paths. Typically /home/... then /usr/share... 
    // We want to process them all.
    
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
    
    // Copy Directory Recursively
    // Use 'cp -r' for simplicity and robustness on Linux
    QString cmd = QString("cp -r \"%1\" \"%2\"").arg(srcInfo.path).arg(userPath);
    int ret = std::system(qPrintable(cmd));
    
    if (ret == 0) {
        // Update metadata.json Name if possible
        QString metaPath = userPath + "/metadata.json";
        // Simple string replace for now? Or parse JSON.
        // Let's leave metadata alone for now, or just log success.
        Logger::log("Cloned theme " + srcName + " to " + newName, Logger::Info);
        return true;
    } else {
        Logger::log("Clone failed: cp command returned " + QString::number(ret), Logger::Error);
        return false;
    }
}

QString GlobalThemeManager::getDefaultsPath(const QString &themeName) {
    // Find path
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
    
    // Backup Logic: Create .bak if it doesn't exist
    QString backupPath = path + ".bak";
    if (!QFile::exists(backupPath) && isUserTheme(themeName)) {
        if (!file.copy(backupPath)) {
             Logger::log("Failed to create backup for " + themeName, Logger::Warning);
        } else {
             Logger::log("Created backup for " + themeName, Logger::Info);
        }
        // Reposition file pointer after copy just in case? 
        // copy uses the filename primarily, but if it used the fd... 
        // QFile::copy(newName) copies the file currently specified by fileName(). 
        // It does not affect the open handle usually, but let's be safe.
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
    
    // Remove current and copy back
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
    
    // Ensure writable
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
    // Categories based on python impl mapping
    if (category == "colors") {
        paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "color-schemes", QStandardPaths::LocateDirectory);
    } else if (category == "icons") {
        paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "icons", QStandardPaths::LocateDirectory);
    } else if (category == "plasma_style") {
        paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "plasma/desktoptheme", QStandardPaths::LocateDirectory);
    } else if (category == "cursors") {
        paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "icons", QStandardPaths::LocateDirectory); // Cursors often in icons
    } else if (category == "window_decorations") {
         // This is harder. For now return empty or try scanning standard aurorae paths
         paths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "kwin/decorations", QStandardPaths::LocateDirectory);
         // Also aurorae themes are in ~/.local/share/aurorae/themes or /usr/share/aurorae/themes
         QStringList aurorae = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "aurorae/themes", QStandardPaths::LocateDirectory);
         paths.append(aurorae);
    } else if (category == "window_decoration_engines") {
         return QStringList{"org.kde.kwin.aurorae", "org.kde.breeze", "org.kde.oxygen"};
    } else if (category == "application_styles") {
         // KStyle. Hard to list via files as they are plugins (shared libs).
         // Return basic known ones for now?
         return QStringList{"Breeze", "Oxygen", "Fusion", "Windows", "kvantum"};
    }
    
    QStringList results;
    for (const QString &path : paths) {
        QDir dir(path);
        QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &e : entries) {
            if (!results.contains(e)) results.append(e);
        }
        // For color schemes, they are files ending in .colors
        if (category == "colors") {
             QStringList files = dir.entryList(QStringList() << "*.colors", QDir::Files);
             for (const QString &f : files) {
                 results.append(f.section('.', 0, -2)); // Remove extension
             }
        }
    }
    
    // De-duplicate
    results.removeDuplicates(); // Actually handle in collection
    std::sort(results.begin(), results.end()); // List sort
    return results;
}

bool GlobalThemeManager::isUserTheme(const QString &themeName) {
     QString path = getDefaultsPath(themeName);
     return path.startsWith(QDir::homePath());
}
