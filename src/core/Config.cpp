#include "Config.h"
#include <KConfig>
#include <KConfigGroup>

static const QString CONFIG_FILE = QStringLiteral("plasma-theme-masterrc");
static const QString GROUP_LOC   = QStringLiteral("Location");

double Config::latitude() {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    return config.group(GROUP_LOC).readEntry("Latitude", 0.0);
}

double Config::longitude() {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    return config.group(GROUP_LOC).readEntry("Longitude", 0.0);
}

void Config::setLatitude(double val) {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    config.group(GROUP_LOC).writeEntry("Latitude", val);
    config.sync();
}

void Config::setLongitude(double val) {
    KConfig config(CONFIG_FILE, KConfig::SimpleConfig);
    config.group(GROUP_LOC).writeEntry("Longitude", val);
    config.sync();
}

static const QString GROUP_GENERAL = QStringLiteral("General");

bool Config::isMaterialYouOverrideEnabled() {
    return KConfig(CONFIG_FILE).group(GROUP_GENERAL).readEntry("MaterialYouOverride", false);
}
void Config::setMaterialYouOverrideEnabled(bool enabled) {
    KConfig c(CONFIG_FILE); c.group(GROUP_GENERAL).writeEntry("MaterialYouOverride", enabled); c.sync();
}

double Config::materialYouChroma() {
    return KConfig(CONFIG_FILE).group(GROUP_GENERAL).readEntry("MaterialYouChroma", 1.0);
}
void Config::setMaterialYouChroma(double chroma) {
    KConfig c(CONFIG_FILE); c.group(GROUP_GENERAL).writeEntry("MaterialYouChroma", chroma); c.sync();
}

double Config::materialYouTone() {
    return KConfig(CONFIG_FILE).group(GROUP_GENERAL).readEntry("MaterialYouTone", 1.0);
}
void Config::setMaterialYouTone(double tone) {
    KConfig c(CONFIG_FILE); c.group(GROUP_GENERAL).writeEntry("MaterialYouTone", tone); c.sync();
}

int Config::materialYouSchemeVariant() {
    return KConfig(CONFIG_FILE).group(GROUP_GENERAL).readEntry("MaterialYouSchemeVariant", 5);
}
void Config::setMaterialYouSchemeVariant(int variant) {
    KConfig c(CONFIG_FILE); c.group(GROUP_GENERAL).writeEntry("MaterialYouSchemeVariant", variant); c.sync();
}
