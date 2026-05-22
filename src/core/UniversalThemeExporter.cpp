#include "UniversalThemeExporter.h"
#include "Config.h"
#include "GlobalThemeManager.h"
#include "Logger.h"
#include "Solar.h"
#include "TemplateConfig.h"
#include "TemplateEngine.h"
#include "ThemeReader.h"
#include "ThemeWriter.h"
#include <KColorScheme>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>

// =============================================================================
// Color Extraction
// =============================================================================

UniversalPalette UniversalThemeExporter::extractColors() {
    auto config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    config->reparseConfiguration();

    if (Config::isMaterialYouOverrideEnabled()) {
        KConfigGroup group = config->group(QStringLiteral("General"));
        QString scheme = group.readEntry(QStringLiteral("ColorScheme"), QString());

        if (scheme != "MaterialYouLight" && scheme != "MaterialYouDark") {
            QString currentGlobal  = ThemeReader::currentGlobalTheme();
            QString currentKvantum = ThemeReader::currentKvantumTheme();

            if (currentGlobal == ThemeReader::defaultDarkTheme() ||
                (!currentKvantum.isEmpty() &&
                 currentKvantum == ThemeReader::nightKvantumTheme())) {
                scheme = "MaterialYouDark";
                Logger::log("Universal Fallback: Deduced Night via structural themes.",
                            Logger::Info);
            } else if (currentGlobal == ThemeReader::defaultLightTheme() ||
                       (!currentKvantum.isEmpty() &&
                        currentKvantum == ThemeReader::dayKvantumTheme())) {
                scheme = "MaterialYouLight";
                Logger::log("Universal Fallback: Deduced Day via structural themes.",
                            Logger::Info);
            } else {
                Logger::log("Universal Fallback: Falling back to Solar Clock.",
                            Logger::Warning);
                double lat = ThemeReader::nativeLatitude();
                double lon = ThemeReader::nativeLongitude();
                int dayOffset = ThemeReader::solarDayOffset();
                int nightOffset = ThemeReader::solarNightOffset();
                scheme = Solar::isDaytime(lat, lon, dayOffset, nightOffset) ? "MaterialYouLight"
                                                                            : "MaterialYouDark";
            }
        }

        QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              "color-schemes/" + scheme + ".colors");
        if (path.isEmpty())
            path = QDir::homePath() + "/.local/share/color-schemes/" + scheme + ".colors";

        if (QFile::exists(path)) {
            Logger::log("UniversalThemeExporter: Sourcing colors from " + path, Logger::Info);
            return extractColors(path);
        }
        Logger::log("UniversalThemeExporter: Failed to find " + scheme + ".colors",
                    Logger::Warning);
    }

    return extractColorsFromConfig(config);
}

UniversalPalette UniversalThemeExporter::extractColors(const QString &configPath) {
    if (configPath.isEmpty() || !QFile::exists(configPath)) {
        Logger::log("Invalid config path for color extraction: " + configPath,
                    Logger::Warning);
        return extractColors();
    }
    auto config = KSharedConfig::openConfig(configPath, KConfig::SimpleConfig);
    config->reparseConfiguration();
    return extractColorsFromConfig(config);
}

UniversalPalette UniversalThemeExporter::extractColorsFromConfig(KSharedConfig::Ptr config) {
    UniversalPalette palette;

    KColorScheme windowScheme(QPalette::Active, KColorScheme::Window, config);
    KColorScheme viewScheme(QPalette::Active, KColorScheme::View, config);
    KColorScheme selectionScheme(QPalette::Active, KColorScheme::Selection, config);
    KColorScheme buttonScheme(QPalette::Active, KColorScheme::Button, config);
    KColorScheme headerScheme(QPalette::Active, KColorScheme::Header, config);

    palette.windowBg = windowScheme.background(KColorScheme::NormalBackground).color();
    palette.windowFg = windowScheme.foreground(KColorScheme::NormalText).color();
    palette.viewBg   = viewScheme.background(KColorScheme::NormalBackground).color();
    palette.viewFg   = viewScheme.foreground(KColorScheme::NormalText).color();
    palette.accent   = selectionScheme.background(KColorScheme::NormalBackground).color();
    palette.selection = palette.accent;
    palette.success  = windowScheme.foreground(KColorScheme::PositiveText).color();
    palette.warning  = windowScheme.foreground(KColorScheme::NeutralText).color();
    palette.error    = windowScheme.foreground(KColorScheme::NegativeText).color();
    palette.titleBarBg = headerScheme.background(KColorScheme::NormalBackground).color();
    palette.titleBarFg = headerScheme.foreground(KColorScheme::NormalText).color();
    palette.buttonBg = buttonScheme.background(KColorScheme::NormalBackground).color();
    palette.buttonFg = buttonScheme.foreground(KColorScheme::NormalText).color();

    palette.ansiBlack   = windowScheme.background(KColorScheme::NormalBackground).color();
    palette.ansiRed     = windowScheme.foreground(KColorScheme::NegativeText).color();
    palette.ansiGreen   = windowScheme.foreground(KColorScheme::PositiveText).color();
    palette.ansiYellow  = windowScheme.foreground(KColorScheme::NeutralText).color();
    palette.ansiBlue    = selectionScheme.background(KColorScheme::NormalBackground).color();
    palette.ansiMagenta = windowScheme.foreground(KColorScheme::ActiveText).color();
    palette.ansiCyan    = windowScheme.foreground(KColorScheme::LinkText).color();
    palette.ansiWhite   = windowScheme.foreground(KColorScheme::NormalText).color();

    palette.ansiBlackBright   = palette.ansiBlack.lighter(120);
    palette.ansiRedBright     = palette.ansiRed.lighter(120);
    palette.ansiGreenBright   = palette.ansiGreen.lighter(120);
    palette.ansiYellowBright  = palette.ansiYellow.lighter(120);
    palette.ansiBlueBright    = palette.ansiBlue.lighter(120);
    palette.ansiMagentaBright = palette.ansiMagenta.lighter(120);
    palette.ansiCyanBright    = palette.ansiCyan.lighter(120);
    palette.ansiWhiteBright   = palette.ansiWhite.lighter(120);

    return palette;
}

// =============================================================================
// Palette selection for day / night / current
// =============================================================================

UniversalPalette UniversalThemeExporter::paletteForEntry(const QString &paletteMode) {
    if (paletteMode == "day") {
        QString schemeName;
        if (Config::isMaterialYouOverrideEnabled()) {
            schemeName = "MaterialYouLight";
        } else {
            schemeName = GlobalThemeManager::getColorSchemeFromGlobal(
                ThemeReader::defaultLightTheme());
        }
        QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              "color-schemes/" + schemeName + ".colors");
        if (path.isEmpty())
            path = QDir::homePath() + "/.local/share/color-schemes/" + schemeName + ".colors";
        if (!path.isEmpty() && QFile::exists(path))
            return extractColors(path);
        // Fallback: try GlobalThemeManager path
        path = GlobalThemeManager::findColorSchemePath(schemeName);
        if (!path.isEmpty()) return extractColors(path);
        Logger::log("paletteForEntry(day): color scheme not found, using current.",
                    Logger::Warning);
    } else if (paletteMode == "night") {
        QString schemeName;
        if (Config::isMaterialYouOverrideEnabled()) {
            schemeName = "MaterialYouDark";
        } else {
            schemeName = GlobalThemeManager::getColorSchemeFromGlobal(
                ThemeReader::defaultDarkTheme());
        }
        QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              "color-schemes/" + schemeName + ".colors");
        if (path.isEmpty())
            path = QDir::homePath() + "/.local/share/color-schemes/" + schemeName + ".colors";
        if (!path.isEmpty() && QFile::exists(path))
            return extractColors(path);
        path = GlobalThemeManager::findColorSchemePath(schemeName);
        if (!path.isEmpty()) return extractColors(path);
        Logger::log("paletteForEntry(night): color scheme not found, using current.",
                    Logger::Warning);
    }
    return extractColors(); // "current"
}

// =============================================================================
// Palette → environment variables (for helper binaries)
// =============================================================================

void UniversalThemeExporter::injectPaletteEnv(const UniversalPalette &p) {
    auto vars = TemplateEngine::buildVars(p);
    for (auto it = vars.cbegin(); it != vars.cend(); ++it) {
        QString envKey = "PTM_" + it.key();
        qputenv(envKey.toLocal8Bit(), it.value().toLocal8Bit());
    }
}

// =============================================================================
// Discord import string builder
// =============================================================================

QString UniversalThemeExporter::buildImports(const QString &appName) {
    // appName is "betterdiscord" or "vencord" — maps to [discord.betterdiscord] etc.
    QString imports;

    bool midnightEnabled =
        (TemplateConfig::getValue(appName, "midnight_enabled", "true") == "true");
    if (midnightEnabled) {
        imports += "@import url(\"https://refact0r.github.io/midnight-discord/build/"
                   "midnight.css\");\n";
    }

    QStringList customImports = TemplateConfig::getList(appName, "custom_imports");
    for (const QString &url : customImports) {
        if (url.trimmed().isEmpty()) continue;
        QString clean = url.trimmed();
        if (!clean.startsWith("@import"))
            imports += QString("@import url(\"%1\");\n").arg(clean);
        else
            imports += clean + "\n";
    }
    return imports;
}

// =============================================================================
// Zed-specific derived color variables
// =============================================================================

QMap<QString, QString> UniversalThemeExporter::buildZedExtras(
    const UniversalPalette &p) {
    auto toHex = [](QColor c) { return c.name(); };
    auto toHexA = [](QColor c, int alpha) -> QString {
        c.setAlpha(alpha);
        return QString("#%1%2%3%4")
            .arg(c.red(), 2, 16, QChar('0'))
            .arg(c.green(), 2, 16, QChar('0'))
            .arg(c.blue(), 2, 16, QChar('0'))
            .arg(c.alpha(), 2, 16, QChar('0'));
    };

    // Derive Catppuccin-style surface levels from the palette
    bool isDark = p.windowBg.lightness() < 128;
    QColor mantle = p.windowBg;
    QColor crust  = isDark ? mantle.darker(115) : mantle.darker(105);
    QColor s0     = isDark ? mantle.lighter(115) : mantle.darker(110);
    QColor s1     = isDark ? mantle.lighter(130) : mantle.darker(120);
    QColor s2     = isDark ? mantle.lighter(145) : mantle.darker(130);

    QColor text     = p.viewFg;
    QColor subtext0 = QColor(
        text.red()   * 0.75 + p.viewBg.red()   * 0.25,
        text.green() * 0.75 + p.viewBg.green() * 0.25,
        text.blue()  * 0.75 + p.viewBg.blue()  * 0.25);
    QColor subtext1 = QColor(
        text.red()   * 0.55 + p.viewBg.red()   * 0.45,
        text.green() * 0.55 + p.viewBg.green() * 0.45,
        text.blue()  * 0.55 + p.viewBg.blue()  * 0.45);
    QColor overlay0 = QColor(
        text.red()   * 0.35 + p.viewBg.red()   * 0.65,
        text.green() * 0.35 + p.viewBg.green() * 0.65,
        text.blue()  * 0.35 + p.viewBg.blue()  * 0.65);

    QMap<QString, QString> e;
    e["ZED_CRUST"]    = toHex(crust);
    e["ZED_SURFACE0"] = toHex(s0);
    e["ZED_SURFACE1"] = toHex(s1);
    e["ZED_SURFACE2"] = toHex(s2);
    e["ZED_SUBTEXT0"] = toHex(subtext0);
    e["ZED_SUBTEXT1"] = toHex(subtext1);
    e["ZED_OVERLAY0"] = toHex(overlay0);
    e["ZED_SURFACE2_80"] = toHexA(s2, 128);
    return e;
}

// =============================================================================
// File utilities
// =============================================================================

bool UniversalThemeExporter::backupFile(const QString &path) {
    QFile file(path);
    if (!file.exists()) return true;
    QString backupPath = path + ".bak";
    if (QFile::exists(backupPath)) return true;
    if (file.copy(backupPath)) {
        Logger::log("Created backup: " + backupPath, Logger::Info);
        return true;
    }
    Logger::log("Failed to create backup: " + backupPath, Logger::Error);
    return false;
}

bool UniversalThemeExporter::restoreFile(const QString &path) {
    QString backupPath = path + ".bak";
    QFile backup(backupPath);
    if (!backup.exists()) {
        Logger::log("No backup found for: " + path, Logger::Warning);
        return false;
    }
    QFile file(path);
    if (file.exists() && !file.remove()) {
        Logger::log("Failed to remove current file during restore: " + path, Logger::Error);
        return false;
    }
    if (backup.copy(path)) {
        Logger::log("Restored from backup: " + backupPath, Logger::Info);
        return true;
    }
    Logger::log("Failed to restore from backup: " + backupPath, Logger::Error);
    return false;
}

// =============================================================================
// Core sync logic
// =============================================================================

void UniversalThemeExporter::syncAll() {
    syncTemplates();
}

void UniversalThemeExporter::syncTemplates() {
    Logger::log("UniversalThemeExporter: Starting template sync...", Logger::Info);

    TemplateConfig::ensureUserConfig();
    QList<TemplateEntry> entries = TemplateConfig::loadTemplates();

    // Built-in post-hooks: the app always knows what each app needs.
    // These run automatically for known template names — users don't configure these.
    // post_hook in config.toml is reserved for user-added custom templates only.
    static const QMap<QString, QString> builtinHooks = {
        {"btop",         "pkill -USR2 btop || true"},
        {"vicinae",      "vicinae theme set plasma-theme-master"},
        {"firefox",      "plasma-theme-master-helper-firefox"},
        {"obsidian",     "plasma-theme-master-helper-obsidian"},
        {"zed",          "plasma-theme-master-helper-zed"},
        {"vscode",       "plasma-theme-master-helper-vscode"},
        {"konsole",      "plasma-theme-master-helper-konsole"},
        {"kitty",        "plasma-theme-master-helper-kitty"},
        {"kitty_dark",   "plasma-theme-master-helper-kitty"},
        {"kitty_light",  "plasma-theme-master-helper-kitty"},
        // betterdiscord, vencord: no post-hook needed
    };

    for (const TemplateEntry &entry : entries) {
        if (!entry.enabled) continue;

        Logger::log("Syncing template: " + entry.name, Logger::Info);

        // --- 1. Resolve palette ---
        UniversalPalette palette = paletteForEntry(entry.palette);

        // --- 2. Build extra variables ---
        QMap<QString, QString> extra = entry.extra;

        // Discord apps: pre-compute {{IMPORTS}}
        if (entry.name == "betterdiscord" || entry.name == "vencord") {
            extra["IMPORTS"] = buildImports(entry.name);
        }
        // Zed: pre-compute derived surface/text colors
        if (entry.name == "zed") {
            auto zedExtras = buildZedExtras(palette);
            for (auto it = zedExtras.cbegin(); it != zedExtras.cend(); ++it)
                extra[it.key()] = it.value();
        }

        // --- 3. Render template → output file (if inputPath is set) ---
        QString renderedTmpPath; // non-empty when we wrote to a temp file
        if (!entry.inputPath.isEmpty()) {
            if (!QFile::exists(entry.inputPath)) {
                Logger::log("Template file not found, skipping: " + entry.inputPath,
                            Logger::Warning);
                continue;
            }

            QString rendered = TemplateEngine::render(entry.inputPath, palette, extra);
            if (rendered.isEmpty()) {
                Logger::log("Template rendered empty, skipping: " + entry.name,
                            Logger::Warning);
                continue;
            }

            if (!entry.outputPath.isEmpty()) {
                QFileInfo fi(entry.outputPath);
                if (!QDir(fi.absolutePath()).exists())
                    QDir().mkpath(fi.absolutePath());

                backupFile(entry.outputPath);

                QSaveFile sf(entry.outputPath);
                if (sf.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream(&sf) << rendered;
                    if (sf.commit()) {
                        Logger::log("Wrote: " + entry.outputPath, Logger::Info);
                    } else {
                        Logger::log("Failed to commit: " + entry.outputPath, Logger::Error);
                        continue;
                    }
                } else {
                    Logger::log("Cannot open for writing: " + entry.outputPath, Logger::Error);
                    continue;
                }
            } else {
                // No output_path: write to a temp file for the helper to consume
                // (e.g. firefox helper reads rendered CSS from a path argument)
                renderedTmpPath = QDir::tempPath() + "/ptm_" + entry.name + ".tmp";
                QFile tmp(renderedTmpPath);
                if (tmp.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream(&tmp) << rendered;
                    tmp.close();
                } else {
                    renderedTmpPath.clear();
                }
            }
        }

        // --- 4. Inject palette into environment for helper binaries ---
        injectPaletteEnv(palette);

        // --- 5. Determine which hook to run ---
        // Priority: built-in hook for known apps > user post_hook for custom apps
        QString hook;
        if (builtinHooks.contains(entry.name)) {
            hook = builtinHooks[entry.name];
        } else if (!entry.postHook.isEmpty()) {
            hook = entry.postHook; // user-defined custom template hook
        }

        if (!hook.isEmpty()) {
            // Append temp file path as argument when helper needs the rendered content
            if (!renderedTmpPath.isEmpty())
                hook += " \"" + renderedTmpPath + "\"";

            Logger::log("Running hook for [" + entry.name + "]: " + hook, Logger::Info);
            QProcess proc;
            proc.start("/bin/sh", {"-c", hook});
            if (!proc.waitForFinished(10000)) {
                Logger::log("Hook timed out for: " + entry.name, Logger::Warning);
                proc.kill();
            } else if (proc.exitCode() != 0) {
                Logger::log("Hook exited with code " +
                                QString::number(proc.exitCode()) +
                                " for: " + entry.name,
                            Logger::Warning);
            }
        }
    }

    Logger::log("UniversalThemeExporter: Template sync complete.", Logger::Info);
}
