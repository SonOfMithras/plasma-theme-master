#ifndef TEMPLATECONFIG_H
#define TEMPLATECONFIG_H

#include <QMap>
#include <QString>
#include <QList>
#include <QVariant>

/**
 * One entry from [templates.NAME] in config.toml.
 */
struct TemplateEntry {
    QString name;
    bool    enabled     = false;
    QString inputPath;   // absolute (~ expanded), empty = helper-only
    QString outputPath;  // absolute (~ expanded), empty = auto/helper
    QString palette;     // "day" | "night" | "current"
    QString postHook;
    QMap<QString, QString> extra; // app-specific keys (e.g. midnight_enabled, custom_imports)
};

/**
 * TemplateConfig - reads/writes ~/.config/plasma-theme-master/config.toml.
 *
 * On first run, the default config is copied from the system data dir.
 * GUI toggles call setEnabled() / setValue() to mutate the TOML in place.
 */
class TemplateConfig {
public:
    // -----------------------------------------------------------------------
    // Path resolution
    // -----------------------------------------------------------------------
    static QString userConfigPath();    // ~/.config/plasma-theme-master/config.toml
    static QString systemConfigPath();  // /usr/share/plasma-theme-master/config.toml.default

    /** Copy system default → user config if the user file doesn't exist yet. */
    static void ensureUserConfig();

    /** Merge any new templates from system config.toml.default into user's config.toml */
    static void mergeMissingTemplates();

    // -----------------------------------------------------------------------
    // Reading
    // -----------------------------------------------------------------------
    /** Load all [templates.*] entries from config.toml (enabled or not). */
    static QList<TemplateEntry> loadTemplates();

    /** Quick lookup — is this template entry enabled? */
    static bool isEnabled(const QString &name);

    /** Read a specific extra string value from a [templates.NAME] section. */
    static QString getValue(const QString &name,
                            const QString &key,
                            const QString &defaultVal = {});

    /** Read a string-array extra value (stored as TOML array). */
    static QStringList getList(const QString &name,
                                const QString &key);

    // -----------------------------------------------------------------------
    // Writing  (mutates config.toml in place)
    // -----------------------------------------------------------------------
    /** Enable or disable a template entry. */
    static void setEnabled(const QString &name, bool enabled);

    /** Set a string value in a [templates.NAME] section. */
    static void setValue(const QString &name,
                         const QString &key,
                         const QString &value);

    /** Set a string list in a [templates.NAME] section. */
    static void setList(const QString &name,
                        const QString &key,
                        const QStringList &values);

    // -----------------------------------------------------------------------
    // App Templates (Unified Built-in & Community Templates)
    // -----------------------------------------------------------------------
    /** Is this application template enabled? */
    static bool isAppEnabled(const QString &appId);
    static void setAppEnabled(const QString &appId, bool enabled);

    /** Master toggle: Are built-in templates enabled globally? */
    static bool isBuiltinTemplatesEnabled();
    static void setBuiltinTemplatesEnabled(bool enabled);

    /** Compatibility aliases */
    static bool isBuiltinAppEnabled(const QString &appId) { return isAppEnabled(appId); }
    static void setBuiltinAppEnabled(const QString &appId, bool enabled) { setAppEnabled(appId, enabled); }
    static bool isCommunityAppEnabled(const QString &catalogId) { return isAppEnabled(catalogId); }
    static void setCommunityAppEnabled(const QString &catalogId, bool enabled) { setAppEnabled(catalogId, enabled); }

    /** Obsidian vaults list configured in config.toml */
    static QStringList getObsidianVaults();

    /** Set Obsidian vaults list in config.toml */
    static void setObsidianVaults(const QStringList &vaults);

    /** Is Obsidian auto-detection of vaults enabled? */
    static bool isObsidianAutoDetectEnabled();

    /** Set Obsidian auto-detection flag */
    static void setObsidianAutoDetect(bool enabled);

    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------
    /** Expand ~ to the home directory in a path string. */
    static QString expandPath(const QString &path);
};

#endif // TEMPLATECONFIG_H
