// vscode_helper.cpp
// Deploys the native "Plasma Theme Master" VS Code extension and configures
// workbench.colorTheme in VSCode, VSCodium, and Antigravity IDE settings.json.
// Completely replaces hacky workbench.colorCustomizations piggybacking with our
// own first-class theme extension.

#include <QCoreApplication>
#include <QDateTime>

static bool isHexDark(const QString &hex) {
  QString h = hex.trimmed();
  if (h.startsWith('#')) h = h.mid(1);
  if (h.size() >= 6) {
    bool okR = false, okG = false, okB = false;
    int r = h.mid(0, 2).toInt(&okR, 16);
    int g = h.mid(2, 2).toInt(&okG, 16);
    int b = h.mid(4, 2).toInt(&okB, 16);
    if (okR && okG && okB) {
      int luma = (r * 299 + g * 587 + b * 114) / 1000;
      return luma < 128;
    }
  }
  return true;
}
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>

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

  // Remove trailing commas
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
        continue;
      }
    }
    finalJson.append(c);
  }

  return finalJson;
}

static bool deployExtension(const QString &extBaseDir) {
  if (!QDir(extBaseDir).exists()) return false;

  QStringList targetFolders;
  bool isUniversal = extBaseDir.contains("antigravity") || extBaseDir.contains("oss");
  if (isUniversal) {
    targetFolders << extBaseDir + "/ptm.ptmtheme-0.0.5-universal";
    targetFolders << extBaseDir + "/plasma-theme-master.plasma-theme-master-1.0.0-universal";
  }
  targetFolders << extBaseDir + "/ptm.ptmtheme-0.0.5";
  targetFolders << extBaseDir + "/plasma-theme-master.plasma-theme-master-1.0.0";

  for (const QString &extDir : targetFolders) {
    QDir().mkpath(extDir + "/themes");

    QString pkgPath = extDir + "/package.json";
    QJsonObject root;
    root["name"] = extDir.contains("plasma-theme-master") ? "plasma-theme-master" : "ptmtheme";
    root["displayName"] = "Plasma Theme Master";
    root["description"] = "Dynamic theme synchronized by Plasma Theme Master";
    root["version"] = extDir.contains("plasma-theme-master") ? "1.0.0" : "0.0.5";
    root["publisher"] = extDir.contains("plasma-theme-master") ? "plasma-theme-master" : "ptm";

    QJsonObject engines;
    engines["vscode"] = "^1.60.0";
    root["engines"] = engines;

    QJsonArray categories;
    categories.append("Themes");
    root["categories"] = categories;

    QJsonArray themes;
    {
      QJsonObject tDark;
      tDark["label"] = "Plasma Theme Master";
      tDark["uiTheme"] = "vs-dark";
      tDark["path"] = "./themes/PTMTheme-color-theme.json";
      tDark["_watch"] = true;
      themes.append(tDark);

      QJsonObject tLight;
      tLight["label"] = "Plasma Theme Master Light";
      tLight["uiTheme"] = "vs";
      tLight["path"] = "./themes/PTMTheme-color-theme.json";
      tLight["_watch"] = true;
      themes.append(tLight);

      QJsonObject tLegacy;
      tLegacy["label"] = "PTMTheme";
      tLegacy["uiTheme"] = "vs-dark";
      tLegacy["path"] = "./themes/PTMTheme-color-theme.json";
      tLegacy["_watch"] = true;
      themes.append(tLegacy);
    }

    QJsonObject contributes;
    contributes["themes"] = themes;
    root["contributes"] = contributes;

    QSaveFile sf(pkgPath);
    if (sf.open(QIODevice::WriteOnly)) {
      sf.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
      sf.commit();
    }
  }

  // Register in extensions.json if present
  QString extensionsJsonPath = extBaseDir + "/extensions.json";
  if (QFile::exists(extensionsJsonPath)) {
    QFile f(extensionsJsonPath);
    if (f.open(QIODevice::ReadOnly)) {
      QByteArray data = f.readAll();
      f.close();
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(data, &err);
      if (doc.isArray()) {
        QJsonArray arr = doc.array();
        bool found = false;
        for (const auto &val : arr) {
          if (val.isObject()) {
            QString id = val.toObject()["identifier"].toObject()["id"].toString().toLower();
            if (id == "ptm.ptmtheme" || id == "plasma-theme-master.plasma-theme-master") {
              found = true;
              break;
            }
          }
        }
        if (!found) {
          QString chosenFolder = targetFolders.first();
          QString relLoc = chosenFolder.section('/', -1);
          QJsonObject extObj;
          QJsonObject ident;
          ident["id"] = "ptm.ptmtheme";
          extObj["identifier"] = ident;
          extObj["version"] = "0.0.5";
          extObj["relativeLocation"] = relLoc;
          QJsonObject loc;
          loc["$mid"] = 1;
          loc["fsPath"] = chosenFolder;
          loc["path"] = chosenFolder;
          loc["scheme"] = "file";
          extObj["location"] = loc;
          QJsonObject meta;
          meta["installedTimestamp"] = QDateTime::currentMSecsSinceEpoch();
          meta["publisherDisplayName"] = "Plasma Theme Master";
          meta["targetPlatform"] = relLoc.contains("universal") ? "universal" : "undefined";
          extObj["metadata"] = meta;
          arr.append(extObj);

          QSaveFile sf(extensionsJsonPath);
          if (sf.open(QIODevice::WriteOnly)) {
            sf.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
            sf.commit();
          }
        }
      }
    }
  }

  return true;
}

static bool updateVSCodeSettings(const QString &path, bool isDark) {
  QFile file(path);
  QFileInfo fi(path);
  if (!QDir(fi.absolutePath()).exists()) {
    QDir().mkpath(fi.absolutePath());
  }

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
        return false;
      }
    }
  }

  // Backup original settings.json if not present
  QString bakPath = path + ".ptm.bak";
  if (!QFile::exists(bakPath) && file.exists()) {
    QFile::copy(path, bakPath);
  }

  // Set workbench.colorTheme directly to Plasma Theme Master
  QString targetTheme = isDark ? "Plasma Theme Master" : "Plasma Theme Master Light";
  root["workbench.colorTheme"] = targetTheme;

  // Clean up piggybacked colorCustomizations so they do not override our native theme
  static const QStringList piggybackKeys = {
      "titleBar.activeBackground", "titleBar.activeForeground",
      "activityBar.background",    "sideBar.background",
      "statusBar.background",      "statusBar.foreground",
      "editor.background",         "editor.foreground",
      "list.activeSelectionBackground",
      "terminal.background",       "terminal.foreground",
      "terminalCursor.background", "terminalCursor.foreground",
      "terminal.ansiBlack",        "terminal.ansiRed",
      "terminal.ansiGreen",        "terminal.ansiYellow",
      "terminal.ansiBlue",         "terminal.ansiMagenta",
      "terminal.ansiCyan",         "terminal.ansiWhite",
      "terminal.ansiBrightBlack",  "terminal.ansiBrightRed",
      "terminal.ansiBrightGreen",  "terminal.ansiBrightYellow",
      "terminal.ansiBrightBlue",   "terminal.ansiBrightMagenta",
      "terminal.ansiBrightCyan",   "terminal.ansiBrightWhite"
  };

  if (root.contains("workbench.colorCustomizations")) {
    QJsonObject existing = root["workbench.colorCustomizations"].toObject();
    for (const QString &k : piggybackKeys) {
      existing.remove(k);
    }
    if (existing.isEmpty()) {
      root.remove("workbench.colorCustomizations");
    } else {
      root["workbench.colorCustomizations"] = existing;
    }
  }

  QSaveFile sf(path);
  if (sf.open(QIODevice::WriteOnly)) {
    sf.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (sf.commit()) {
      qInfo() << "VSCode helper: updated" << path << "to native theme:" << targetTheme;
      return true;
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  // Determine if active palette is dark or light
  bool isDark = true;
  QString isDarkEnv = QString::fromLocal8Bit(qgetenv("PTM_IS_DARK"));
  if (!isDarkEnv.isEmpty()) {
    isDark = (isDarkEnv.toLower() != "light");
  } else {
    QString bgEnv = QString::fromLocal8Bit(qgetenv("PTM_WINDOW_BG"));
    if (!bgEnv.isEmpty()) {
      isDark = isHexDark(bgEnv);
    } else {
      // Fallback: query kreadconfig6
      QProcess proc;
      proc.start("kreadconfig6", {"--file", "kdeglobals", "--group", "Colors:Window", "--key", "BackgroundNormal"});
      if (proc.waitForFinished(1000)) {
        QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        QStringList parts = out.split(',');
        if (parts.size() >= 3) {
          int r = parts[0].toInt(), g = parts[1].toInt(), b = parts[2].toInt();
          int luma = (r * 299 + g * 587 + b * 114) / 1000;
          isDark = (luma < 128);
        }
      }
    }
  }

  struct EditorTarget {
    QString extDir;
    QStringList settingsPaths;
  };

  QList<EditorTarget> targets = {
    {
      QDir::homePath() + "/.antigravity-ide/extensions",
      {
        QDir::homePath() + "/.config/Antigravity IDE/User/settings.json",
        QDir::homePath() + "/.config/Antigravity/User/settings.json"
      }
    },
    {
      QDir::homePath() + "/.vscode/extensions",
      {
        QDir::homePath() + "/.config/Code/User/settings.json"
      }
    },
    {
      QDir::homePath() + "/.vscode-oss/extensions",
      {
        QDir::homePath() + "/.config/Code - OSS/User/settings.json",
        QDir::homePath() + "/.config/VSCodium/User/settings.json"
      }
    },
    {
      QDir::homePath() + "/.cursor/extensions",
      {
        QDir::homePath() + "/.config/Cursor/User/settings.json"
      }
    },
    {
      QDir::homePath() + "/.windsurf/extensions",
      {
        QDir::homePath() + "/.config/Windsurf/User/settings.json"
      }
    }
  };

  bool anySuccess = false;
  for (const auto &target : targets) {
    bool hasExt = QDir(target.extDir).exists();
    bool hasSettings = false;
    for (const QString &p : target.settingsPaths) {
      if (QFile::exists(p) || QDir(QFileInfo(p).absolutePath()).exists()) {
        hasSettings = true;
        break;
      }
    }

    if (!hasExt && !hasSettings) continue;

    if (hasExt || hasSettings) {
      if (deployExtension(target.extDir)) {
        anySuccess = true;
      }
    }

    for (const QString &settingsPath : target.settingsPaths) {
      if (QFile::exists(settingsPath) || QDir(QFileInfo(settingsPath).absolutePath()).exists()) {
        if (updateVSCodeSettings(settingsPath, isDark)) {
          anySuccess = true;
        }
      }
    }
  }

  return anySuccess ? 0 : 1;
}
