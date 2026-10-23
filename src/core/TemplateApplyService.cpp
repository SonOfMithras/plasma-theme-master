#include "TemplateApplyService.h"
#include "TemplateConfig.h"
#include "TemplateEngine.h"
#include "Logger.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QThreadPool>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>
#include <atomic>

TemplateApplyService &TemplateApplyService::instance() {
    static TemplateApplyService inst;
    return inst;
}

TemplateApplyService::TemplateApplyService(QObject *parent)
    : QObject(parent), m_hookRunner(new HookRunner(4, this)) {
    connect(m_hookRunner, &HookRunner::allHooksFinished,
            this, &TemplateApplyService::onAllHooksFinished);
}

TemplateApplyService::~TemplateApplyService() = default;

bool TemplateApplyService::isBusy() const {
    return m_isApplying || !m_hookRunner->isIdle();
}

quint64 TemplateApplyService::applyAsync(const QString &paletteMode) {
    quint64 gen = m_generation.fetchAndAddRelaxed(1) + 1;
    m_activeGeneration = gen;
    m_hookRunner->invalidateBefore(gen);

    Logger::log(QString("TemplateApplyService: Queueing async sync (generation %1)...").arg(gen),
                Logger::Info);

    emit syncStarted(gen);

    QThreadPool::globalInstance()->start([this, gen, paletteMode]() {
        executeApply(gen, paletteMode);
    });

    return gen;
}

bool TemplateApplyService::applySync(const QString &paletteMode, int timeoutMs) {
    quint64 gen = m_generation.fetchAndAddRelaxed(1) + 1;
    m_activeGeneration = gen;
    m_hookRunner->invalidateBefore(gen);

    Logger::log(QString("TemplateApplyService: Executing sync (generation %1)...").arg(gen),
                Logger::Info);

    emit syncStarted(gen);
    executeApply(gen, paletteMode);

    // Wait for hooks to complete
    QElapsedTimer timer;
    timer.start();
    while (!m_hookRunner->isIdle() && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }

    return m_hookRunner->isIdle();
}

QStringList TemplateApplyService::resolveObsidianOutputPaths(const CommunityTemplateApp &app,
                                                            const CommunityTemplateEntry &entry) {
    QStringList paths;

    // 1. Configured user vaults in config.toml
    QStringList configuredVaults = TemplateConfig::getObsidianVaults();
    for (const QString &vaultDir : configuredVaults) {
        if (QDir(vaultDir).exists()) {
            QString snippetsDir = vaultDir + "/.obsidian/snippets";
            QDir().mkpath(snippetsDir);
            paths << snippetsDir + "/ptm.css";
        }
    }

    // 2. Auto-detect vaults if enabled
    if (TemplateConfig::isObsidianAutoDetectEnabled() && !entry.outputPathDynamic.isEmpty()) {
        QStringList detected = CommunityTemplateCatalog::resolveDynamicOutputPaths(
            entry.outputPathDynamic, app.templateDir);
        for (const QString &p : detected) {
            if (!paths.contains(p)) paths << p;
        }
    }

    return paths;
}

void TemplateApplyService::executeApply(quint64 generation, const QString &paletteMode) {
    m_isApplying = true;
    m_warnings.clear();

    if (generation < m_generation.loadRelaxed()) {
        m_isApplying = false;
        return; // Superseded by newer request
    }

    TemplateConfig::ensureUserConfig();

    // 1. Extract Palette
    UniversalPalette palette = (paletteMode == "day" || paletteMode == "night")
                                   ? UniversalThemeExporter::paletteForEntry(paletteMode)
                                   : UniversalThemeExporter::extractColors();

    // 2. Inject environment variables for helper processes
    auto vars = TemplateEngine::buildVars(palette);
    for (auto it = vars.cbegin(); it != vars.cend(); ++it) {
        QString envKey = "PTM_" + it.key();
        qputenv(envKey.toLocal8Bit(), it.value().toLocal8Bit());
    }

    // 3. Load catalog & entries
    CommunityTemplateCatalog &catalog = CommunityTemplateCatalog::instance();
    const QList<CommunityTemplateApp> &commApps = catalog.apps();

    // Load user-defined custom templates from config.toml (excluding catalog apps and legacy aliases)
    static const QSet<QString> legacyKnown = {
        "kitty", "kitty_dark", "kitty_light", "btop", "vicinae", "firefox",
        "obsidian", "betterdiscord", "vencord", "zed", "vscode", "millennium",
        "steam", "konsole", "zen_browser_chrome", "zen_browser_content",
        "zen-browser", "palette", "discord", "gtk", "gtk3", "gtk4"
    };

    QList<TemplateEntry> customEntries;
    for (const TemplateEntry &entry : TemplateConfig::loadTemplates()) {
        if (!entry.enabled) continue;
        if (legacyKnown.contains(entry.name.toLower())) continue;
        if (catalog.findApp(entry.name)) continue;
        customEntries.append(entry);
    }

    int totalItems = customEntries.size();
    for (const auto &app : commApps) {
        bool enabled = TemplateConfig::isAppEnabled(app.id);
        if (enabled) totalItems += app.entries.size();
    }

    int progress = 0;

    // Step 2: Live GTK4 Reload Hook (runs prior to other application templates)
    // Rate limit GTK reload so rapid sync requests or watchers don't repeatedly toggle GNOME interface
    static std::atomic<qint64> s_lastGtkReloadMs{0};
    qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    qint64 lastMs = s_lastGtkReloadMs.load(std::memory_order_relaxed);
    if (nowMs - lastMs > 3500 && s_lastGtkReloadMs.compare_exchange_strong(lastMs, nowMs)) {
        QString gtkReload = QStandardPaths::findExecutable("plasma-theme-master-helper-gtk4-reload");
        if (gtkReload.isEmpty()) {
            QString localHelper = QCoreApplication::applicationDirPath() + "/helpers/plasma-theme-master-helper-gtk4-reload";
            if (QFile::exists(localHelper)) {
                gtkReload = localHelper;
            } else {
                QString srcHelper = QCoreApplication::applicationDirPath() + "/../src/helpers/gtk4-reload.sh";
                if (QFile::exists(srcHelper)) {
                    gtkReload = srcHelper;
                }
            }
        }
        if (!gtkReload.isEmpty()) {
            m_hookRunner->enqueue(gtkReload, generation, QString(), "GTK4 Reload");
        }
    }

    struct PendingPostHook {
        QString command;
        QString workingDir;
        QString label;
    };
    QList<PendingPostHook> pendingPostHooks;
    QSet<QString> enqueuedCmds;

    // 4. Apply User-Defined Custom Templates (from config.toml [templates.*])
    for (const TemplateEntry &entry : customEntries) {
        if (generation < m_generation.loadRelaxed()) {
            m_isApplying = false;
            return;
        }

        progress++;
        emit syncProgress(generation, progress, totalItems, entry.name);

        UniversalPalette entryPalette = palette;
        if (!entry.palette.isEmpty() && entry.palette != "current") {
            entryPalette = UniversalThemeExporter::paletteForEntry(entry.palette);
        }

        QString inputPath = entry.inputPath;
        if (!inputPath.isEmpty() && QFile::exists(inputPath)) {
            QString rendered = TemplateEngine::render(inputPath, entryPalette, entry.extra);
            if (!rendered.isEmpty() && !entry.outputPath.isEmpty()) {
                QFileInfo fi(entry.outputPath);
                QDir().mkpath(fi.absolutePath());
                UniversalThemeExporter::backupFile(entry.outputPath);

                QSaveFile sf(entry.outputPath);
                if (sf.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream(&sf) << rendered;
                    sf.commit();
                }
            }
        }

        // Collect user-configured post_hook if present
        if (!entry.postHook.isEmpty() && !enqueuedCmds.contains(entry.postHook)) {
            enqueuedCmds.insert(entry.postHook);
            pendingPostHooks.append({entry.postHook, QString(), entry.name});
        }
    }

    // 5. Apply Built-in & Community Templates
    for (const CommunityTemplateApp &app : commApps) {
        bool enabled = TemplateConfig::isAppEnabled(app.id);
        if (!enabled) continue;
        if (generation < m_generation.loadRelaxed()) {
            m_isApplying = false;
            return;
        }

        for (const CommunityTemplateEntry &entry : app.entries) {
            progress++;
            emit syncProgress(generation, progress, totalItems, app.name);

            // Check requires_path
            if (!entry.requiresPath.isEmpty()) {
                QString req = CommunityTemplateCatalog::expandPath(entry.requiresPath);
                if (!QFile::exists(req) && !QDir(req).exists()) {
                    continue; // App requirement not satisfied on this system
                }
            }

            // Determine output paths
            QStringList targets;
            if (app.id == "obsidian") {
                targets = resolveObsidianOutputPaths(app, entry);
            } else if (!entry.outputPathDynamic.isEmpty()) {
                targets = CommunityTemplateCatalog::resolveDynamicOutputPaths(
                    entry.outputPathDynamic, app.templateDir);
            } else {
                targets = entry.outputPaths;
            }

            if (targets.isEmpty() && entry.postHook.isEmpty()) {
                continue;
            }

            // Resolve input file path
            QString inputFilename = entry.inputPath;
            if (!entry.inputPathModes.isEmpty()) {
                bool isDark = (palette.windowBg.lightness() < 128);
                inputFilename = isDark ? entry.inputPathModes.value("dark") : entry.inputPathModes.value("light");
            }

            QString absInput = inputFilename;
            if (!absInput.isEmpty() && !QFileInfo(absInput).isAbsolute()) {
                absInput = app.templateDir + "/" + inputFilename;
            }

            // Fallback resolution if template file extension differs (e.g. foot vs foot.ini, ghostty vs ghostty.conf)
            if (!absInput.isEmpty() && !QFile::exists(absInput)) {
                if (absInput.endsWith(".ini") && QFile::exists(absInput.left(absInput.length() - 4))) {
                    absInput = absInput.left(absInput.length() - 4);
                } else if (absInput.endsWith(".conf") && QFile::exists(absInput.left(absInput.length() - 5))) {
                    absInput = absInput.left(absInput.length() - 5);
                } else if (absInput.endsWith(".toml") && QFile::exists(absInput.left(absInput.length() - 5))) {
                    absInput = absInput.left(absInput.length() - 5);
                } else if (QFile::exists(absInput + ".ini")) {
                    absInput += ".ini";
                } else if (QFile::exists(absInput + ".conf")) {
                    absInput += ".conf";
                } else if (QFile::exists(absInput + ".toml")) {
                    absInput += ".toml";
                }
            }

            // Render template
            QString rendered;
            if (!absInput.isEmpty()) {
                if (QFile::exists(absInput)) {
                    QMap<QString, QString> extra;
                    extra["config_dir"] = app.templateDir;
                    rendered = TemplateEngine::render(absInput, palette, extra);
                } else {
                    Logger::log(QString("TemplateApplyService: Template input file not found for app %1: %2").arg(app.name, absInput), Logger::Warning);
                    m_warnings << QString("Template input not found for %1 (%2)").arg(app.name, absInput);
                }
            }

            // Write output targets
            if (!rendered.isEmpty()) {
                for (const QString &targetPath : targets) {
                    QString outExpanded = CommunityTemplateCatalog::expandPath(targetPath);
                    QFileInfo fi(outExpanded);
                    QDir().mkpath(fi.absolutePath());

                    UniversalThemeExporter::backupFile(outExpanded);

                    QSaveFile sf(outExpanded);
                    if (sf.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        QTextStream(&sf) << rendered;
                        sf.commit();
                    }
                }
            } else if (!targets.isEmpty() && !inputFilename.isEmpty()) {
                // If targets were expected and rendering produced nothing or file was missing,
                // do not run post_hook to avoid leaving the app in an invalid state.
                Logger::log(QString("TemplateApplyService: Skipping post_hook for %1 because template output was not rendered.").arg(app.name), Logger::Warning);
                continue;
            }

            // Collect post_hook
            if (!entry.postHook.isEmpty()) {
                QString hookCmd = entry.postHook;
                hookCmd.replace("{{ config_dir }}", app.templateDir);
                hookCmd.replace("{{config_dir}}", app.templateDir);
                bool isDark = (palette.windowBg.lightness() < 128);
                QString mode = isDark ? "dark" : "light";
                hookCmd.replace("{{ mode }}", mode);
                hookCmd.replace("{{mode}}", mode);
                if (!enqueuedCmds.contains(hookCmd)) {
                    enqueuedCmds.insert(hookCmd);
                    pendingPostHooks.append({hookCmd, app.templateDir, app.name});
                }
            }
        }
    }

    // 6. Dispatch deduplicated post-hooks in batch
    for (const auto &task : pendingPostHooks) {
        m_hookRunner->enqueue(task.command, generation, task.workingDir, task.label);
    }

    m_isApplying = false;

    if (m_hookRunner->isIdle()) {
        emit syncFinished(generation, true, m_warnings);
    }
}

void TemplateApplyService::onAllHooksFinished(quint64 generation) {
    if (generation == m_activeGeneration && !m_isApplying) {
        Logger::log(QString("TemplateApplyService: All hooks finished for generation %1.").arg(generation),
                    Logger::Info);
        emit syncFinished(generation, true, m_warnings);
    }
}
