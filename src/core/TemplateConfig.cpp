#include "TemplateConfig.h"
#include "Logger.h"

// Disable TOML++ exceptions, use error_info instead
#define TOML_EXCEPTIONS 0
#include "third_party/toml.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>

static bool saveDoc(const toml::table &tbl);

// ---------------------------------------------------------------------------
// Path helpers
// ---------------------------------------------------------------------------

QString TemplateConfig::expandPath(const QString &path) {
    if (path.startsWith("~/")) {
        return QDir::homePath() + path.mid(1);
    }
    return path;
}

QString TemplateConfig::userConfigPath() {
    return QDir::homePath() +
           "/.config/plasma-theme-master/config.toml";
}

QString TemplateConfig::systemConfigPath() {
    // Try standard data locations first, fallback to adjacent-to-binary path
    QString path = QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        "plasma-theme-master/config.toml.default");
    if (!path.isEmpty()) return path;
    return "/usr/share/plasma-theme-master/config.toml.default";
}

void TemplateConfig::ensureUserConfig() {
    QString userPath = userConfigPath();
    if (QFile::exists(userPath)) {
        mergeMissingTemplates();
        return;
    }

    QFileInfo fi(userPath);
    QDir().mkpath(fi.absolutePath());

    QString sysPath = systemConfigPath();
    if (QFile::exists(sysPath)) {
        if (QFile::copy(sysPath, userPath)) {
            Logger::log("TemplateConfig: Installed default config to " + userPath,
                        Logger::Info);
            return;
        }
    }

    // Neither system path exists (dev environment) — write a minimal stub
    QFile f(userPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        out << "# Plasma Theme Master - config.toml\n"
            << "# Add [templates.NAME] entries here.\n\n"
            << "[config]\n";
        Logger::log("TemplateConfig: Created minimal stub config at " + userPath,
                    Logger::Info);
    }
}

void TemplateConfig::mergeMissingTemplates() {
    QString userPath = userConfigPath();
    QString sysPath = systemConfigPath();
    if (!QFile::exists(userPath) || !QFile::exists(sysPath)) return;

    // Load default config
    QFile sysFile(sysPath);
    if (!sysFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream sysIn(&sysFile);
    std::string sysContent = sysIn.readAll().toStdString();
    sysFile.close();
    auto sysResult = toml::parse(sysContent);
    if (!sysResult) return;

    // Load user config
    QFile userFile(userPath);
    if (!userFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream userIn(&userFile);
    std::string userContent = userIn.readAll().toStdString();
    userFile.close();
    auto userResult = toml::parse(userContent);
    if (!userResult) return;

    toml::table sysDoc = std::move(sysResult).table();
    toml::table userDoc = std::move(userResult).table();

    auto *sysTemplates = sysDoc.get_as<toml::table>("templates");
    if (!sysTemplates) return;

    if (!userDoc.contains("templates"))
        userDoc.insert("templates", toml::table{});

    auto *userTemplates = userDoc.get_as<toml::table>("templates");
    if (!userTemplates) return;

    bool modified = false;
    for (const auto &[key, val] : *sysTemplates) {
        if (!val.is_table()) continue;
        std::string sKey(key);
        if (!userTemplates->contains(sKey)) {
            userTemplates->insert(sKey, val);
            modified = true;
            Logger::log("TemplateConfig: Merged missing template section: " +
                        QString::fromStdString(sKey), Logger::Info);
        }
    }

    if (modified) {
        saveDoc(userDoc);
    }
}

// ---------------------------------------------------------------------------
// Internal: load the toml document (returns empty table on error)
// ---------------------------------------------------------------------------
static toml::parse_result loadDoc() {
    QString path = TemplateConfig::userConfigPath();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::log("TemplateConfig: Cannot open " + path, Logger::Warning);
        return toml::parse("");
    }
    QTextStream in(&f);
    std::string content = in.readAll().toStdString();
    f.close();
    auto result = toml::parse(content);
    if (!result) {
        Logger::log("TemplateConfig: TOML parse error in " + path + ": " +
                        QString::fromStdString(std::string(result.error().description())),
                    Logger::Error);
    }
    return result;
}

// Internal: save a toml::table back to disk
static bool saveDoc(const toml::table &tbl) {
    QString path = TemplateConfig::userConfigPath();
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        Logger::log("TemplateConfig: Cannot write " + path, Logger::Error);
        return false;
    }
    std::ostringstream oss;
    oss << tbl;
    QTextStream out(&f);
    out << QString::fromStdString(oss.str());
    return true;
}

// ---------------------------------------------------------------------------
// Reading
// ---------------------------------------------------------------------------

QList<TemplateEntry> TemplateConfig::loadTemplates() {
    QList<TemplateEntry> entries;
    auto result = loadDoc();
    if (!result) return entries;
    const toml::table &doc = result.table();

    auto *templatesNode = doc.get("templates");
    if (!templatesNode || !templatesNode->is_table()) return entries;

    const toml::table &templates = *templatesNode->as_table();
    for (const auto &[key, val] : templates) {
        if (!val.is_table()) continue;
        if (key == "palette") continue; // Configuration section, not a template
        const toml::table &section = *val.as_table();

        TemplateEntry entry;
        entry.name        = QString::fromStdString(std::string(key));
        entry.enabled     = section["enabled"].value_or(false);
        entry.inputPath   = expandPath(
            QString::fromStdString(section["input_path"].value_or(std::string(""))));
        entry.outputPath  = expandPath(
            QString::fromStdString(section["output_path"].value_or(std::string(""))));
        entry.palette     = QString::fromStdString(
            section["palette"].value_or(std::string("current")));
        entry.postHook    = QString::fromStdString(
            section["post_hook"].value_or(std::string("")));

        // Collect any extra keys not part of the standard set
        static const QSet<QString> standardKeys = {
            "enabled", "input_path", "output_path", "palette", "post_hook"
        };
        for (const auto &[k, v] : section) {
            QString qk = QString::fromStdString(std::string(k));
            if (standardKeys.contains(qk)) continue;
            if (v.is_string()) {
                entry.extra[qk] = QString::fromStdString(
                    std::string(v.as_string()->get()));
            } else if (v.is_boolean()) {
                entry.extra[qk] = v.as_boolean()->get() ? "true" : "false";
            } else if (v.is_integer()) {
                entry.extra[qk] = QString::number(v.as_integer()->get());
            }
        }

        entries.append(entry);
    }
    return entries;
}

bool TemplateConfig::isEnabled(const QString &name) {
    auto result = loadDoc();
    if (!result) return false;
    const toml::table &doc = result.table();
    auto *v = doc.get_as<toml::table>("templates");
    if (!v) return false;
    auto *section = v->get_as<toml::table>(name.toStdString());
    if (!section) return false;
    return section->get("enabled")->value_or(false);
}

QString TemplateConfig::getValue(const QString &name, const QString &key,
                                  const QString &defaultVal) {
    auto result = loadDoc();
    if (!result) return defaultVal;
    const toml::table &doc = result.table();

    // 1. Check top-level [name] (e.g. [palette].current)
    if (const auto *top = doc.get_as<toml::table>(name.toStdString())) {
        if (const auto *node = top->get(key.toStdString())) {
            if (node->is_string())
                return QString::fromStdString(std::string(node->as_string()->get()));
            if (node->is_boolean())
                return node->as_boolean()->get() ? "true" : "false";
        }
    }

    // 2. Check [templates.name] (e.g. [templates.palette].current or [templates.myapp].key)
    if (const auto *v = doc.get_as<toml::table>("templates")) {
        if (const auto *section = v->get_as<toml::table>(name.toStdString())) {
            if (const auto *node = section->get(key.toStdString())) {
                if (node->is_string())
                    return QString::fromStdString(std::string(node->as_string()->get()));
                if (node->is_boolean())
                    return node->as_boolean()->get() ? "true" : "false";
            }
        }
    }

    return defaultVal;
}

QStringList TemplateConfig::getList(const QString &name, const QString &key) {
    QStringList out;
    auto result = loadDoc();
    if (!result) return out;
    const toml::table &doc = result.table();

    // Support both [templates.NAME] and [discord.NAME] style sections
    auto findSection = [&](const std::string &topKey) -> const toml::table * {
        auto *top = doc.get_as<toml::table>(topKey);
        if (!top) return nullptr;
        return top->get_as<toml::table>(name.toStdString());
    };

    const toml::table *section = findSection("templates");
    if (!section) section = findSection("discord");
    if (!section) return out;

    auto *node = section->get(key.toStdString());
    if (!node || !node->is_array()) return out;
    for (const auto &item : *node->as_array()) {
        if (item.is_string())
            out << QString::fromStdString(std::string(item.as_string()->get()));
    }
    return out;
}

// ---------------------------------------------------------------------------
// Writing (simple regex-free approach: reload → mutate → save)
// ---------------------------------------------------------------------------

void TemplateConfig::setEnabled(const QString &name, bool enabled) {
    auto result = loadDoc();
    if (!result) return;
    toml::table doc = result.table();  // mutable copy

    if (!doc.contains("templates"))
        doc.insert("templates", toml::table{});

    if (auto *templates = doc.get_as<toml::table>("templates")) {
        if (!templates->contains(name.toStdString()))
            templates->insert(name.toStdString(), toml::table{});
        if (auto *section = templates->get_as<toml::table>(name.toStdString())) {
            section->insert_or_assign("enabled", enabled);
        }
    }

    if (!doc.contains("app_templates"))
        doc.insert("app_templates", toml::table{});

    if (auto *apps = doc.get_as<toml::table>("app_templates")) {
        if (!apps->contains(name.toStdString()))
            apps->insert(name.toStdString(), toml::table{});
        if (auto *section = apps->get_as<toml::table>(name.toStdString())) {
            section->insert_or_assign("enabled", enabled);
        }
    }

    saveDoc(doc);
}

void TemplateConfig::setValue(const QString &name, const QString &key,
                               const QString &value) {
    auto result = loadDoc();
    if (!result) return;
    toml::table doc = result.table();

    // For palette, maintain both [palette].current and [templates.palette].current
    if (name == "palette") {
        if (!doc.contains("palette"))
            doc.insert("palette", toml::table{});
        if (auto *pTbl = doc.get_as<toml::table>("palette")) {
            pTbl->insert_or_assign(key.toStdString(), value.toStdString());
        }
    }

    if (!doc.contains("templates"))
        doc.insert("templates", toml::table{});

    auto *templates = doc.get_as<toml::table>("templates");
    if (!templates) return;

    if (!templates->contains(name.toStdString()))
        templates->insert(name.toStdString(), toml::table{});

    auto *section = templates->get_as<toml::table>(name.toStdString());
    if (!section) return;

    section->insert_or_assign(key.toStdString(), value.toStdString());
    saveDoc(doc);
}

void TemplateConfig::setList(const QString &name, const QString &key,
                              const QStringList &values) {
    auto result = loadDoc();
    if (!result) return;
    toml::table doc = result.table();

    // Support [templates.NAME] or [discord.NAME]
    auto ensureSection = [&](const std::string &topKey) -> toml::table * {
        if (!doc.contains(topKey))
            doc.insert(topKey, toml::table{});
        auto *top = doc.get_as<toml::table>(topKey);
        if (!top) return nullptr;
        if (!top->contains(name.toStdString()))
            top->insert(name.toStdString(), toml::table{});
        return top->get_as<toml::table>(name.toStdString());
    };

    // Try to find which top-level section the name lives under
    toml::table *section = nullptr;
    if (doc.get_as<toml::table>("discord") &&
        doc.get_as<toml::table>("discord")->contains(name.toStdString())) {
        section = ensureSection("discord");
    } else {
        section = ensureSection("templates");
    }
    if (!section) return;

    toml::array arr;
    for (const QString &v : values) arr.push_back(v.toStdString());
    section->insert_or_assign(key.toStdString(), std::move(arr));
    saveDoc(doc);
}

// ---------------------------------------------------------------------------
// Community Templates & Extended App Config
// ---------------------------------------------------------------------------
// Built-in Templates Configuration
// ---------------------------------------------------------------------------

bool TemplateConfig::isBuiltinTemplatesEnabled() {
    auto result = loadDoc();
    if (!result) return true;
    const toml::table &doc = result.table();

    if (const auto *tmpls = doc.get_as<toml::table>("templates")) {
        if (const auto *val = tmpls->get("enable_builtins")) {
            if (val->is_boolean()) return val->as_boolean()->get();
        }
    }
    return true; // Enabled by default
}

void TemplateConfig::setBuiltinTemplatesEnabled(bool enabled) {
    auto result = loadDoc();
    if (!result) return;
    toml::table doc = result.table();

    if (!doc.contains("templates")) {
        doc.insert("templates", toml::table{});
    }

    auto *tmpls = doc.get_as<toml::table>("templates");
    if (tmpls) {
        tmpls->insert_or_assign("enable_builtins", enabled);
        saveDoc(doc);
    }
}

bool TemplateConfig::isAppEnabled(const QString &appId) {
    auto result = loadDoc();
    if (!result) return false;
    const toml::table &doc = result.table();

    // 1. Check [app_templates.<appId>].enabled
    if (const auto *apps = doc.get_as<toml::table>("app_templates")) {
        if (const auto *section = apps->get_as<toml::table>(appId.toStdString())) {
            if (const auto *val = section->get("enabled")) {
                if (val->is_boolean()) return val->as_boolean()->get();
            }
        }
    }

    // 2. Check [community_templates.<appId>].enabled
    if (const auto *comm = doc.get_as<toml::table>("community_templates")) {
        if (const auto *section = comm->get_as<toml::table>(appId.toStdString())) {
            if (const auto *val = section->get("enabled")) {
                if (val->is_boolean()) return val->as_boolean()->get();
            }
        }
    }

    // 3. Check [builtin_templates.<appId>].enabled
    if (const auto *builtins = doc.get_as<toml::table>("builtin_templates")) {
        if (const auto *section = builtins->get_as<toml::table>(appId.toStdString())) {
            if (const auto *val = section->get("enabled")) {
                if (val->is_boolean()) return val->as_boolean()->get();
            }
        }
    }

    // 4. Check [templates.<appId>].enabled
    if (const auto *tmpls = doc.get_as<toml::table>("templates")) {
        if (const auto *section = tmpls->get_as<toml::table>(appId.toStdString())) {
            if (const auto *val = section->get("enabled")) {
                if (val->is_boolean()) return val->as_boolean()->get();
            }
        }
    }

    // Special aliases
    if (appId == "discord") {
        return isEnabled("betterdiscord") || isEnabled("vencord");
    }
    if (appId == "kitty") {
        return isEnabled("kitty") || isEnabled("kitty_dark") || isEnabled("kitty_light");
    }
    if (appId == "gtk") {
        return isEnabled("gtk3") || isEnabled("gtk4");
    }

    return false;
}

void TemplateConfig::setAppEnabled(const QString &appId, bool enabled) {
    auto result = loadDoc();
    if (!result) return;
    toml::table doc = result.table();

    // 1. app_templates
    if (!doc.contains("app_templates")) {
        doc.insert("app_templates", toml::table{});
    }
    if (auto *apps = doc.get_as<toml::table>("app_templates")) {
        if (!apps->contains(appId.toStdString())) {
            apps->insert(appId.toStdString(), toml::table{});
        }
        if (auto *section = apps->get_as<toml::table>(appId.toStdString())) {
            section->insert_or_assign("enabled", enabled);
        }
        if (appId == "zen-browser") {
            apps->erase("zen_browser_chrome");
            apps->erase("zen_browser_content");
        }
    }

    // 2. If user already has an existing [templates.<appId>] entry, sync its enabled flag
    if (auto *templates = doc.get_as<toml::table>("templates")) {
        if (templates->contains(appId.toStdString())) {
            if (auto *section = templates->get_as<toml::table>(appId.toStdString())) {
                section->insert_or_assign("enabled", enabled);
            }
        }
        if (appId == "zen-browser") {
            templates->erase("zen_browser_chrome");
            templates->erase("zen_browser_content");
        }
    }

    // 3. Mirror to community_templates and builtin_templates if present
    if (doc.contains("community_templates")) {
        if (auto *comm = doc.get_as<toml::table>("community_templates")) {
            if (comm->contains(appId.toStdString())) {
                if (auto *s = comm->get_as<toml::table>(appId.toStdString())) {
                    s->insert_or_assign("enabled", enabled);
                }
            }
        }
    }
    if (doc.contains("builtin_templates")) {
        if (auto *b = doc.get_as<toml::table>("builtin_templates")) {
            if (b->contains(appId.toStdString())) {
                if (auto *s = b->get_as<toml::table>(appId.toStdString())) {
                    s->insert_or_assign("enabled", enabled);
                }
            }
        }
    }

    saveDoc(doc);
}

QStringList TemplateConfig::getObsidianVaults() {
    QStringList vaults;
    auto result = loadDoc();
    if (!result) return vaults;
    const toml::table &doc = result.table();

    // Check [obsidian].vaults
    if (const auto *obsTbl = doc.get_as<toml::table>("obsidian")) {
        if (const auto *arrNode = obsTbl->get_as<toml::array>("vaults")) {
            for (const auto &item : *arrNode) {
                if (item.is_string()) {
                    vaults << expandPath(QString::fromStdString(std::string(item.as_string()->get())));
                }
            }
        }
    }

    // Fallback: legacy single path in [obsidian].obsidian_vault_path or [config].obsidian_vault_path
    if (vaults.isEmpty()) {
        QString legacy = getValue("obsidian", "obsidian_vault_path");
        if (legacy.isEmpty()) {
            if (const auto *cfg = doc.get_as<toml::table>("config")) {
                if (const auto *p = cfg->get_as<std::string>("obsidian_vault_path")) {
                    legacy = QString::fromStdString(p->get());
                }
            }
        }
        if (!legacy.isEmpty()) {
            vaults << expandPath(legacy);
        }
    }

    return vaults;
}

void TemplateConfig::setObsidianVaults(const QStringList &vaults) {
    auto result = loadDoc();
    if (!result) return;
    toml::table doc = result.table();

    if (!doc.contains("obsidian")) {
        doc.insert("obsidian", toml::table{});
    }

    auto *obsTbl = doc.get_as<toml::table>("obsidian");
    if (!obsTbl) return;

    toml::array arr;
    for (const QString &v : vaults) {
        arr.push_back(v.toStdString());
    }
    obsTbl->insert_or_assign("vaults", std::move(arr));

    // Also update legacy single path for backwards compatibility
    if (!vaults.isEmpty()) {
        setValue("obsidian", "obsidian_vault_path", vaults.first());
    }

    saveDoc(doc);
}

bool TemplateConfig::isObsidianAutoDetectEnabled() {
    auto result = loadDoc();
    if (!result) return true;
    const toml::table &doc = result.table();

    if (const auto *obsTbl = doc.get_as<toml::table>("obsidian")) {
        if (const auto *val = obsTbl->get("auto_detect")) {
            if (val->is_boolean()) return val->as_boolean()->get();
        }
    }
    return true; // default to auto-detecting
}

void TemplateConfig::setObsidianAutoDetect(bool enabled) {
    auto result = loadDoc();
    if (!result) return;
    toml::table doc = result.table();

    if (!doc.contains("obsidian")) {
        doc.insert("obsidian", toml::table{});
    }

    auto *obsTbl = doc.get_as<toml::table>("obsidian");
    if (!obsTbl) return;

    obsTbl->insert_or_assign("auto_detect", enabled);
    saveDoc(doc);
}
