#include "UniversalThemeExporter.h"
#include "ThemeWriter.h"
#include "Config.h"
#include <KColorScheme>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QThread>
#include "Logger.h"

UniversalPalette UniversalThemeExporter::extractColors() {
    UniversalPalette palette;
    
    // Load system color scheme
    auto config = KSharedConfig::openConfig();
    config->reparseConfiguration(); // Force reload from disk to get latest changes
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

static void colorToHsl(const QColor &color, int &h, int &s, int &l) {
    color.getHsl(&h, &s, &l);
    // Convert ranges:
    // QColor: H(0-359), S(0-255), L(0-255)
    // CSS:    H(0-360), S(0-100%), L(0-100%)
    if (h == -1) h = 0; // Achromatic
    s = (s * 100) / 255;
    l = (l * 100) / 255;
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
        if (file.open(QIODevice::ReadWrite)) {
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

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonDocument doc(root);
        file.write(doc.toJson());
        file.close();
        Logger::log("Wrote VS Code theme to: " + path, Logger::Info);
        return true;
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
    
    if (Config::isFirefoxSyncEnabled()) exportToFirefox(palette);
    if (Config::isBetterDiscordSyncEnabled()) exportToBetterDiscord(palette);
    if (Config::isKittySyncEnabled()) exportToKitty(palette); 
    if (Config::isKonsoleSyncEnabled()) exportToKonsole(palette);
    if (Config::isGenericSyncEnabled()) exportGeneric(palette); 
    
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
    
    // 1. Base Material Theme (Toggled) - First for overrides
    if (Config::isBetterDiscordMaterialEnabled()) {
        imports += "@import url(\"https://capnkitten.github.io/BetterDiscord/Themes/Material-Discord/css/source.css\");\n";
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

    // 3. Default Addons (Icons, Custom Themes, Material You) - LAST
    imports += "@import url(\"https://capnkitten.github.io/BetterDiscord/Themes/Material-Discord/css/addons/custom-themes/source.css\");\n"
               "@import url(\"https://capnkitten.github.io/BetterDiscord/Themes/Material-Discord/css/addons/material-you/source.css\");\n";

    // Helper for HSL extraction
    int ah, as, al; colorToHsl(palette.accent, ah, as, al);
    int wh, ws, wl; colorToHsl(palette.warning, wh, ws, wl); // Using Warning for Warning
    int eh, es, el; colorToHsl(palette.error, eh, es, el);     // Using Error for Alert

    QString css = QString(
        "/**\n"
        " * @name Plasma Master (Material)\n"
        " * @version 1.0.0\n"
        " * @description A theme based on Google's Material Design, synchronized with KDE Plasma.\n"
        " * @author Plasma Theme Master\n"
        " * @source https://github.com/CapnKitten/BetterDiscord/blob/master/Themes/Material-Discord/css/source.css\n"
        " */\n\n"
        "%16\n" 
        ":root {\n"
        "    /* APP FONT SETTINGS */\n"
        "    --app-font: \"Google Sans Flex\";\n"
        "    --app-font-width: 100;\n"
        "    --app-font-roundness: 100;\n\n"
        "    /* ACCENT HSL */\n"
        "    --accent-hue: %1;\n"
        "    --accent-saturation: %2%;\n"
        "    --accent-lightness: %3%;\n"
        "    --accent-text-color: hsl(0,0%,100%);\n\n"
        "    /* ALERT (Error) HSL */\n"
        "    --alert-hue: %4;\n"
        "    --alert-saturation: %5%;\n"
        "    --alert-lightness: %6%;\n"
        "    --alert-text-color: hsl(0,0%,100%);\n\n"
        "    /* WARNING HSL */\n"
        "    --warning-hue: %7;\n"
        "    --warning-saturation: %8%;\n"
        "    --warning-lightness: %9%;\n"
        "    --warning-text-color: hsl(0,0%,100%);\n\n"
        "    /* Plasma Overrides (Enforce Backgrounds) */\n"
        "    /* Specific MaterialDiscord Vars */\n"
        "    --main-alt: %10 !important;             /* App Background */\n"
        "    --main-content-color: %11 !important;   /* Content Region */\n"
        "    --main-textarea-color: %13 !important;  /* Chat Input */\n"
        "    --main-textarea-border: %12 !important;\n"
        "    \n"
        "    /* Standard Discord Vars (Backup) */\n"
        "    --background-primary: %10 !important;\n"
        "    --background-secondary: %11 !important;\n"
        "    --background-secondary-alt: %11 !important;\n"
        "    --background-tertiary: %12 !important;\n"
        "    --channeltextarea-background: %13 !important;\n"
        "    --text-normal: %14 !important;\n"
        "    --text-muted: %15 !important;\n"
        "}\n\n"
        /* Dark/Light mode overrides for Material calculation adjustments */
        ".theme-dark {\n"
        "    --saturation-modifier: 1;\n"
        "    --lightness-modifier: 0.225;\n"
        "    --text-lightness-modifier: 1.0;\n"
        "    --ui-darkness-modifier: 1.0;\n"
        "}\n"
        ".theme-light {\n"
        "    --saturation-modifier: 1;\n"
        "    --lightness-modifier: 2.125;\n"
        "    --text-lightness-modifier: 1.0;\n"
        "}\n"
    ).arg(ah).arg(as).arg(al)      // 1-3: Accent
     .arg(eh).arg(es).arg(el)      // 4-6: Alert (Error)
     .arg(wh).arg(ws).arg(wl)      // 7-9: Warning
     .arg(colorToHex(palette.viewBg))               // 10: Primary BG (App BG)
     .arg(colorToHex(palette.windowBg))             // 11: Secondary BG
     .arg(colorToHex(palette.windowBg.darker(110))) // 12: Tertiary BG (Borders)
     .arg(colorToHex(palette.viewBg.darker(110)))   // 13: Text Area
     .arg(colorToHex(palette.viewFg))               // 14: Text Normal
     .arg(colorToHex(palette.viewFg.darker(130)))   // 15: Text Muted
     .arg(imports);                                 // 16: Imports

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

bool UniversalThemeExporter::exportToKitty(const UniversalPalette &palette) {
    if (!Config::isKittySyncEnabled()) return false;
    QString confPath = QDir::homePath() + "/.config/kitty/plasma-colors.conf";
    QFileInfo fileInfo(confPath);
    QDir().mkpath(fileInfo.absolutePath());

    QString conf = QString(
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
    ).arg(colorToHex(palette.ansiWhite)) // Foreground usually matches Text
     .arg(colorToHex(palette.ansiBlack)) // Background usually matches Window
     .arg(colorToHex(palette.ansiWhite)) // Selection FG
     .arg(colorToHex(palette.selection)) // Selection BG
     .arg(colorToHex(palette.accent)) // Active Border
     .arg(colorToHex(palette.ansiCyan)) // URL Color
     .arg(colorToHex(palette.ansiBlack)) .arg(colorToHex(palette.ansiBlackBright))
     .arg(colorToHex(palette.ansiRed))   .arg(colorToHex(palette.ansiRedBright))
     .arg(colorToHex(palette.ansiGreen)) .arg(colorToHex(palette.ansiGreenBright))
     .arg(colorToHex(palette.ansiYellow)).arg(colorToHex(palette.ansiYellowBright))
     .arg(colorToHex(palette.ansiBlue))  .arg(colorToHex(palette.ansiBlueBright))
     .arg(colorToHex(palette.ansiMagenta)).arg(colorToHex(palette.ansiMagentaBright))
     .arg(colorToHex(palette.ansiCyan))  .arg(colorToHex(palette.ansiCyanBright))
     .arg(colorToHex(palette.ansiWhite)) .arg(colorToHex(palette.ansiWhiteBright));

    backupFile(confPath);
    bool res = writeToFile(confPath, conf);
    if (res) {
         // Ensure kitty.conf includes this file
        QString originalConfPath = QDir::homePath() + "/.config/kitty/kitty.conf";
        QFile mainConf(originalConfPath);
        if (mainConf.open(QIODevice::ReadWrite | QIODevice::Text)) {
            QString content = mainConf.readAll();
            // Use standard include path format
            QString includeLine = "include " + confPath;
            QString shortInclude = "include ~/.config/kitty/plasma-colors.conf";
            
            if (!content.contains("plasma-colors.conf")) {
                QTextStream out(&mainConf);
                out.seek(mainConf.size()); // Append to end
                backupFile(originalConfPath);
                out << "\n# Plasma Theme Master\ninclude " << confPath << "\n";
                Logger::log("Added include to kitty.conf", Logger::Info);
            }
            mainConf.close();
        }
        Logger::log("Exported theme to Kitty", Logger::Info);
    }
    return res;
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

bool UniversalThemeExporter::exportGeneric(const UniversalPalette &palette) {
    if (!Config::isGenericSyncEnabled()) return false;
    QString cachePath = QDir::homePath() + "/.cache/plasma-theme-master/universal.css";
    QFileInfo fileInfo(cachePath);
    QDir().mkpath(fileInfo.absolutePath());
    
    QString css = QString(
        ":root {\n"
        "  --ptm-window-bg: %1;\n"
        "  --ptm-window-fg: %2;\n"
        "  --ptm-view-bg: %3;\n"
        "  --ptm-view-fg: %4;\n"
        "  --ptm-accent: %5;\n"
        "}\n"
    ).arg(colorToHex(palette.windowBg))
     .arg(colorToHex(palette.windowFg))
     .arg(colorToHex(palette.viewBg))
     .arg(colorToHex(palette.viewFg))
     .arg(colorToHex(palette.accent));

    return writeToFile(cachePath, css);
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
    // 1. Remove include from kitty.conf by restoring backup
    QString kittyConf = QDir::homePath() + "/.config/kitty/kitty.conf";
    restoreFile(kittyConf);
    
    // 2. Delete plasma-colors.conf
    QString colorsPath = QDir::homePath() + "/.config/kitty/plasma-colors.conf";
    QFile f(colorsPath);
    if (f.exists()) {
        f.remove();
        Logger::log("Removed Kitty colors file: " + colorsPath, Logger::Info);
    }
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

bool UniversalThemeExporter::restoreGeneric() {
    QString path = QDir::homePath() + "/.cache/plasma-theme-master/universal.css";
    if (QFile::exists(path)) {
        QFile::remove(path);
        Logger::log("Removed Generic CSS: " + path, Logger::Info);
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
