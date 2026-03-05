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

    static bool isVencordSyncEnabled();
    static void setVencordSyncEnabled(bool enabled);
    
    static bool isZedSyncEnabled();
    static void setZedSyncEnabled(bool enabled);
    
    static bool isKittySyncEnabled();
    static void setKittySyncEnabled(bool enabled);
    
    static bool isKonsoleSyncEnabled();
    static void setKonsoleSyncEnabled(bool enabled);

    static bool isBtopSyncEnabled();
    static void setBtopSyncEnabled(bool enabled);

    static bool isVicinaeSyncEnabled();
    static void setVicinaeSyncEnabled(bool enabled);

    static bool isMaterialYouOverrideEnabled();
    static void setMaterialYouOverrideEnabled(bool enabled);

    static QString obsidianVaultPath();
    static void setObsidianVaultPath(const QString &path);
    static bool isObsidianSyncEnabled();
    static void setObsidianSyncEnabled(bool enabled);

    // BetterDiscord Advanced
    static bool isBetterDiscordMaterialEnabled();
    static void setBetterDiscordMaterialEnabled(bool enabled);
    static QStringList betterDiscordImports();
    static void setBetterDiscordImports(const QStringList &imports);

    // Vencord Advanced
    static bool isVencordMidnightEnabled();
    static void setVencordMidnightEnabled(bool enabled);
    static QStringList vencordImports();
    static void setVencordImports(const QStringList &imports);

};

#endif // CONFIG_H
