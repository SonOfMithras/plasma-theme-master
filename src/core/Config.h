#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

class Config {
public:
    static double latitude();
    static double longitude();
    static void setLatitude(double val);
    static void setLongitude(double val);

    // Universal Sync Settings
    static bool isVSCodeSyncEnabled();
    static void setVSCodeSyncEnabled(bool enabled);

    static bool isFirefoxSyncEnabled();
    static void setFirefoxSyncEnabled(bool enabled);

    static bool isBetterDiscordSyncEnabled();
    static void setBetterDiscordSyncEnabled(bool enabled);

    static bool isKittySyncEnabled();
    static void setKittySyncEnabled(bool enabled);
    
    static bool isGenericSyncEnabled();
    static void setGenericSyncEnabled(bool enabled);

    static QString obsidianVaultPath();
    static void setObsidianVaultPath(const QString &path);
    static bool isObsidianSyncEnabled();
    static void setObsidianSyncEnabled(bool enabled);

    // BetterDiscord Advanced
    static bool isBetterDiscordMaterialEnabled();
    static void setBetterDiscordMaterialEnabled(bool enabled);
    static QStringList betterDiscordImports();
    static void setBetterDiscordImports(const QStringList &imports);
};

#endif // CONFIG_H
