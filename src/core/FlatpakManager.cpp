#include "FlatpakManager.h"
#include "Logger.h"
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

bool FlatpakManager::isFlatpakInstalled() {
  return !QStandardPaths::findExecutable("flatpak").isEmpty();
}

bool FlatpakManager::hasFilesystemAccess() {
  if (!isFlatpakInstalled())
    return false;

  QStringList exposed = getExposedDirectories();

  bool hasThemes = false;
  bool hasGtkConfig = false;

  for (const QString &path : exposed) {
    if (path.startsWith("~/.themes") ||
        path.startsWith("~/.local/share/themes")) {
      hasThemes = true;
    }
    if (path.startsWith("xdg-config/gtk-3.0")) {
      hasGtkConfig = true;
    }
  }

  return hasThemes && hasGtkConfig;
}

QStringList FlatpakManager::getExposedDirectories() {
  QStringList exposed;
  if (!isFlatpakInstalled())
    return exposed;

  QProcess process;
  process.start("flatpak", QStringList() << "override" << "--user" << "--show");
  process.waitForFinished();

  QString output = QString::fromUtf8(process.readAllStandardOutput());

  // Simple parsing to extract filesystem paths
  // Format is usually [Context] filesystems=path1;path2;...

  QRegularExpression re("filesystems=([^\\n]+)");
  QRegularExpressionMatch match = re.match(output);
  if (match.hasMatch()) {
    QString filesystems = match.captured(1);
    exposed = filesystems.split(";", Qt::SkipEmptyParts);
  }

  return exposed;
}

bool FlatpakManager::setupFlatpakEnvironment() {
  if (!isFlatpakInstalled()) {
    Logger::log("Flatpak not installed. Cannot setup environment.",
                Logger::Error);
    return false;
  }

  QStringList args;
  args << "override" << "--user";
  args << "--filesystem=xdg-config/gtk-3.0:ro";
  args << "--filesystem=xdg-config/gtk-4.0:ro";
  args << "--filesystem=~/.icons";
  args << "--filesystem=~/.themes";
  args << "--filesystem=~/.local/share/themes";
  args << "--filesystem=xdg-config/kdeglobals:ro";
  args << "--filesystem=xdg-config/Kvantum:ro";

  QProcess process;
  process.start("flatpak", args);
  process.waitForFinished();

  if (process.exitCode() == 0) {
    Logger::log("Flatpak environment configured successfully.", Logger::Info);
    return true;
  } else {
    QString error = QString::fromUtf8(process.readAllStandardError());
    Logger::log("Failed to setup Flatpak environment: " + error, Logger::Error);
    return false;
  }
}

QString FlatpakManager::flatpakStatus() {
  if (!isFlatpakInstalled()) {
    return "Not Installed";
  }

  if (hasFilesystemAccess()) {
    return "Active (Overrides Set)";
  }

  return "Not Configured";
}

bool FlatpakManager::setFlatpakGtkTheme(const QString &themeName) {
  if (!isFlatpakInstalled())
    return false;

  QProcess process;
  QStringList args;
  args << "override" << "--user"
       << QString("--env=GTK_THEME=%1").arg(themeName);

  process.start("flatpak", args);
  process.waitForFinished();

  if (process.exitCode() == 0) {
    Logger::log(QString("Flatpak GTK_THEME set to: %1").arg(themeName),
                Logger::Info);
    return true;
  } else {
    QString error = QString::fromUtf8(process.readAllStandardError());
    Logger::log(QString("Failed to set Flatpak GTK_THEME: %1").arg(error),
                Logger::Error);
    return false;
  }
}

QString FlatpakManager::getFlatpakGtkTheme() {
    if (!isFlatpakInstalled()) return QString();

    QProcess process;
    process.start("flatpak", QStringList() << "override" << "--user" << "--show");
    process.waitForFinished();

    QString output = QString::fromUtf8(process.readAllStandardOutput());
    
    QRegularExpression re("GTK_THEME=([^\\n]+)");
    QRegularExpressionMatch match = re.match(output);
    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    return QString();
}
