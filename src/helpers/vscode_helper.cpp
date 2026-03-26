// vscode_helper.cpp
// Merges workbench.colorCustomizations into VSCode/VSCodium/Antigravity settings.json.
// Called by post_hook from [templates.vscode] in config.toml.
// Reads palette variables from environment (set by main app before exec).

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QDebug>

static QString colorFromEnv(const QString &key) {
    return QString::fromLocal8Bit(qgetenv(key.toLocal8Bit()));
}

static bool mergeVSCodeSettings(const QString &path, const QJsonObject &colors) {
    QFile file(path);
    QFileInfo fi(path);
    if (!QDir(fi.absolutePath()).exists())
        QDir().mkpath(fi.absolutePath());

    QJsonObject root;
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) root = doc.object();
    }

    QJsonObject existing;
    if (root.contains("workbench.colorCustomizations"))
        existing = root["workbench.colorCustomizations"].toObject();

    for (auto it = colors.begin(); it != colors.end(); ++it)
        existing[it.key()] = it.value();

    root["workbench.colorCustomizations"] = existing;

    QSaveFile sf(path);
    if (sf.open(QIODevice::WriteOnly)) {
        sf.write(QJsonDocument(root).toJson());
        if (sf.commit()) {
            qInfo() << "VSCode helper: wrote" << path;
            return true;
        }
    }
    qWarning() << "VSCode helper: failed to write" << path;
    return false;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Read colors injected by the main process via environment variables
    QJsonObject colors;
    colors["titleBar.activeBackground"]  = colorFromEnv("PTM_TITLEBAR_BG");
    colors["titleBar.activeForeground"]  = colorFromEnv("PTM_TITLEBAR_FG");
    colors["activityBar.background"]     = colorFromEnv("PTM_WINDOW_BG");
    colors["sideBar.background"]         = colorFromEnv("PTM_WINDOW_BG");
    colors["statusBar.background"]       = colorFromEnv("PTM_WINDOW_BG");
    colors["statusBar.foreground"]       = colorFromEnv("PTM_VIEW_FG");
    colors["editor.background"]          = colorFromEnv("PTM_VIEW_BG");
    colors["editor.foreground"]          = colorFromEnv("PTM_VIEW_FG");
    colors["list.activeSelectionBackground"] = colorFromEnv("PTM_SELECTION");
    colors["terminal.background"]        = colorFromEnv("PTM_ANSI_BLACK");
    colors["terminal.foreground"]        = colorFromEnv("PTM_ANSI_WHITE");

    // If all empty (not called from main app), bail silently
    if (colors["editor.background"].toString().isEmpty()) {
        qWarning() << "VSCode helper: No palette env vars set. "
                      "Run via plasma-theme-master sync-universal.";
        return 1;
    }

    QStringList paths = {
        QDir::homePath() + "/.config/Code/User/settings.json",
        QDir::homePath() + "/.config/Code - OSS/User/settings.json",
        QDir::homePath() + "/.config/VSCodium/User/settings.json",
        QDir::homePath() + "/.config/Antigravity/User/settings.json",
        QDir::homePath() + "/.config/antigravity/User/settings.json",
        QDir::homePath() + "/.antigravity/User/settings.json",
    };

    bool any = false;
    for (const QString &p : paths) {
        if (QFile::exists(p) || p.contains("Code/"))
            if (mergeVSCodeSettings(p, colors)) any = true;
    }

    return any ? 0 : 1;
}
