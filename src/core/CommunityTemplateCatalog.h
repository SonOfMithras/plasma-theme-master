#ifndef COMMUNITYTEMPLATECATALOG_H
#define COMMUNITYTEMPLATECATALOG_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>

struct CommunityTemplateEntry {
    QString name;                          // e.g. "bat", "fuzzel"
    QString inputPath;                     // relative to templateDir or absolute
    QMap<QString, QString> inputPathModes; // "dark" -> "...", "light" -> "..."
    QStringList outputPaths;               // one or more output paths (supports ~ and $XDG_*)
    QString outputPathDynamic;             // command returning output paths (e.g. obsidian)
    QString preHook;                       // shell command
    QString postHook;                      // shell command
    QString undoHook;                      // shell command
    QString requiresPath;                  // path check before applying
    bool hookAsync = true;
};

struct CommunityTemplateApp {
    QString id;                            // directory name, e.g. "obsidian", "bat"
    QString name;                          // display name, e.g. "Obsidian", "bat"
    QString category;                      // "terminal", "editor", "browser", "chat", "launcher", "system", "audio", "gaming", "ai", "misc"
    QString templateDir;                   // absolute path to template folder
    QString readmePath;                    // absolute path to README.md
    QString description;                   // short summary extracted from README.md or fallback
    QString documentationUrl;              // docs link
    bool isBuiltin = false;                // is this one of Noctalia's built-in templates
    bool isUser = false;                   // is this a user-created template in ~/.local/share/plasma-theme-master/templates
    QList<CommunityTemplateEntry> entries;

    bool isInstalled() const;
    QString resolvedCategoryName() const;
    QString categoryIconName() const;
};

class CommunityTemplateCatalog {
public:
    static CommunityTemplateCatalog &instance();

    // Reload all templates from standard template directories
    void reload();

    // List all loaded template apps (both builtin and community)
    const QList<CommunityTemplateApp> &apps() const;

    // List only built-in apps (Alacritty, Btop, Cava, etc.)
    const QList<CommunityTemplateApp> &builtinApps() const;

    // List only community apps
    const QList<CommunityTemplateApp> &communityApps() const;

    int bundledCount() const;
    int userCount() const;

    // Find app by ID
    const CommunityTemplateApp *findApp(const QString &id) const;

    // Standard directories scanned
    static QStringList templateSearchPaths();

    // Helper: resolve output path string expanding ~, $XDG_CONFIG_HOME, etc.
    static QString expandPath(const QString &rawPath);

    // Helper: run a dynamic output command and return stdout lines as paths
    static QStringList resolveDynamicOutputPaths(const QString &cmd, const QString &templateDir);

private:
    CommunityTemplateCatalog();
    void loadFromDirectory(const QString &dirPath);

    QList<CommunityTemplateApp> m_apps;
    QList<CommunityTemplateApp> m_builtinApps;
    QList<CommunityTemplateApp> m_communityApps;
    QMap<QString, int> m_idToIndex;
};

#endif // COMMUNITYTEMPLATECATALOG_H
