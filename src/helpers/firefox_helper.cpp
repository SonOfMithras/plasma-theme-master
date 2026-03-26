// firefox_helper.cpp
// Discovers Firefox / Zen profile folders and injects plasma-colors.css.
// The CSS content is read from stdin (piped by the main app after template render)
// or from a path passed as argv[1].

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>

static bool injectIntoProfile(const QString &profileDir, const QString &cssContent) {
    QString chromeDir = profileDir + "/chrome";
    if (!QDir(chromeDir).exists())
        QDir().mkpath(chromeDir);

    // Write plasma-colors.css
    QString cssPath = chromeDir + "/plasma-colors.css";
    QFile cssFile(cssPath);
    if (!cssFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;
    QTextStream(&cssFile) << cssContent;
    cssFile.close();

    // Ensure userChrome.css imports it
    QString userChromePath = chromeDir + "/userChrome.css";
    QString importLine = "@import \"plasma-colors.css\";";
    QFile uc(userChromePath);

    if (uc.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString existing = QTextStream(&uc).readAll();
        if (!existing.contains("plasma-colors.css")) {
            uc.seek(0);
            QTextStream(&uc) << importLine << "\n" << existing;
        }
        uc.close();
    } else {
        if (uc.open(QIODevice::WriteOnly | QIODevice::Text))
            QTextStream(&uc) << importLine << "\n";
    }

    return true;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Read CSS from file path argument or stdin
    QString cssContent;
    if (argc > 1) {
        QFile f(QString::fromLocal8Bit(argv[1]));
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
            cssContent = QTextStream(&f).readAll();
    } else {
        QTextStream in(stdin);
        cssContent = in.readAll();
    }

    if (cssContent.isEmpty()) {
        qWarning() << "Firefox helper: no CSS content provided";
        return 1;
    }

    struct BrowserPath { QString name, path; };
    QList<BrowserPath> browsers = {
        {"Firefox",      QDir::homePath() + "/.mozilla/firefox"},
        {"Zen",          QDir::homePath() + "/.zen"},
        {"Zen (Flatpak)",QDir::homePath() + "/.var/app/app.zen_browser.zen/.zen"},
    };

    bool any = false;
    for (const auto &browser : browsers) {
        QDir dir(browser.path);
        if (!dir.exists()) continue;

        QStringList profiles = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &profile : profiles) {
            if (profile.contains("default") || profile.contains("release")
                    || profiles.count() < 3) {
                QString profilePath = browser.path + "/" + profile;
                if (injectIntoProfile(profilePath, cssContent)) {
                    qInfo() << "Firefox helper: injected into" << browser.name << profile;
                    any = true;
                }
            }
        }
    }

    return any ? 0 : 1;
}
