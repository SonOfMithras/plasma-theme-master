#ifndef GLOBALTHEMEMANAGER_H
#define GLOBALTHEMEMANAGER_H

#include <QString>
#include <QStringList>
#include <QList>

struct GlobalThemeInfo {
    QString name;
    QString path;
    bool isSystem;
};

class GlobalThemeManager {
public:
    // List all installed global themes (System + User)
    static QList<GlobalThemeInfo> listInstalledThemes();

    // Clone a theme to the user's local directory
    static bool cloneTheme(const QString &srcName, const QString &newName);

    // Read contents/defaults file
    static QString readDefaults(const QString &themeName);

    // Write contents/defaults file
    static bool writeDefaults(const QString &themeName, const QString &content);
    static QString restoreDefaults(const QString &themeName); // Returns content or empty on failure

    // List available sub-themes (colors, icons, etc.)
    // Categories: "colors", "icons", "application_styles", "plasma_style", "window_decorations"
    static QStringList listSubThemes(const QString &category);
    
    // Check if a theme is user-writable
    static bool isUserTheme(const QString &themeName);
    
    // Get absolute path to details file
    static QString getDefaultsPath(const QString &themeName);
};

#endif // GLOBALTHEMEMANAGER_H
