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
    if (QFile::exists(userPath)) return;

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
    auto *v = doc.get_as<toml::table>("templates");
    if (!v) return defaultVal;
    auto *section = v->get_as<toml::table>(name.toStdString());
    if (!section) return defaultVal;
    auto *node = section->get(key.toStdString());
    if (!node) return defaultVal;
    if (node->is_string())
        return QString::fromStdString(std::string(node->as_string()->get()));
    if (node->is_boolean())
        return node->as_boolean()->get() ? "true" : "false";
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

    auto *templates = doc.get_as<toml::table>("templates");
    if (!templates) return;

    if (!templates->contains(name.toStdString()))
        templates->insert(name.toStdString(), toml::table{});

    auto *section = templates->get_as<toml::table>(name.toStdString());
    if (!section) return;

    section->insert_or_assign("enabled", enabled);
    saveDoc(doc);
}

void TemplateConfig::setValue(const QString &name, const QString &key,
                               const QString &value) {
    auto result = loadDoc();
    if (!result) return;
    toml::table doc = result.table();

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
