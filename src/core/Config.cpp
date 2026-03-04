#include "Config.h"
#include <KConfig>
#include <KConfigGroup>
#include <iostream>

static const QString CONFIG_FILE = QStringLiteral("plasma-theme-masterrc");
static const QString GROUP_LOC = QStringLiteral("Location");

double Config::latitude() {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    KConfigGroup group = config.group(GROUP_LOC);
    return group.readEntry("Latitude", 0.0);
}

double Config::longitude() {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    KConfigGroup group = config.group(GROUP_LOC);
    return group.readEntry("Longitude", 0.0);
}

void Config::setLatitude(double val) {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    KConfigGroup group = config.group(GROUP_LOC);
    group.writeEntry("Latitude", val);
    config.sync();
}

void Config::setLongitude(double val) {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    KConfigGroup group = config.group(GROUP_LOC);
    group.writeEntry("Longitude", val);
    config.sync();
}

static const QString GROUP_UNIVERSAL = QStringLiteral("UniversalTheme");

bool Config::isVSCodeSyncEnabled() {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    return config.group(GROUP_UNIVERSAL).readEntry("VSCode", false);
}
void Config::setVSCodeSyncEnabled(bool enabled) {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    config.group(GROUP_UNIVERSAL).writeEntry("VSCode", enabled);
    config.sync();
}

bool Config::isFirefoxSyncEnabled() { return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("Firefox", false); }
void Config::setFirefoxSyncEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("Firefox", enabled); c.sync();
}

bool Config::isBetterDiscordSyncEnabled() { return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("BetterDiscord", false); }
void Config::setBetterDiscordSyncEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("BetterDiscord", enabled); c.sync();
}

bool Config::isKittySyncEnabled() { return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("Kitty", false); }
void Config::setKittySyncEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("Kitty", enabled); c.sync();
}

bool Config::isVencordSyncEnabled() { return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("Vencord", false); }
void Config::setVencordSyncEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("Vencord", enabled); c.sync();
}

bool Config::isBtopSyncEnabled() { return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("Btop", false); }
void Config::setBtopSyncEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("Btop", enabled); c.sync();
}

bool Config::isVicinaeSyncEnabled() { return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("Vicinae", false); }
void Config::setVicinaeSyncEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("Vicinae", enabled); c.sync();
}

bool Config::isMaterialYouOverrideEnabled() { return KConfig(CONFIG_FILE).group("General").readEntry("MaterialYouOverride", false); }
void Config::setMaterialYouOverrideEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group("General").writeEntry("MaterialYouOverride", enabled); c.sync();
}

QString Config::obsidianVaultPath() { return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("ObsidianVaultPath", ""); }
void Config::setObsidianVaultPath(const QString &path) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("ObsidianVaultPath", path); c.sync();
}

bool Config::isObsidianSyncEnabled() { return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("Obsidian", false); }
void Config::setObsidianSyncEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("Obsidian", enabled); c.sync();
}

bool Config::isBetterDiscordMaterialEnabled() {
    return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("BetterDiscordUseMaterial", true);
}
void Config::setBetterDiscordMaterialEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("BetterDiscordUseMaterial", enabled); c.sync();
}

QStringList Config::betterDiscordImports() {
    return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("BetterDiscordImports", QStringList());
}
void Config::setBetterDiscordImports(const QStringList &imports) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("BetterDiscordImports", imports); c.sync();
}

bool Config::isVencordMidnightEnabled() {
    return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("VencordUseMidnight", true);
}
void Config::setVencordMidnightEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("VencordUseMidnight", enabled); c.sync();
}

QStringList Config::vencordImports() {
    return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("VencordImports", QStringList());
}
void Config::setVencordImports(const QStringList &imports) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("VencordImports", imports); c.sync();
}

bool Config::isKonsoleSyncEnabled() { return KConfig(CONFIG_FILE).group(GROUP_UNIVERSAL).readEntry("Konsole", false); }
void Config::setKonsoleSyncEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_UNIVERSAL).writeEntry("Konsole", enabled); c.sync();
}
