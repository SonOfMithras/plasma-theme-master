#include "Config.h"
#include <KConfig>
#include <KConfigGroup>

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
