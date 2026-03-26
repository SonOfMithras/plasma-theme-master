// konsole_helper.cpp
// Generates PlasmaMaster.profile for Konsole and sets it as default.
// The colorscheme .tpl is rendered by the main app (writing the .colorscheme file).
// This helper only creates the .profile and updates konsolerc.

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QString dataDir = QStandardPaths::writableLocation(
        QStandardPaths::GenericDataLocation);

    // 1. Write profile file
    QString profilePath = dataDir + "/konsole/PlasmaMaster.profile";
    QFileInfo profileInfo(profilePath);
    QDir().mkpath(profileInfo.absolutePath());

    QFile profile(profilePath);
    if (profile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&profile);
        out << "[Appearance]\n"
            << "ColorScheme=PlasmaMaster\n\n"
            << "[General]\n"
            << "Name=PlasmaMaster\n"
            << "Parent=FALLBACK/\n";
        profile.close();
        qInfo() << "Konsole helper: wrote profile" << profilePath;
    } else {
        qWarning() << "Konsole helper: failed to write profile" << profilePath;
        return 1;
    }

    // 2. Set as default profile in konsolerc
    QString configPath = QStandardPaths::writableLocation(
                             QStandardPaths::ConfigLocation) + "/konsolerc";
    KConfig kconfig(configPath, KConfig::SimpleConfig);
    KConfigGroup group = kconfig.group("Desktop Entry");
    group.writeEntry("DefaultProfile", "PlasmaMaster.profile");
    kconfig.sync();
    qInfo() << "Konsole helper: set default profile to PlasmaMaster";

    return 0;
}
