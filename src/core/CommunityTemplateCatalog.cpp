#include "CommunityTemplateCatalog.h"
#include "Logger.h"

#define TOML_EXCEPTIONS 0
#include "third_party/toml.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>

// ---------------------------------------------------------------------------
// CommunityTemplateApp helpers
// ---------------------------------------------------------------------------

bool CommunityTemplateApp::isInstalled() const {
    for (const auto &e : entries) {
        if (!e.requiresPath.isEmpty()) {
            QString path = CommunityTemplateCatalog::expandPath(e.requiresPath);
            if (QFile::exists(path) || QDir(path).exists()) return true;
        }
        for (const QString &out : e.outputPaths) {
            QString path = CommunityTemplateCatalog::expandPath(out);
            QFileInfo fi(path);
            if (fi.dir().exists()) return true;
        }
    }
    return false;
}

QString CommunityTemplateApp::resolvedCategoryName() const {
    QString cat = category.toLower().trimmed();
    if (cat == "terminal")   return QObject::tr("Terminal");
    if (cat == "editor")     return QObject::tr("Editor");
    if (cat == "browser")    return QObject::tr("Browser");
    if (cat == "chat")       return QObject::tr("Chat");
    if (cat == "launcher")   return QObject::tr("Launcher");
    if (cat == "system")     return QObject::tr("System");
    if (cat == "audio")      return QObject::tr("Audio");
    if (cat == "gaming")     return QObject::tr("Gaming");
    if (cat == "ai")         return QObject::tr("AI & Tools");
    return QObject::tr("Misc");
}

QString CommunityTemplateApp::categoryIconName() const {
    QString cat = category.toLower().trimmed();
    if (cat == "terminal")   return "utilities-terminal";
    if (cat == "editor")     return "accessories-text-editor";
    if (cat == "browser")    return "web-browser";
    if (cat == "chat")       return "internet-chat";
    if (cat == "launcher")   return "system-run";
    if (cat == "system")     return "preferences-system";
    if (cat == "audio")      return "multimedia-audio-player";
    if (cat == "gaming")     return "applications-games";
    if (cat == "ai")         return "applications-development";
    return "applications-other";
}

// ---------------------------------------------------------------------------
// CommunityTemplateCatalog
// ---------------------------------------------------------------------------

CommunityTemplateCatalog &CommunityTemplateCatalog::instance() {
    static CommunityTemplateCatalog inst;
    return inst;
}

CommunityTemplateCatalog::CommunityTemplateCatalog() {
    reload();
}

QStringList CommunityTemplateCatalog::templateSearchPaths() {
    QStringList paths;

    // 1. User custom templates
    paths << QDir::homePath() + "/.local/share/plasma-theme-master/templates";

    // 2. Development source path (when running uninstalled or from build tree)
    QString appDirSrcTree = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../src/core/templates");
    if (QDir(appDirSrcTree).exists() && !paths.contains(appDirSrcTree)) paths << appDirSrcTree;

    QString srcTree = QDir::cleanPath(QDir::currentPath() + "/src/core/templates");
    if (QDir(srcTree).exists() && !paths.contains(srcTree)) paths << srcTree;

    // 3. Binary relative path
    QString binRel = QCoreApplication::applicationDirPath() + "/templates";
    if (!paths.contains(binRel)) paths << binRel;

    // 4. System installed templates
    paths << "/usr/share/plasma-theme-master/templates";

    // 5. Generic data locations
    QString dataLoc = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                            "plasma-theme-master/templates",
                                            QStandardPaths::LocateDirectory);
    if (!dataLoc.isEmpty() && !paths.contains(dataLoc)) paths << dataLoc;

    return paths;
}

QString CommunityTemplateCatalog::expandPath(const QString &rawPath) {
    if (rawPath.isEmpty()) return {};

    QString p = rawPath;

    QString home = QDir::homePath();
    QString xdgConfig = qEnvironmentVariable("XDG_CONFIG_HOME", home + "/.config");
    QString xdgData   = qEnvironmentVariable("XDG_DATA_HOME",   home + "/.local/share");
    QString xdgCache  = qEnvironmentVariable("XDG_CACHE_HOME",  home + "/.cache");
    QString xdgState  = qEnvironmentVariable("XDG_STATE_HOME",  home + "/.local/state");

    p.replace("$XDG_CONFIG_HOME", xdgConfig);
    p.replace("${XDG_CONFIG_HOME}", xdgConfig);
    p.replace("$XDG_DATA_HOME", xdgData);
    p.replace("${XDG_DATA_HOME}", xdgData);
    p.replace("$XDG_CACHE_HOME", xdgCache);
    p.replace("${XDG_CACHE_HOME}", xdgCache);
    p.replace("$XDG_STATE_HOME", xdgState);
    p.replace("${XDG_STATE_HOME}", xdgState);
    p.replace("$HOME", home);
    p.replace("${HOME}", home);

    if (p.startsWith("~/")) {
        p = home + p.mid(1);
    } else if (p == "~") {
        p = home;
    }

    return p;
}

QStringList CommunityTemplateCatalog::resolveDynamicOutputPaths(const QString &rawCmd,
                                                               const QString &templateDir) {
    if (rawCmd.isEmpty()) return {};

    QString cmd = rawCmd;
    cmd.replace("{{ config_dir }}", templateDir);
    cmd.replace("{{config_dir}}", templateDir);

    QProcess proc;
    proc.setWorkingDirectory(templateDir);
    proc.start("/bin/sh", {"-c", cmd});
    if (!proc.waitForFinished(5000)) {
        proc.kill();
        return {};
    }

    QString out = QString::fromUtf8(proc.readAllStandardOutput());
    QStringList lines;
    for (const QString &line : out.split('\n')) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty() && !trimmed.startsWith('#')) {
            lines << expandPath(trimmed);
        }
    }
    return lines;
}

const QList<CommunityTemplateApp> &CommunityTemplateCatalog::apps() const {
    return m_apps;
}

const QList<CommunityTemplateApp> &CommunityTemplateCatalog::builtinApps() const {
    return m_builtinApps;
}

const QList<CommunityTemplateApp> &CommunityTemplateCatalog::communityApps() const {
    return m_communityApps;
}

int CommunityTemplateCatalog::bundledCount() const {
    int count = 0;
    for (const auto &app : m_apps) {
        if (!app.isUser) count++;
    }
    return count;
}

int CommunityTemplateCatalog::userCount() const {
    int count = 0;
    for (const auto &app : m_apps) {
        if (app.isUser) count++;
    }
    return count;
}

void CommunityTemplateCatalog::reload() {
    m_apps.clear();
    m_builtinApps.clear();
    m_communityApps.clear();
    m_idToIndex.clear();

    QStringList searchDirs = templateSearchPaths();

    // Load templates from standard template directories
    for (const QString &dirPath : searchDirs) {
        if (QDir(dirPath).exists()) {
            loadFromDirectory(dirPath);
        }
    }

    Logger::log(QString("CommunityTemplateCatalog: Loaded %1 template apps (%2 bundled, %3 user).")
                .arg(m_apps.size())
                .arg(bundledCount())
                .arg(userCount()),
                Logger::Info);
}

void CommunityTemplateCatalog::loadFromDirectory(const QString &dirPath) {
    QDir dir(dirPath);
    QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    QString userTemplatesDir = QDir::cleanPath(QDir::homePath() + "/.local/share/plasma-theme-master/templates");
    bool isUserDir = (QDir::cleanPath(dirPath) == userTemplatesDir);

    for (const QString &subName : subDirs) {
        // Skip if already loaded from higher priority path
        if (m_idToIndex.contains(subName)) continue;

        QString appDir = dir.filePath(subName);
        QString tomlPath = appDir + "/template.toml";
        if (!QFile::exists(tomlPath)) continue;

        QFile tomlFile(tomlPath);
        if (!tomlFile.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        QTextStream in(&tomlFile);
        std::string tomlContent = in.readAll().toStdString();
        tomlFile.close();

        auto parseResult = toml::parse(tomlContent);
        if (!parseResult) continue;

        const toml::table &tbl = parseResult.table();

        CommunityTemplateApp app;
        app.id = subName;
        app.templateDir = appDir;
        app.readmePath = appDir + "/README.md";
        app.isUser = isUserDir;
        app.isBuiltin = !isUserDir;

        // 1. Catalog section: [catalog.<id>]
        if (const auto *catTbl = tbl.get_as<toml::table>("catalog")) {
            for (const auto &[k, v] : *catTbl) {
                if (const auto *info = v.as_table()) {
                    if (const auto *nameNode = info->get_as<std::string>("name")) {
                        app.name = QString::fromStdString(nameNode->get());
                    }
                    if (const auto *catNode = info->get_as<std::string>("category")) {
                        app.category = QString::fromStdString(catNode->get());
                    }
                    if (const auto *docNode = info->get_as<std::string>("documentation_url")) {
                        app.documentationUrl = QString::fromStdString(docNode->get());
                    }
                }
            }
        }
        if (app.name.isEmpty()) app.name = subName;
        if (app.category.isEmpty()) app.category = "misc";

        // Ignore compositor templates (KDE Plasma & KWin only)
        if (app.category.toLower().trimmed() == "compositor") continue;

        // 2. Readme description extraction
        if (QFile::exists(app.readmePath)) {
            QFile rFile(app.readmePath);
            if (rFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream rIn(&rFile);
                while (!rIn.atEnd()) {
                    QString line = rIn.readLine().trimmed();
                    if (line.isEmpty() || line.startsWith('#') || line.startsWith('<') || line.startsWith('!') || line.startsWith('[')) {
                        continue;
                    }
                    app.description = line;
                    break;
                }
            }
        }
        if (app.description.isEmpty()) {
            app.description = QString("%1 theme template (%2)").arg(app.name, app.resolvedCategoryName());
        }

        // 3. Templates section: [templates.<entry_id>]
        if (const auto *tmplTbl = tbl.get_as<toml::table>("templates")) {
            for (const auto &[k, v] : *tmplTbl) {
                if (const auto *entryTbl = v.as_table()) {
                    CommunityTemplateEntry entry;
                    entry.name = QString::fromUtf8(k.str().data(), k.str().size());

                    if (const auto *inPath = entryTbl->get_as<std::string>("input_path")) {
                        entry.inputPath = QString::fromStdString(inPath->get());
                    }

                    if (const auto *modes = entryTbl->get_as<toml::table>("input_path_modes")) {
                        if (const auto *d = modes->get_as<std::string>("dark")) {
                            entry.inputPathModes["dark"] = QString::fromStdString(d->get());
                        }
                        if (const auto *l = modes->get_as<std::string>("light")) {
                            entry.inputPathModes["light"] = QString::fromStdString(l->get());
                        }
                    }

                    // output_path can be string or array of strings
                    if (const auto *outPathStr = entryTbl->get_as<std::string>("output_path")) {
                        entry.outputPaths << expandPath(QString::fromStdString(outPathStr->get()));
                    } else if (const auto *outArr = entryTbl->get_as<toml::array>("output_path")) {
                        for (const auto &item : *outArr) {
                            if (const auto *s = item.as_string()) {
                                entry.outputPaths << expandPath(QString::fromStdString(s->get()));
                            }
                        }
                    }

                    if (const auto *dynOut = entryTbl->get_as<std::string>("output_path_dynamic")) {
                        entry.outputPathDynamic = QString::fromStdString(dynOut->get());
                    }

                    if (const auto *post = entryTbl->get_as<std::string>("post_hook")) {
                        entry.postHook = QString::fromStdString(post->get());
                    }
                    if (const auto *pre = entryTbl->get_as<std::string>("pre_hook")) {
                        entry.preHook = QString::fromStdString(pre->get());
                    }
                    if (const auto *undo = entryTbl->get_as<std::string>("undo_hook")) {
                        entry.undoHook = QString::fromStdString(undo->get());
                    }
                    if (const auto *req = entryTbl->get_as<std::string>("requires_path")) {
                        entry.requiresPath = QString::fromStdString(req->get());
                    }
                    if (const auto *asyncNode = entryTbl->get_as<bool>("hook_async")) {
                        entry.hookAsync = asyncNode->get();
                    }

                    app.entries.append(entry);
                }
            }
        }

        m_idToIndex[app.id] = m_apps.size();
        m_apps.append(app);
        if (!app.isUser) {
            m_builtinApps.append(app);
        }
        m_communityApps.append(app);
    }
}

const CommunityTemplateApp *CommunityTemplateCatalog::findApp(const QString &id) const {
    if (m_idToIndex.contains(id)) {
        return &m_apps[m_idToIndex[id]];
    }
    return nullptr;
}
