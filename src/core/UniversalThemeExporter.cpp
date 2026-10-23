#include "UniversalThemeExporter.h"
#include "TemplateApplyService.h"
#include "PaletteCatalog.h"
#include <QApplication>
#include <QCoreApplication>
#include <QTimer>
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

    QString currentPal = TemplateConfig::getValue("palette", "current", "");
    if (currentPal.isEmpty()) {
        currentPal = QStringLiteral("Breeze");
    }
    if (!currentPal.isEmpty()) {
        const ThemePalette *pal = PaletteCatalog::instance().findPalette(currentPal);
        if (pal) {
            bool isDark = true;
            KConfigGroup group = config->group(QStringLiteral("General"));
            QString scheme = group.readEntry(QStringLiteral("ColorScheme"), QString());
            if (scheme.contains("Light", Qt::CaseInsensitive)) {
                isDark = false;
            } else if (scheme.contains("Dark", Qt::CaseInsensitive)) {
                isDark = true;
            } else {
                KConfigGroup winGroup = config->group(QStringLiteral("Colors:Window"));
                QColor winBg = winGroup.readEntry("BackgroundNormal", QColor(30, 30, 30));
                isDark = (winBg.lightness() < 128);
            }
            return pal->toUniversalPalette(isDark);
        }
    }

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

    palette.specialColors["terminal_bg"] = palette.windowBg.name();
    palette.specialColors["terminal_background"] = palette.windowBg.name();
    palette.specialColors["terminal_fg"] = palette.windowFg.name();
    palette.specialColors["terminal_foreground"] = palette.windowFg.name();
    palette.specialColors["cursor"] = palette.accent.name();
    palette.specialColors["terminal_cursor"] = palette.accent.name();
    palette.specialColors["cursor_text"] = palette.windowBg.name();
    palette.specialColors["terminal_cursor_text"] = palette.windowBg.name();
    palette.specialColors["selection_bg"] = palette.selection.name();
    palette.specialColors["terminal_selection_bg"] = palette.selection.name();
    palette.specialColors["selection_fg"] = palette.windowFg.name();
    palette.specialColors["terminal_selection_fg"] = palette.windowFg.name();

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
            const ThemePalette *pal = PaletteCatalog::instance().findPalette(
                PaletteCatalog::instance().activePaletteName());
            if (pal) {
                return pal->toUniversalPalette(false);
            }
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
            const ThemePalette *pal = PaletteCatalog::instance().findPalette(
                PaletteCatalog::instance().activePaletteName());
            if (pal) {
                return pal->toUniversalPalette(true);
            }
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
        // If no backup exists but the file exists, it was created by us, so remove it!
        QFile file(path);
        if (file.exists()) {
            if (file.remove()) {
                Logger::log("Removed generated file (no backup existed): " + path, Logger::Info);
                // Also clean up Vesktop folders if Vencord
                if (path.endsWith("Vencord/themes/PlasmaMaster.theme.css")) {
                    QStringList vesktopPaths = {
                        QDir::homePath() + "/.config/vesktop/themes/PlasmaMaster.theme.css",
                        QDir::homePath() + "/.var/app/dev.vencord.Vesktop/config/vesktop/themes/PlasmaMaster.theme.css"
                    };
                    for (const QString &vpath : vesktopPaths) {
                        QFile vfile(vpath);
                        if (vfile.exists()) {
                            vfile.remove();
                            Logger::log("Removed Vesktop sync file: " + vpath, Logger::Info);
                        }
                    }
                }
                return true;
            }
            Logger::log("Failed to remove generated file: " + path, Logger::Error);
            return false;
        }
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
        
        // Also clean up Vesktop folders if Vencord
        if (path.endsWith("Vencord/themes/PlasmaMaster.theme.css")) {
            QStringList vesktopPaths = {
                QDir::homePath() + "/.config/vesktop/themes/PlasmaMaster.theme.css",
                QDir::homePath() + "/.var/app/dev.vencord.Vesktop/config/vesktop/themes/PlasmaMaster.theme.css"
            };
            for (const QString &vpath : vesktopPaths) {
                QFile vfile(vpath);
                if (vfile.exists()) {
                    vfile.remove();
                    Logger::log("Removed Vesktop sync file: " + vpath, Logger::Info);
                }
            }
        }
        
        return true;
    }
    Logger::log("Failed to restore from backup: " + backupPath, Logger::Error);
    return false;
}

// =============================================================================
// Core sync logic
// =============================================================================

void UniversalThemeExporter::syncAll() {
    if (qobject_cast<QApplication*>(QCoreApplication::instance())) {
        TemplateApplyService::instance().applyAsync();
    } else {
        TemplateApplyService::instance().applySync();
    }
}


