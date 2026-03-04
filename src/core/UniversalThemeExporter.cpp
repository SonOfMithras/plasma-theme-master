#include "UniversalThemeExporter.h"
#include "ThemeWriter.h"
#include "Config.h"
#include "Solar.h"
#include <KColorScheme>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QThread>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include "ThemeReader.h"
#include "GlobalThemeManager.h"
#include "Logger.h"

UniversalPalette UniversalThemeExporter::extractColors() {
    auto config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    config->reparseConfiguration(); // Force reload from disk to get latest changes

    if (Config::isMaterialYouOverrideEnabled()) {
        KConfigGroup group = config->group(QStringLiteral("General"));
        QString scheme = group.readEntry(QStringLiteral("ColorScheme"), QString());
        
        if (scheme != "MaterialYouLight" && scheme != "MaterialYouDark") {
            // Intelligent Fallback: Check structural theme configuration first
            QString currentGlobal = ThemeReader::currentGlobalTheme();
            QString currentKvantum = ThemeReader::currentKvantumTheme();
            
            if (currentGlobal == ThemeReader::defaultDarkTheme() || (!currentKvantum.isEmpty() && currentKvantum == ThemeReader::nightKvantumTheme())) {
                scheme = "MaterialYouDark";
                Logger::log("Universal Fallback: Deduced Night based on active structural themes.", Logger::Info);
            } else if (currentGlobal == ThemeReader::defaultLightTheme() || (!currentKvantum.isEmpty() && currentKvantum == ThemeReader::dayKvantumTheme())) {
                scheme = "MaterialYouLight";
                Logger::log("Universal Fallback: Deduced Day based on active structural themes.", Logger::Info);
            } else {
                // Absolute final fallback to solar clock
                Logger::log("Universal Fallback: Both colorscheme and structural themes unrecognized. Falling back to Solar Clock.", Logger::Warning);
                double lat = ThemeReader::nativeLatitude();
                double lon = ThemeReader::nativeLongitude();
                int offset = ThemeReader::solarPadding();
                bool isDay = Solar::isDaytime(lat, lon, offset);
                scheme = isDay ? "MaterialYouLight" : "MaterialYouDark";
            }
        }
        
        QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "color-schemes/" + scheme + ".colors");
        if (path.isEmpty()) {
            path = QDir::homePath() + "/.local/share/color-schemes/" + scheme + ".colors";
        }
        
        if (QFile::exists(path)) {
            Logger::log("UniversalThemeExporter: Sourcing colors from " + path, Logger::Info);
            return extractColors(path);
        } else {
             Logger::log("UniversalThemeExporter: Failed to find " + scheme + ".colors", Logger::Warning);
        }
    }

    return extractColorsFromConfig(config);
}

UniversalPalette UniversalThemeExporter::extractColors(const QString &configPath) {
    if (configPath.isEmpty() || !QFile::exists(configPath)) {
        // Fallback to current if path invalid
        Logger::log("Invalid config path for color extraction: " + configPath, Logger::Warning);
        return extractColors(); 
    }
    auto config = KSharedConfig::openConfig(configPath, KConfig::SimpleConfig);
    // Explicitly break Qt memory cache to re-read updated MaterialYou hexes
    config->reparseConfiguration(); 
    return extractColorsFromConfig(config);
}

UniversalPalette UniversalThemeExporter::extractColorsFromConfig(KSharedConfig::Ptr config) {
    UniversalPalette palette;
    
    // Load system color scheme
    KColorScheme windowScheme(QPalette::Active, KColorScheme::Window, config);
    KColorScheme viewScheme(QPalette::Active, KColorScheme::View, config);
    KColorScheme selectionScheme(QPalette::Active, KColorScheme::Selection, config);
    KColorScheme buttonScheme(QPalette::Active, KColorScheme::Button, config);

    KColorScheme headerScheme(QPalette::Active, KColorScheme::Header, config);

    // Extract Colors
    palette.windowBg = windowScheme.background(KColorScheme::NormalBackground).color();
    palette.windowFg = windowScheme.foreground(KColorScheme::NormalText).color();
    
    palette.viewBg = viewScheme.background(KColorScheme::NormalBackground).color();
    palette.viewFg = viewScheme.foreground(KColorScheme::NormalText).color();
    
    palette.accent = selectionScheme.background(KColorScheme::NormalBackground).color();
    palette.selection = selectionScheme.background(KColorScheme::NormalBackground).color(); // Typically accent, but explicit
    
    palette.success = windowScheme.foreground(KColorScheme::PositiveText).color();
    palette.warning = windowScheme.foreground(KColorScheme::NeutralText).color();
    palette.error = windowScheme.foreground(KColorScheme::NegativeText).color();

    palette.titleBarBg = headerScheme.background(KColorScheme::NormalBackground).color();
    palette.titleBarFg = headerScheme.foreground(KColorScheme::NormalText).color();
    
    palette.buttonBg = buttonScheme.background(KColorScheme::NormalBackground).color();
    palette.buttonFg = buttonScheme.foreground(KColorScheme::NormalText).color();

    // Map Semantic Roles to ANSI Colors
    // 0: Black (Window BG or dark variant)
    palette.ansiBlack = windowScheme.background(KColorScheme::NormalBackground).color(); // Darkest
    // 1: Red (Negative)
    palette.ansiRed = windowScheme.foreground(KColorScheme::NegativeText).color();
    // 2: Green (Positive)
    palette.ansiGreen = windowScheme.foreground(KColorScheme::PositiveText).color();
    // 3: Yellow (Neutral)
    palette.ansiYellow = windowScheme.foreground(KColorScheme::NeutralText).color();
    // 4: Blue (Focus/Active)
    palette.ansiBlue = selectionScheme.background(KColorScheme::NormalBackground).color();
    // 5: Magenta (ActiveText often used for focus) or just reuse Blue derivative?
    palette.ansiMagenta = windowScheme.foreground(KColorScheme::ActiveText).color();
    // 6: Cyan (Link? or Visited?)
    palette.ansiCyan = windowScheme.foreground(KColorScheme::LinkText).color();
    // 7: White (Window FG)
    palette.ansiWhite = windowScheme.foreground(KColorScheme::NormalText).color();

    // Bright variants - for now, map to normal or slightly lighten
    palette.ansiBlackBright = palette.ansiBlack.lighter(120);
    palette.ansiRedBright = palette.ansiRed.lighter(120);
    palette.ansiGreenBright = palette.ansiGreen.lighter(120);
    palette.ansiYellowBright = palette.ansiYellow.lighter(120);
    palette.ansiBlueBright = palette.ansiBlue.lighter(120);
    palette.ansiMagentaBright = palette.ansiMagenta.lighter(120);
    palette.ansiCyanBright = palette.ansiCyan.lighter(120);
    palette.ansiWhiteBright = palette.ansiWhite.lighter(120);

    return palette;
}

QString UniversalThemeExporter::colorToHex(const QColor &color) {
    return color.name();
}

QString UniversalThemeExporter::colorToRgb(const QColor &color) {
    return QString("%1, %2, %3").arg(color.red()).arg(color.green()).arg(color.blue());
}

bool UniversalThemeExporter::writeToFile(const QString &path, const QString &content) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&file);
        out << content;
        file.close();
        Logger::log("Wrote universal theme file: " + path, Logger::Info);
        return true;
    }
    Logger::log("Failed to write to: " + path, Logger::Error);
    return false;
}

bool UniversalThemeExporter::backupFile(const QString &path) {
    QFile file(path);
    if (!file.exists()) return true; // Nothing to backup

    QString backupPath = path + ".bak";
    if (QFile::exists(backupPath)) return true; // Backup already exists, don't overwrite original backup logic

    if (file.copy(backupPath)) {
        Logger::log("Created backup: " + backupPath, Logger::Info);
        return true;
    } else {
        Logger::log("Failed to create backup: " + backupPath, Logger::Error);
        return false;
    }
}

bool UniversalThemeExporter::restoreFile(const QString &path) {
    QString backupPath = path + ".bak";
    QFile backup(backupPath);
    if (!backup.exists()) {
        Logger::log("No backup found for: " + path, Logger::Warning);
        return false;
    }

    QFile file(path);
    if (file.exists()) {
        if (!file.remove()) {
            Logger::log("Failed to remove current file during restore: " + path, Logger::Error);
            return false;
        }
    }

    if (backup.copy(path)) {
        Logger::log("Restored from backup: " + backupPath, Logger::Info);
        return true;
    } else {
        Logger::log("Failed to restore from backup: " + backupPath, Logger::Error);
        return false;
    }
}

// -----------------------------------------------------------------------------
// Exporters
// -----------------------------------------------------------------------------

bool UniversalThemeExporter::exportToVSCode(const UniversalPalette &palette) {
    if (!Config::isVSCodeSyncEnabled()) return false;

    QStringList paths = {
        QDir::homePath() + "/.config/Code/User/settings.json",
        QDir::homePath() + "/.config/Code - OSS/User/settings.json",
        QDir::homePath() + "/.config/VSCodium/User/settings.json",
        "/home/ammar/Code/Antigravity/.vscode/settings.json" // Explicit Antigravity workspace
    };

    bool anySuccess = false;
    for (const QString &path : paths) {
        if (exportToVSCodeJSON(path, palette)) {
            anySuccess = true;
        }
    }
    return anySuccess;
}

bool UniversalThemeExporter::exportToVSCodeJSON(const QString &path, const UniversalPalette &palette) {
    QFile file(path);
    QFileInfo fileInfo(path);
    if (!QDir(fileInfo.absolutePath()).exists()) {
        QDir().mkpath(fileInfo.absolutePath());
    }
    
    // If file doesn't exist, create empty Object
    QJsonObject root;
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
             QByteArray data = file.readAll();
             QJsonDocument doc = QJsonDocument::fromJson(data);
             if (doc.isObject()) {
                 root = doc.object();
             }
             file.close();
        }
    }

    QJsonObject colors;
    if (root.contains("workbench.colorCustomizations")) {
        colors = root["workbench.colorCustomizations"].toObject();
    }

    // Update keys
    colors["titleBar.activeBackground"] = colorToHex(palette.titleBarBg.isValid() ? palette.titleBarBg : palette.windowBg);
    colors["titleBar.activeForeground"] = colorToHex(palette.titleBarFg.isValid() ? palette.titleBarFg : palette.windowFg);
    colors["activityBar.background"] = colorToHex(palette.windowBg);
    colors["sideBar.background"] = colorToHex(palette.windowBg);
    colors["statusBar.background"] = colorToHex(palette.windowBg);
    colors["statusBar.foreground"] = colorToHex(palette.viewFg);
    colors["editor.background"] = colorToHex(palette.viewBg);
    colors["editor.foreground"] = colorToHex(palette.viewFg);
    colors["list.activeSelectionBackground"] = colorToHex(palette.selection);
    colors["terminal.background"] = colorToHex(palette.ansiBlack);
    colors["terminal.foreground"] = colorToHex(palette.ansiWhite);
    
    root["workbench.colorCustomizations"] = colors;
    
    // Backup before write
    backupFile(path);

    QSaveFile saveFile(path);
    if (saveFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(root);
        saveFile.write(doc.toJson());
        if (saveFile.commit()) {
            Logger::log("Wrote VS Code theme to: " + path, Logger::Info);
            return true;
        }
    }
    
    Logger::log("Failed to write to: " + path, Logger::Error);
    return false;
}

void UniversalThemeExporter::syncAll() {
    Logger::log("Triggering Universal Theme Sync...", Logger::Info);
    UniversalPalette palette = extractColors();
    
    // VS Code
    if (Config::isVSCodeSyncEnabled()) {
        exportToVSCode(palette);
    }
    
    if (Config::isFirefoxSyncEnabled()) {
        exportToFirefox(palette);
    }

    if (Config::isBetterDiscordSyncEnabled()) {
        exportToBetterDiscord(palette);
    }
   
    if (Config::isKittySyncEnabled()) {
        exportToKitty(palette);
    }
    
    if (Config::isKonsoleSyncEnabled()) {
        exportToKonsole(palette);
    }
    
    if (Config::isVencordSyncEnabled()) {
        exportToVencord(palette);
    }
    if (Config::isBtopSyncEnabled()) {
        exportToBtop(palette);
    }
    if (Config::isVicinaeSyncEnabled()) {
        exportToVicinae(palette);
    }
    
    if (Config::isObsidianSyncEnabled()) {
        QString vault = Config::obsidianVaultPath();
        if (!vault.isEmpty()) exportToObsidian(palette, vault);
    }
}

bool UniversalThemeExporter::exportToFirefox(const UniversalPalette &palette) {
    if (!Config::isFirefoxSyncEnabled()) return false;

    // We export to both Firefox and Zen Browser if found.
    // Implementation: Write a CSS file that defines variables
    QString cssContent = QString(
        ":root {\n"
        "  --plasma-window-bg: %1;\n"
        "  --plasma-window-fg: %2;\n"
        "  --plasma-view-bg: %3;\n"
        "  --plasma-view-fg: %4;\n"
        "  --plasma-accent: %5;\n"
        "}\n"
        "/* Automated Theme Override */\n"
        "#navigator-toolbox { background-color: var(--plasma-window-bg) !important; }\n"
        ".tab-background[selected=\"true\"] { background-color: var(--plasma-accent) !important; color: #fff !important; }\n"
        "/* Zen Browser Specifics might go here if needed */\n"
    ).arg(colorToHex(palette.windowBg))
     .arg(colorToHex(palette.windowFg))
     .arg(colorToHex(palette.viewBg))
     .arg(colorToHex(palette.viewFg))
     .arg(colorToHex(palette.accent));

    bool success = false;
    
    // Arrays of paths to check
    struct BrowserPath {
        QString name;
        QString path;
    };
    
    QList<BrowserPath> browsers = {
        {"Firefox", QDir::homePath() + "/.mozilla/firefox"},
        {"Zen", QDir::homePath() + "/.zen"},
        {"Zen (Flatpak)", QDir::homePath() + "/.var/app/app.zen_browser.zen/.zen"}
    };
    
    for (const auto &browser : browsers) {
        QDir browserDir(browser.path);
        if (!browserDir.exists()) continue;

        QStringList profiles = browserDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &profile : profiles) {
            // We look for profiles that are likely active (default or specific naming)
            // Or just try to inject into all found profiles that contain a prefs.js or extensions
            // Simpler: Just look for folder.
            
            QString chromeDir = browser.path + "/" + profile + "/chrome";
            // For Zen, it might share structure or differ. Docs say standard userChrome.css location.
            
            // Heuristic: If it has "default" in name or is the only one.
            if (profile.contains("default") || profile.contains("release") || profiles.count() < 3) { 
                 if (!QDir(chromeDir).exists()) QDir().mkpath(chromeDir);

                // Write generated plasma-colors.css
                backupFile(chromeDir + "/plasma-colors.css");
                if (writeToFile(chromeDir + "/plasma-colors.css", cssContent)) {
                     // Check userChrome.css
                    QString userChromePath = chromeDir + "/userChrome.css";
                    QFile userChrome(userChromePath);
                    QString importLine = "@import \"plasma-colors.css\";";
                    
                    backupFile(userChromePath);
                    
                    if (userChrome.open(QIODevice::ReadWrite | QIODevice::Text)) {
                        QString content = userChrome.readAll();
                        if (!content.contains("plasma-colors.css")) {
                            QTextStream out(&userChrome);
                            out.seek(0);
                            out << importLine << "\n" << content;
                        }
                        userChrome.close();
                        success = true;
                        Logger::log("Exported theme to " + browser.name + " profile: " + profile, Logger::Info);
                    } else {
                        // Create new
                        writeToFile(userChromePath, importLine);
                        success = true;
                        Logger::log("Created theme for " + browser.name + " profile: " + profile, Logger::Info);
                    }
                }
            }
        }
    }
    
    return success;
}

bool UniversalThemeExporter::exportToBetterDiscord(const UniversalPalette &palette) {
    if (!Config::isBetterDiscordSyncEnabled()) return false;
    // Standalone theme in themes folder
    QString themePath = QDir::homePath() + "/.config/BetterDiscord/themes/PlasmaMaster.theme.css";
    QFileInfo fileInfo(themePath);
    if (!QDir(fileInfo.absolutePath()).exists()) {
        QDir().mkpath(fileInfo.absolutePath());
    }

    // Prepare Imports
    QString imports = "";
    
    // 1. Base Theme (Midnight instead of Material)
    if (Config::isBetterDiscordMaterialEnabled()) { // Using the same bool for backwards compatibility
        imports += "@import url(\"https://refact0r.github.io/midnight-discord/build/midnight.css\");\n";
    }

    // 2. Custom / User Selected Imports
    QStringList customImports = Config::betterDiscordImports();
    for (const QString &url : customImports) {
        if (!url.trimmed().isEmpty()) {
            QString cleanUrl = url.trimmed();
            if (!cleanUrl.startsWith("@import")) {
                imports += QString("@import url(\"%1\");\n").arg(cleanUrl);
            } else {
                imports += cleanUrl + "\n";
            }
        }
    }

    QString css = QString(
        "/**\n"
        " * @name Plasma Master (BetterDiscord)\n"
        " * @description A dynamic discord theme based on Midnight, synchronized with KDE Plasma.\n"
        " * @author Plasma Theme Master\n"
        " * @version 1.0.0\n"
        " */\n"
        "%1\n\n"
        ":root {\n"
        "  /* text colors */\n"
        "  --text-0: %2;\n"
        "  --text-1: %3;\n"
        "  --text-2: %4;\n"
        "  --text-3: %5;\n"
        "  --text-4: %6;\n"
        "  --text-5: %7;\n"
        "  /* background and dark colors */\n"
        "  --bg-1: %8;\n"
        "  --bg-2: %9;\n"
        "  --bg-3: %10;\n"
        "  --bg-4: %11;\n"
        "  /* accent colors */\n"
        "  --accent-1: %12;\n"
        "  --accent-2: %13;\n"
        "  --accent-3: %14;\n"
        "  --accent-4: %15;\n"
        "  --accent-5: %16;\n"
        "  --accent-new: %21;\n"
        "  /* status indicator colors */\n"
        "  --online: %17;\n"
        "  --dnd: %18;\n"
        "  --idle: %19;\n"
        "  --streaming: %20;\n"
        "}\n"
    ).arg(imports)
     .arg(colorToHex(palette.windowBg))                    // text-0 (text on colored elements)
     .arg(colorToHex(palette.windowFg.lighter(120)))       // text-1
     .arg(colorToHex(palette.windowFg))                    // text-2
     .arg(colorToHex(palette.viewFg))                      // text-3 (normal)
     .arg(colorToHex(palette.viewFg.darker(150)))          // text-4 (muted)
     .arg(colorToHex(palette.viewFg.darker(180)))          // text-5 (very muted)
     .arg(colorToHex(palette.buttonBg.darker(150)))        // bg-1
     .arg(colorToHex(palette.buttonBg))                    // bg-2
     .arg(colorToHex(palette.viewBg))                      // bg-3
     .arg(colorToHex(palette.windowBg))                    // bg-4
     .arg(colorToHex(palette.accent.lighter(120)))         // accent-1
     .arg(colorToHex(palette.accent))                      // accent-2
     .arg(colorToHex(palette.accent))                      // accent-3
     .arg(colorToHex(palette.accent.lighter(110)))         // accent-4
     .arg(colorToHex(palette.accent.darker(110)))          // accent-5
     .arg(colorToHex(palette.success))                     // online
     .arg(colorToHex(palette.error))                       // dnd
     .arg(colorToHex(palette.warning))                     // idle
     .arg(colorToHex(palette.ansiMagenta))                 // streaming
     .arg(colorToHex(palette.error));                      // accent-new (usually red for mute/deafen)

    backupFile(themePath);
    bool res = writeToFile(themePath, css);
    if (res) Logger::log("Exported theme to BetterDiscord: " + themePath, Logger::Info);
    return res;
}

bool UniversalThemeExporter::exportToObsidian(const UniversalPalette &palette, const QString &vaultPath) {
    if (!Config::isObsidianSyncEnabled()) return false;
    if (vaultPath.isEmpty()) return false;
    
    QString snippetPath = vaultPath + "/.obsidian/snippets";
    QDir().mkpath(snippetPath);
    
    QString css = QString(
        "body {\n"
        "  --background-primary: %1;\n"
        "  --background-secondary: %2;\n"
        "  --text-normal: %3;\n"
        "  --interactive-accent: %4;\n"
        "}\n"
    ).arg(colorToHex(palette.viewBg))
     .arg(colorToHex(palette.windowBg))
     .arg(colorToHex(palette.viewFg))
     .arg(colorToHex(palette.accent));

    backupFile(snippetPath + "/plasma-master.css");
    bool res = writeToFile(snippetPath + "/plasma-master.css", css);
    if (res) Logger::log("Exported theme to Obsidian: " + vaultPath, Logger::Info);
    return res;
}

bool UniversalThemeExporter::exportToKitty(const UniversalPalette &) {
    if (!Config::isKittySyncEnabled()) return false;
    
    // We ignore the passed palette because we need to generate two distinct files 
    // for Day and Night, based on the stored Global Theme settings.
    
    struct ThemeTarget {
        QString name;
        QString path;
        QString globalTheme;
    };
    
    QList<ThemeTarget> targets = {
        {"Day", QDir::homePath() + "/.config/kitty/light-theme.auto.conf", ThemeReader::defaultLightTheme()},
        {"Night", QDir::homePath() + "/.config/kitty/dark-theme.auto.conf", ThemeReader::defaultDarkTheme()}
    };
    
    // Helper to generate content
    auto generateConfig = [](const UniversalPalette &p) -> QString {
        return QString(
            "foreground %1\n"
            "background %2\n"
            "selection_foreground %3\n"
            "selection_background %4\n"
            "active_border_color %5\n"
            "url_color %6\n\n"
            "# black\n"
            "color0 %7\n"
            "color8 %8\n\n"
            "# red\n"
            "color1 %9\n"
            "color9 %10\n\n"
            "# green\n"
            "color2 %11\n"
            "color10 %12\n\n"
            "# yellow\n"
            "color3 %13\n"
            "color11 %14\n\n"
            "# blue\n"
            "color4 %15\n"
            "color12 %16\n\n"
            "# magenta\n"
            "color5 %17\n"
            "color13 %18\n\n"
            "# cyan\n"
            "color6 %19\n"
            "color14 %20\n\n"
            "# white\n"
            "color7 %21\n"
            "color15 %22\n"
        ).arg(colorToHex(p.ansiWhite)) // Foreground usually matches Text
         .arg(colorToHex(p.ansiBlack)) // Background usually matches Window
         .arg(colorToHex(p.ansiWhite)) // Selection FG
         .arg(colorToHex(p.selection)) // Selection BG
         .arg(colorToHex(p.accent)) // Active Border
         .arg(colorToHex(p.ansiCyan)) // URL Color
         .arg(colorToHex(p.ansiBlack)) .arg(colorToHex(p.ansiBlackBright))
         .arg(colorToHex(p.ansiRed))   .arg(colorToHex(p.ansiRedBright))
         .arg(colorToHex(p.ansiGreen)) .arg(colorToHex(p.ansiGreenBright))
         .arg(colorToHex(p.ansiYellow)).arg(colorToHex(p.ansiYellowBright))
         .arg(colorToHex(p.ansiBlue))  .arg(colorToHex(p.ansiBlueBright))
         .arg(colorToHex(p.ansiMagenta)).arg(colorToHex(p.ansiMagentaBright))
         .arg(colorToHex(p.ansiCyan))  .arg(colorToHex(p.ansiCyanBright))
         .arg(colorToHex(p.ansiWhite)) .arg(colorToHex(p.ansiWhiteBright));
    };

    bool anySuccess = false;
    
    for (const auto &target : targets) {
        if (target.globalTheme.isEmpty()) {
            Logger::log("Skipping Kitty export for " + target.name + ": No Global Theme set.", Logger::Warning);
            continue;
        }
        
        QString colorsPath;
        if (Config::isMaterialYouOverrideEnabled()) {
             QString schemeName = (target.name == "Day") ? "MaterialYouLight" : "MaterialYouDark";
             colorsPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "color-schemes/" + schemeName + ".colors");
             if (colorsPath.isEmpty()) {
                 colorsPath = QDir::homePath() + "/.local/share/color-schemes/" + schemeName + ".colors";
             }
        } else {
             QString schemeName = GlobalThemeManager::getColorSchemeFromGlobal(target.globalTheme);
             colorsPath = GlobalThemeManager::findColorSchemePath(schemeName);
             if (colorsPath.isEmpty()) {
                  Logger::log("Could not find color scheme file for " + target.globalTheme + " (" + schemeName + "). Using current system colors as fallback.", Logger::Warning);
             }
        }
        
        UniversalPalette palette = extractColors(colorsPath);
        QString content = generateConfig(palette);
        
        QFileInfo fileInfo(target.path);
        QDir().mkpath(fileInfo.absolutePath());
        
        if (writeToFile(target.path, content)) {
            anySuccess = true;
            Logger::log("Exported Kitty config (" + target.name + "): " + target.path, Logger::Info);
        }
    }
    
    // Cleanup old plasma-colors.conf inclusion if present
    // We don't want to break user's config, but we should remove our old include.
    // The user instruction said: "check the contents of the existing files for how the file should be set up"
    // and "rework the revert kitty logic to just delete light and dark-theme.auto.conf"
    // But we should also stop adding the old include.
    
    return anySuccess;
}

bool UniversalThemeExporter::exportToKonsole(const UniversalPalette &palette) {
    if (!Config::isKonsoleSyncEnabled()) return false;

    // 1. Generate Color Scheme
    QString schemePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/konsole/PlasmaMaster.colorscheme";
    QFileInfo schemeInfo(schemePath);
    QDir().mkpath(schemeInfo.absolutePath());

    auto rgb = [](const QColor &c) { return QString("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue()); };

    QString schemeContent = QString(
        "[General]\n"
        "Description=PlasmaMaster\n"
        "Opacity=1\n"
        "Wallpaper=\n\n"
        
        "[Background]\n"
        "Color=%1\n\n"
        "[BackgroundIntense]\n"
        "Color=%2\n\n"
        
        "[Foreground]\n"
        "Color=%3\n\n"
        "[ForegroundIntense]\n"
        "Color=%4\n\n"

        "[Color0]\n"
        "Color=%5\n\n"
        "[Color0Intense]\n"
        "Color=%6\n\n"

        "[Color1]\n"
        "Color=%7\n\n"
        "[Color1Intense]\n"
        "Color=%8\n\n"

        "[Color2]\n"
        "Color=%9\n\n"
        "[Color2Intense]\n"
        "Color=%10\n\n"

        "[Color3]\n"
        "Color=%11\n\n"
        "[Color3Intense]\n"
        "Color=%12\n\n"

        "[Color4]\n"
        "Color=%13\n\n"
        "[Color4Intense]\n"
        "Color=%14\n\n"

        "[Color5]\n"
        "Color=%15\n\n"
        "[Color5Intense]\n"
        "Color=%16\n\n"

        "[Color6]\n"
        "Color=%17\n\n"
        "[Color6Intense]\n"
        "Color=%18\n\n"

        "[Color7]\n"
        "Color=%19\n\n"
        "[Color7Intense]\n"
        "Color=%20\n\n"
    ).arg(rgb(palette.ansiBlack))
     .arg(rgb(palette.ansiBlack))
     .arg(rgb(palette.ansiWhite))
     .arg(rgb(palette.ansiWhiteBright))
     .arg(rgb(palette.ansiBlack))        .arg(rgb(palette.ansiBlackBright))
     .arg(rgb(palette.ansiRed))          .arg(rgb(palette.ansiRedBright))
     .arg(rgb(palette.ansiGreen))        .arg(rgb(palette.ansiGreenBright))
     .arg(rgb(palette.ansiYellow))       .arg(rgb(palette.ansiYellowBright))
     .arg(rgb(palette.ansiBlue))         .arg(rgb(palette.ansiBlueBright))
     .arg(rgb(palette.ansiMagenta))      .arg(rgb(palette.ansiMagentaBright))
     .arg(rgb(palette.ansiCyan))         .arg(rgb(palette.ansiCyanBright))
     .arg(rgb(palette.ansiWhite))        .arg(rgb(palette.ansiWhiteBright));

    writeToFile(schemePath, schemeContent);

    // 2. Generate Profile
    QString profilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/konsole/PlasmaMaster.profile";
    QString profileContent = 
        "[Appearance]\n"
        "ColorScheme=PlasmaMaster\n\n"
        "[General]\n"
        "Name=PlasmaMaster\n"
        "Parent=FALLBACK/\n";
        
    bool profileWritten = writeToFile(profilePath, profileContent);
    
    // 3. Set as Default Profile
    if (profileWritten) {
        QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/konsolerc";
        KConfig config(configPath, KConfig::SimpleConfig);
        KConfigGroup group = config.group("Desktop Entry");
        group.writeEntry("DefaultProfile", "PlasmaMaster.profile");
        config.sync();
        Logger::log("Set Konsole default profile to PlasmaMaster", Logger::Info);
    }

    return true;
}

// -----------------------------------------------------------------------------
// Restore Methods
// -----------------------------------------------------------------------------

bool UniversalThemeExporter::restoreVSCode() {
    QStringList paths = {
        QDir::homePath() + "/.config/Code/User/settings.json",
        QDir::homePath() + "/.config/Code - OSS/User/settings.json",
        QDir::homePath() + "/.config/VSCodium/User/settings.json"
    };
    bool anySuccess = false;
    for (const auto &p : paths) {
        if (restoreFile(p)) anySuccess = true;
    }
    return anySuccess;
}

bool UniversalThemeExporter::restoreFirefox() {
    // Basic restore of known paths. 
    // Cleaning up "plasma-colors.css" inside profile is tricky without re-scanning.
    // For now, relies on restoreFile logic if the user has a backup of userChrome.css.
    // We can also try to delete plasma-colors.css if found.
    return false; // Not fully implemented for robust cleanup yet without scanning
}

bool UniversalThemeExporter::restoreBetterDiscord() {
    QString themePath = QDir::homePath() + "/.config/BetterDiscord/themes/PlasmaMaster.theme.css";
    QFile f(themePath);
    if (f.exists()) {
        if (f.remove()) {
             Logger::log("Removed BetterDiscord theme: " + themePath, Logger::Info);
             return true;
        }
    }
    return false;
}

bool UniversalThemeExporter::restoreKitty() {
    // 1. Remove include from kitty.conf by restoring backup (Legacy cleanup)
    QString kittyConf = QDir::homePath() + "/.config/kitty/kitty.conf";
    restoreFile(kittyConf);
    
    // 2. Delete plasma-colors.conf (Legacy cleanup)
    QString colorsPath = QDir::homePath() + "/.config/kitty/plasma-colors.conf";
    QFile f(colorsPath);
    if (f.exists()) {
        f.remove();
        Logger::log("Removed Kitty colors file: " + colorsPath, Logger::Info);
    }

    // 3. Delete new auto conf files
    QString lightPath = QDir::homePath() + "/.config/kitty/light-theme.auto.conf";
    if (QFile::exists(lightPath)) QFile::remove(lightPath);

    QString darkPath = QDir::homePath() + "/.config/kitty/dark-theme.auto.conf";
    if (QFile::exists(darkPath)) QFile::remove(darkPath);
    
    Logger::log("Removed Kitty auto theme files", Logger::Info);
    
    return true;
}

bool UniversalThemeExporter::restoreObsidian() {
    // Delete the snippet if we know the path. 
    // We need Config to know valid path, or checking Config::obsidianVaultPath()
    QString vault = Config::obsidianVaultPath();
    if (vault.isEmpty()) return false;
    
    QString path = vault + "/.obsidian/snippets/plasma-master.css";
    if (QFile::exists(path)) {
        QFile::remove(path);
        Logger::log("Removed Obsidian snippet: " + path, Logger::Info);
        return true;
    }
    return false;
}

bool UniversalThemeExporter::restoreKonsole() {
    QString schemePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/konsole/PlasmaMaster.colorscheme";
    QString profilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/konsole/PlasmaMaster.profile";
    
    if (QFile::exists(schemePath)) QFile::remove(schemePath);
    if (QFile::exists(profilePath)) QFile::remove(profilePath);

    Logger::log("Removed PlasmaMaster Konsole files. You may need to manually reset your default Konsole profile.", Logger::Warning);
    
    return true;
}


// -----------------------------------------------------------------------------
// Scanners
// -----------------------------------------------------------------------------

QStringList UniversalThemeExporter::scanBetterDiscordImports() {
    QStringList results;
    QDir dir(QDir::homePath() + "/.config/BetterDiscord/themes");
    if (!dir.exists()) return results;
    
    QStringList files = dir.entryList(QStringList() << "*.css", QDir::Files);
    for (const QString &filename : files) {
        if (filename == "PlasmaMaster.theme.css") continue; // Skip self
        
        QFile f(dir.absoluteFilePath(filename));
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&f);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.startsWith("@import url")) {
                    // Extract URL roughly
                    int start = line.indexOf('"');
                    int end = line.lastIndexOf('"');
                    if (start != -1 && end != -1 && end > start) {
                         QString url = line.mid(start + 1, end - start - 1);
                         if (!results.contains(url)) {
                             results << url;
                         }
                    }
                 }
             }
             f.close();
         }
     }
     return results;
}

QStringList UniversalThemeExporter::scanVencordImports() {
    QStringList results;
    QDir dir(QDir::homePath() + "/.config/Vencord/themes");
    if (!dir.exists()) return results;
    
    QStringList files = dir.entryList(QStringList() << "*.css", QDir::Files);
    for (const QString &filename : files) {
        if (filename == "PlasmaMaster.theme.css") continue; // Skip self
        
        QFile f(dir.absoluteFilePath(filename));
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&f);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.startsWith("@import url")) {
                    // Extract URL roughly
                    int start = line.indexOf('"');
                    int end = line.lastIndexOf('"');
                    if (start != -1 && end != -1 && end > start) {
                         QString url = line.mid(start + 1, end - start - 1);
                         if (!results.contains(url)) {
                             results << url;
                         }
                    }
                }
            }
            f.close();
        }
    }
    return results;
}


// -----------------------------------------------------------------------------
// Vencord
// -----------------------------------------------------------------------------
bool UniversalThemeExporter::exportToVencord(const UniversalPalette &palette) {
    if (!Config::isVencordSyncEnabled()) return false;
    
    // Vencord uses a standard themes folder
    QString themePath = QDir::homePath() + "/.config/Vencord/themes/PlasmaMaster.theme.css";
    QFileInfo fileInfo(themePath);
    if (!QDir(fileInfo.absolutePath()).exists()) {
        QDir().mkpath(fileInfo.absolutePath());
    }

    QString imports = "";
    if (Config::isVencordMidnightEnabled()) {
        imports += "@import url(\"https://refact0r.github.io/midnight-discord/build/midnight.css\");\n";
    }

    // Custom / User Selected Imports
    QStringList customImports = Config::vencordImports();
    for (const QString &url : customImports) {
        if (!url.trimmed().isEmpty()) {
            QString cleanUrl = url.trimmed();
            if (!cleanUrl.startsWith("@import")) {
                imports += QString("@import url(\"%1\");\n").arg(cleanUrl);
            } else {
                imports += cleanUrl + "\n";
            }
        }
    }

    QString css = QString(
        "/**\n"
        " * @name Plasma Master (Vencord)\n"
        " * @description A dynamic discord theme based on Midnight, synchronized with KDE Plasma.\n"
        " * @author Plasma Theme Master\n"
        " * @version 1.0.0\n"
        " */\n"
        "%1\n\n"
        ":root {\n"
        "  /* text colors */\n"
        "  --text-0: %2;\n"
        "  --text-1: %3;\n"
        "  --text-2: %4;\n"
        "  --text-3: %5;\n"
        "  --text-4: %6;\n"
        "  --text-5: %7;\n"
        "  /* background and dark colors */\n"
        "  --bg-1: %8;\n"
        "  --bg-2: %9;\n"
        "  --bg-3: %10;\n"
        "  --bg-4: %11;\n"
        "  /* accent colors */\n"
        "  --accent-1: %12;\n"
        "  --accent-2: %13;\n"
        "  --accent-3: %14;\n"
        "  --accent-4: %15;\n"
        "  --accent-5: %16;\n"
        "  --accent-new: %21;\n"
        "  /* status indicator colors */\n"
        "  --online: %17;\n"
        "  --dnd: %18;\n"
        "  --idle: %19;\n"
        "  --streaming: %20;\n"
        "}\n"
    ).arg(imports).arg(colorToHex(palette.windowBg))                    // text-0 (text on colored elements)
     .arg(colorToHex(palette.windowFg.lighter(120)))       // text-1
     .arg(colorToHex(palette.windowFg))                    // text-2
     .arg(colorToHex(palette.viewFg))                      // text-3 (normal)
     .arg(colorToHex(palette.viewFg.darker(150)))          // text-4 (muted)
     .arg(colorToHex(palette.viewFg.darker(180)))          // text-5 (very muted)
     .arg(colorToHex(palette.buttonBg.darker(150)))        // bg-1
     .arg(colorToHex(palette.buttonBg))                    // bg-2
     .arg(colorToHex(palette.viewBg))                      // bg-3
     .arg(colorToHex(palette.windowBg))                    // bg-4
     .arg(colorToHex(palette.accent.lighter(120)))         // accent-1
     .arg(colorToHex(palette.accent))                      // accent-2
     .arg(colorToHex(palette.accent))                      // accent-3
     .arg(colorToHex(palette.accent.lighter(110)))         // accent-4
     .arg(colorToHex(palette.accent.darker(110)))          // accent-5
     .arg(colorToHex(palette.success))                     // online
     .arg(colorToHex(palette.error))                       // dnd
     .arg(colorToHex(palette.warning))                     // idle
     .arg(colorToHex(palette.ansiMagenta))                 // streaming
     .arg(colorToHex(palette.error));                      // accent-new (usually red for mute/deafen)

    return writeToFile(themePath, css);
}

bool UniversalThemeExporter::restoreVencord() {
    QString themePath = QDir::homePath() + "/.config/Vencord/themes/PlasmaMaster.theme.css";
    return QFile::remove(themePath);
}

// -----------------------------------------------------------------------------
// Btop
// -----------------------------------------------------------------------------
bool UniversalThemeExporter::exportToBtop(const UniversalPalette &palette) {
    if (!Config::isBtopSyncEnabled()) return false;
    
    QString themeDir = QDir::homePath() + "/.config/btop/themes";
    if (!QDir(themeDir).exists()) {
        QDir().mkpath(themeDir);
    }
    QString themePath = themeDir + "/plasma-theme-master.theme";

    QString btopTheme = QString(
        "# Theme generated by Plasma Theme Master\n"
        "theme[main_bg]=\"%3\"\n"
        "theme[main_fg]=\"%1\"\n"
        "theme[title]=\"%1\"\n"
        "theme[hi_fg]=\"%2\"\n"
        "theme[selected_bg]=\"%3\"\n"
        "theme[selected_fg]=\"%2\"\n"
        "theme[inactive_fg]=\"%4\"\n"
        "theme[graph_text]=\"%1\"\n"
        "theme[meter_bg]=\"%3\"\n"
        "theme[proc_misc]=\"%1\"\n"
        "theme[cpu_box]=\"%2\"\n"
        "theme[mem_box]=\"%5\"\n"
        "theme[net_box]=\"%6\"\n"
        "theme[proc_box]=\"%2\"\n"
        "theme[div_line]=\"%4\"\n"
        "theme[temp_start]=\"%5\"\n"
        "theme[temp_mid]=\"%7\"\n"
        "theme[temp_end]=\"%6\"\n"
        "theme[cpu_start]=\"%2\"\n"
        "theme[cpu_mid]=\"%2\"\n"
        "theme[cpu_end]=\"%2\"\n"
        "theme[free_start]=\"%5\"\n"
        "theme[free_mid]=\"%7\"\n"
        "theme[free_end]=\"%6\"\n"
        "theme[cached_start]=\"%2\"\n"
        "theme[cached_mid]=\"%2\"\n"
        "theme[cached_end]=\"%2\"\n"
        "theme[available_start]=\"%2\"\n"
        "theme[available_mid]=\"%2\"\n"
        "theme[available_end]=\"%2\"\n"
        "theme[used_start]=\"%5\"\n"
        "theme[used_mid]=\"%7\"\n"
        "theme[used_end]=\"%6\"\n"
        "theme[download_start]=\"%5\"\n"
        "theme[download_mid]=\"%7\"\n"
        "theme[download_end]=\"%6\"\n"
        "theme[upload_start]=\"%5\"\n"
        "theme[upload_mid]=\"%7\"\n"
        "theme[upload_end]=\"%6\"\n"
        "theme[process_start]=\"%2\"\n"
        "theme[process_mid]=\"%2\"\n"
        "theme[process_end]=\"%2\"\n"
    ).arg(colorToHex(palette.viewFg))
     .arg(colorToHex(palette.accent))
     .arg(colorToHex(palette.windowBg))
     .arg(colorToHex(palette.viewFg.darker(150)))
     .arg(colorToHex(palette.success))
     .arg(colorToHex(palette.error))
     .arg(colorToHex(palette.warning));

    if (!writeToFile(themePath, btopTheme)) {
        return false;
    }

    // Attempt to update btop configuration to use this theme if it's set to something else
    QString confPath = QDir::homePath() + "/.config/btop/btop.conf";
    QFile confFile(confPath);
    if (confFile.exists() && confFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString confContent = QTextStream(&confFile).readAll();
        QRegularExpression themeRegex("color_theme\\s*=\\s*\\\"[^\\\"]*\\\"");
        if (confContent.contains(themeRegex)) {
            confContent.replace(themeRegex, "color_theme = \"plasma-theme-master\"");
        } else {
            confContent += "\ncolor_theme = \"plasma-theme-master\"\n";
        }
        confFile.resize(0);
        QTextStream out(&confFile);
        out << confContent;
        confFile.close();
    }

    // Send SIGUSR2 to running btop instances to reload the theme
    QProcess process;
    process.start("pkill", QStringList() << "-USR2" << "btop");
    process.waitForFinished(1000);

    return true;
}

bool UniversalThemeExporter::restoreBtop() {
    QString themePath = QDir::homePath() + "/.config/btop/themes/plasma-theme-master.theme";
    return QFile::remove(themePath);
}

// -----------------------------------------------------------------------------
// Vicinae
// -----------------------------------------------------------------------------
bool UniversalThemeExporter::exportToVicinae(const UniversalPalette &palette) {
    if (!Config::isVicinaeSyncEnabled()) return false;
    
    QString themeDir = QDir::homePath() + "/.local/share/vicinae/themes";
    if (!QDir(themeDir).exists()) {
        QDir().mkpath(themeDir);
    }
    QString themePath = themeDir + "/plasma-theme-master.toml";

    QString isDark = (palette.windowBg.lightness() < 128) ? "dark" : "light";
    QString inherits = "vicinae-" + isDark;

    QString vicinaeTheme = QString(
        "[meta]\n"
        "version = 1\n"
        "name = \"Plasma Theme Master\"\n"
        "description = \"Dynamically generated theme matching KDE Plasma\"\n"
        "variant = \"%8\"\n"
        "inherits = \"%9\"\n\n"
        "[colors.core]\n"
        "accent = \"%1\"\n"
        "accent_foreground = \"%2\"\n"
        "background = \"%3\"\n"
        "foreground = \"%4\"\n"
        "secondary_background = \"%5\"\n"
        "border = \"%6\"\n\n"
        "[colors.accents]\n"
        "blue = \"%1\"\n"
        "green = \"%10\"\n"
        "red = \"%11\"\n"
        "yellow = \"%12\"\n"
    ).arg(colorToHex(palette.accent))
     .arg(colorToHex(palette.viewBg))
     .arg(colorToHex(palette.windowBg))
     .arg(colorToHex(palette.viewFg))
     .arg(colorToHex(palette.windowBg.darker(110)))
     .arg(colorToHex(palette.windowBg.darker(120)))
     .arg(colorToHex(palette.windowBg)) 
     .arg(isDark)
     .arg(inherits)
     .arg(colorToHex(palette.success))
     .arg(colorToHex(palette.error))
     .arg(colorToHex(palette.warning));

    if (writeToFile(themePath, vicinaeTheme)) {
        QProcess::startDetached("vicinae", QStringList() << "theme" << "set" << "plasma-theme-master");
        Logger::log("Exported theme to Vicinae and triggered update.", Logger::Info);
        return true;
    }
    return false;
}

bool UniversalThemeExporter::restoreVicinae() {
    QString themePath = QDir::homePath() + "/.local/share/vicinae/themes/plasma-theme-master.toml";
    return QFile::remove(themePath);
}

