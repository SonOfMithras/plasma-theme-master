// zed_helper.cpp
// Updates ~/.config/zed/settings.json to reference the Plasma Master themes.
// Called by post_hook from [templates.zed] after the theme JSON is rendered.

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTextStream>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QString settingsPath = QDir::homePath() + "/.config/zed/settings.json";
    QFile file(settingsPath);

    QString content = "{}";
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        content = QTextStream(&file).readAll();
        file.close();
    }

    // Backup
    QString backupPath = settingsPath + ".bak";
    if (!QFile::exists(backupPath) && QFile::exists(settingsPath))
        QFile::copy(settingsPath, backupPath);

    QRegularExpression themeRegex(
        R"("theme"\s*:\s*(?:"[^"]*"|\{[^{}]*\}))",
        QRegularExpression::DotMatchesEverythingOption);

    QString newThemeBlock = R"("theme": {
    "mode": "system",
    "light": "Plasma Master Light",
    "dark": "Plasma Master Dark"
  })";

    if (content.contains(themeRegex)) {
        content.replace(themeRegex, newThemeBlock);
    } else {
        int lastBrace = content.lastIndexOf('}');
        if (lastBrace != -1) {
            QString before = content.left(lastBrace).trimmed();
            bool isEmpty = (before == "{");
            if (before.endsWith(',')) before.chop(1);
            content = before + (isEmpty ? "\n  " : ",\n  ") + newThemeBlock + "\n}\n";
        } else {
            content = "{\n  " + newThemeBlock + "\n}\n";
        }
    }

    QSaveFile sf(settingsPath);
    if (sf.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream(&sf) << content;
        if (sf.commit()) {
            qInfo() << "Zed helper: updated settings.json";
            return 0;
        }
    }

    qWarning() << "Zed helper: failed to write" << settingsPath;
    return 1;
}
