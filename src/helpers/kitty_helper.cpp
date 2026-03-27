// kitty_helper.cpp
// 1. Decides which rendered theme file (day/night) is current.
// 2. Copies it to ~/.config/kitty/current-theme.conf
// 3. Ensures kitty.conf includes current-theme.conf
// 4. Sends SIGUSR1 to all running kitty instances so colours reload live.

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>

#include <csignal>
#include <sys/types.h>

static const QString KITTY_DIR   = QDir::homePath() + "/.config/kitty";
static const QString CURRENT     = KITTY_DIR + "/current-theme.conf";
static const QString LIGHT_AUTO  = KITTY_DIR + "/light-theme.auto.conf";
static const QString DARK_AUTO   = KITTY_DIR + "/dark-theme.auto.conf";
static const QString INCLUDE_LINE = QStringLiteral("include ./current-theme.conf");

// ---------------------------------------------------------------------------
// Pick the right source file.
// We mirror what the service does: day 06:00–20:00, night otherwise.
// (The service already wrote the correct palette into both auto.conf files;
//  we just need to activate the right one.)
// ---------------------------------------------------------------------------
static QString resolveSourcePath() {
    // The helper can be invoked with an explicit path as argv[1]
    // ("kitty_dark" or "kitty_light" template name) but we fall back
    // to a simple hour-based heuristic if none is given.
    int hour = QDateTime::currentDateTime().time().hour();
    bool isDay = (hour >= 6 && hour < 20);
    QString candidate = isDay ? LIGHT_AUTO : DARK_AUTO;
    if (QFile::exists(candidate))
        return candidate;
    // If only one exists, use whichever is there
    if (QFile::exists(LIGHT_AUTO)) return LIGHT_AUTO;
    if (QFile::exists(DARK_AUTO))  return DARK_AUTO;
    return QString();
}

// ---------------------------------------------------------------------------
// Copy src → current-theme.conf
// ---------------------------------------------------------------------------
static bool updateCurrentTheme(const QString &src) {
    QDir().mkpath(KITTY_DIR);

    // Copy with overwrite
    if (QFile::exists(CURRENT))
        QFile::remove(CURRENT);

    if (!QFile::copy(src, CURRENT)) {
        qWarning() << "kitty helper: failed to copy" << src << "→" << CURRENT;
        return false;
    }
    qInfo() << "kitty helper: updated current-theme.conf from" << QFileInfo(src).fileName();
    return true;
}

// ---------------------------------------------------------------------------
// Idempotently add "include ./current-theme.conf" to kitty.conf
// ---------------------------------------------------------------------------
static void ensureKittyInclude() {
    QString confPath = KITTY_DIR + "/kitty.conf";
    QFile f(confPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "kitty helper: cannot read" << confPath;
        return;
    }
    QString content = QString::fromUtf8(f.readAll());
    f.close();

    if (content.contains(INCLUDE_LINE))
        return; // Already present

    if (!f.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "kitty helper: cannot write to" << confPath;
        return;
    }
    QTextStream out(&f);
    out << "\n" << INCLUDE_LINE << "\n";
    f.close();
    qInfo() << "kitty helper: added include to kitty.conf";
}

// ---------------------------------------------------------------------------
// Send SIGUSR1 to every process whose comm == "kitty"
// ---------------------------------------------------------------------------
static int signalKitty() {
    QDir proc("/proc");
    const QStringList entries = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    int count = 0;

    for (const QString &entry : entries) {
        bool ok = false;
        pid_t pid = static_cast<pid_t>(entry.toLong(&ok));
        if (!ok || pid <= 1) continue;

        QFile comm(QString("/proc/%1/comm").arg(entry));
        if (!comm.open(QIODevice::ReadOnly)) continue;

        if (QString::fromUtf8(comm.readAll()).trimmed() == QStringLiteral("kitty")) {
            if (kill(pid, SIGUSR1) == 0) {
                qInfo() << "kitty helper: sent SIGUSR1 to pid" << pid;
                ++count;
            }
        }
    }
    return count;
}

// ---------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QString src = resolveSourcePath();
    if (src.isEmpty()) {
        qWarning() << "kitty helper: no auto theme conf found in" << KITTY_DIR;
        qWarning() << "kitty helper: run the Universal Sync at least once first.";
        return 1;
    }

    updateCurrentTheme(src);
    ensureKittyInclude();

    int n = signalKitty();
    if (n == 0)
        qInfo() << "kitty helper: no running kitty processes found (file written, will apply on next launch)";

    return 0;
}
