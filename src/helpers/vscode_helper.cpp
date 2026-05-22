// vscode_helper.cpp
// Merges workbench.colorCustomizations into VSCode/VSCodium/Antigravity
// settings.json. Called by post_hook from [templates.vscode] in config.toml.
// Reads palette variables from environment (set by main app before exec).

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>

static QString colorFromEnv(const QString &key) {
  return QString::fromLocal8Bit(qgetenv(key.toLocal8Bit()));
}

static QByteArray cleanJsonc(const QByteArray &data) {
  QByteArray clean;
  clean.reserve(data.size());

  bool inString = false;
  bool inLineComment = false;
  bool inBlockComment = false;

  for (int i = 0; i < data.size(); ++i) {
    char c = data.at(i);

    if (inLineComment) {
      if (c == '\n') {
        inLineComment = false;
        clean.append(c);
      }
      continue;
    }

    if (inBlockComment) {
      if (c == '*' && i + 1 < data.size() && data.at(i + 1) == '/') {
        inBlockComment = false;
        i++; // Skip '/'
      }
      continue;
    }

    if (inString) {
      clean.append(c);
      if (c == '\\' && i + 1 < data.size()) {
        clean.append(data.at(i + 1));
        i++;
      } else if (c == '"') {
        inString = false;
      }
      continue;
    }

    // Outside string/comment
    if (c == '"') {
      inString = true;
      clean.append(c);
    } else if (c == '/' && i + 1 < data.size() && data.at(i + 1) == '/') {
      inLineComment = true;
      i++; // Skip '/'
    } else if (c == '/' && i + 1 < data.size() && data.at(i + 1) == '*') {
      inBlockComment = true;
      i++; // Skip '*'
    } else {
      clean.append(c);
    }
  }

  // Remove trailing commas: a comma followed only by whitespace/newlines and then } or ]
  QByteArray finalJson;
  finalJson.reserve(clean.size());
  for (int i = 0; i < clean.size(); ++i) {
    char c = clean.at(i);
    if (c == ',') {
      int j = i + 1;
      while (j < clean.size() && (clean.at(j) == ' ' || clean.at(j) == '\t' ||
                                  clean.at(j) == '\r' || clean.at(j) == '\n')) {
        j++;
      }
      if (j < clean.size() && (clean.at(j) == '}' || clean.at(j) == ']')) {
        continue; // Skip the comma
      }
    }
    finalJson.append(c);
  }

  return finalJson;
}

static bool mergeVSCodeSettings(const QString &path,
                                const QJsonObject &colors) {
  QFile file(path);
  QFileInfo fi(path);
  if (!QDir(fi.absolutePath()).exists())
    QDir().mkpath(fi.absolutePath());

  QJsonObject root;
  if (file.exists()) {
    if (file.open(QIODevice::ReadOnly)) {
      QByteArray data = file.readAll();
      file.close();

      QByteArray cleanedData = cleanJsonc(data);
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(cleanedData, &err);
      if (doc.isObject()) {
        root = doc.object();
      } else {
        qWarning() << "VSCode helper: failed to parse existing settings.json at" << path << ":" << err.errorString();
        qWarning() << "VSCode helper: ABORTING write to prevent overwriting user settings!";
        return false;
      }
    } else {
      qWarning() << "VSCode helper: failed to open settings.json for reading at" << path;
      return false;
    }
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
  colors["titleBar.activeBackground"] = colorFromEnv("PTM_TITLEBAR_BG");
  colors["titleBar.activeForeground"] = colorFromEnv("PTM_TITLEBAR_FG");
  colors["activityBar.background"] = colorFromEnv("PTM_WINDOW_BG");
  colors["sideBar.background"] = colorFromEnv("PTM_WINDOW_BG");
  colors["statusBar.background"] = colorFromEnv("PTM_WINDOW_BG");
  colors["statusBar.foreground"] = colorFromEnv("PTM_VIEW_FG");
  colors["editor.background"] = colorFromEnv("PTM_VIEW_BG");
  colors["editor.foreground"] = colorFromEnv("PTM_VIEW_FG");
  colors["list.activeSelectionBackground"] = colorFromEnv("PTM_SELECTION");
  colors["terminal.background"] = colorFromEnv("PTM_ANSI_BLACK");
  colors["terminal.foreground"] = colorFromEnv("PTM_ANSI_WHITE");

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
      QDir::homePath() + "/.config/Antigravity/User/settings.json", // legacy
      QDir::homePath() + "/.config/Antigravity IDE/User/settings.json",
  };

  bool any = false;
  for (const QString &p : paths) {
    if (QFile::exists(p) || p.contains("Code/"))
      if (mergeVSCodeSettings(p, colors))
        any = true;
  }

  return any ? 0 : 1;
}
