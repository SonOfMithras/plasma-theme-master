#include "PaletteCatalog.h"
#include "TemplateConfig.h"
#include "TemplateApplyService.h"
#include "UniversalThemeExporter.h"
#include "ThemeWriter.h"
#include "Config.h"
#include "Logger.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <KConfig>
#include <KConfigGroup>

UniversalPalette ThemePalette::toUniversalPalette(bool isDark) const {
    const PaletteModeColors &m = isDark ? dark : light;

    UniversalPalette p;
    p.windowBg = m.surface.isValid() ? m.surface : (isDark ? QColor("#232629") : QColor("#eff0f1"));
    p.windowFg = m.onSurface.isValid() ? m.onSurface : (isDark ? QColor("#eff0f1") : QColor("#232629"));
    p.viewBg = m.surfaceVariant.isValid() ? m.surfaceVariant : (isDark ? QColor("#1b1e20") : QColor("#ffffff"));
    p.viewFg = p.windowFg;
    p.accent = m.primary.isValid() ? m.primary : QColor("#3daee9");
    p.selection = p.accent;
    p.error = m.error.isValid() ? m.error : QColor("#da4453");
    p.warning = m.tertiary.isValid() ? m.tertiary : QColor("#f67400");
    p.buttonBg = m.surfaceVariant.isValid() ? m.surfaceVariant : (isDark ? QColor("#31363b") : QColor("#e4e6e8"));
    p.buttonFg = p.windowFg;

    // Terminal Normal Colors (Official KDE Breeze Console)
    p.ansiBlack   = m.terminalNormal[0].isValid() ? m.terminalNormal[0] : (isDark ? QColor("#232629") : QColor("#232629"));
    p.ansiRed     = m.terminalNormal[1].isValid() ? m.terminalNormal[1] : QColor("#ed1515");
    p.ansiGreen   = m.terminalNormal[2].isValid() ? m.terminalNormal[2] : QColor("#11d116");
    p.ansiYellow  = m.terminalNormal[3].isValid() ? m.terminalNormal[3] : QColor("#f67400");
    p.ansiBlue    = m.terminalNormal[4].isValid() ? m.terminalNormal[4] : QColor("#1d99f3");
    p.ansiMagenta = m.terminalNormal[5].isValid() ? m.terminalNormal[5] : QColor("#9b59b6");
    p.ansiCyan    = m.terminalNormal[6].isValid() ? m.terminalNormal[6] : QColor("#3daee9");
    p.ansiWhite   = m.terminalNormal[7].isValid() ? m.terminalNormal[7] : (isDark ? QColor("#eff0f1") : QColor("#232629"));

    // Terminal Bright Colors (Official KDE Breeze Console)
    p.ansiBlackBright   = m.terminalBright[0].isValid() ? m.terminalBright[0] : (isDark ? QColor("#7f8c8d") : QColor("#7f8c8d"));
    p.ansiRedBright     = m.terminalBright[1].isValid() ? m.terminalBright[1] : QColor("#c0392b");
    p.ansiGreenBright   = m.terminalBright[2].isValid() ? m.terminalBright[2] : QColor("#55ff55");
    p.ansiYellowBright  = m.terminalBright[3].isValid() ? m.terminalBright[3] : QColor("#fdbc4b");
    p.ansiBlueBright    = m.terminalBright[4].isValid() ? m.terminalBright[4] : QColor("#3daee9");
    p.ansiMagentaBright = m.terminalBright[5].isValid() ? m.terminalBright[5] : QColor("#8e44ad");
    p.ansiCyanBright    = m.terminalBright[6].isValid() ? m.terminalBright[6] : QColor("#16a085");
    p.ansiWhiteBright   = m.terminalBright[7].isValid() ? m.terminalBright[7] : QColor("#ffffff");
    p.titleBarBg = p.windowBg;
    p.titleBarFg = p.windowFg;

    if (m.terminalBg.isValid()) {
        p.specialColors["terminal_bg"] = m.terminalBg.name();
        p.specialColors["terminal_background"] = m.terminalBg.name();
    }
    if (m.terminalFg.isValid()) {
        p.specialColors["terminal_fg"] = m.terminalFg.name();
        p.specialColors["terminal_foreground"] = m.terminalFg.name();
    }
    if (m.terminalCursor.isValid()) {
        p.specialColors["cursor"] = m.terminalCursor.name();
        p.specialColors["terminal_cursor"] = m.terminalCursor.name();
    }
    if (m.terminalCursorText.isValid()) {
        p.specialColors["cursor_text"] = m.terminalCursorText.name();
        p.specialColors["terminal_cursor_text"] = m.terminalCursorText.name();
    }
    if (m.terminalSelectionBg.isValid()) {
        p.specialColors["selection_bg"] = m.terminalSelectionBg.name();
        p.specialColors["terminal_selection_bg"] = m.terminalSelectionBg.name();
    }
    if (m.terminalSelectionFg.isValid()) {
        p.specialColors["selection_fg"] = m.terminalSelectionFg.name();
        p.specialColors["terminal_selection_fg"] = m.terminalSelectionFg.name();
    }

    return p;
}

PaletteCatalog &PaletteCatalog::instance() {
    static PaletteCatalog inst;
    return inst;
}

PaletteCatalog::PaletteCatalog() {
    reload();
}

QStringList PaletteCatalog::paletteSearchPaths() {
    QStringList paths;
    paths << QDir::homePath() + "/.local/share/plasma-theme-master/palettes";

    // Development source path (when running uninstalled or from build tree)
    QString appDirSrcTree = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../src/core/palettes");
    if (QDir(appDirSrcTree).exists() && !paths.contains(appDirSrcTree)) paths << appDirSrcTree;

    QString srcTree = QDir::cleanPath(QDir::currentPath() + "/src/core/palettes");
    if (QDir(srcTree).exists() && !paths.contains(srcTree)) paths << srcTree;

    QString binRel = QCoreApplication::applicationDirPath() + "/palettes";
    if (!paths.contains(binRel)) paths << binRel;

    paths << "/usr/share/plasma-theme-master/palettes";

    QString dataLoc = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                            "plasma-theme-master/palettes",
                                            QStandardPaths::LocateDirectory);
    if (!dataLoc.isEmpty() && !paths.contains(dataLoc)) paths << dataLoc;

    return paths;
}

static PaletteModeColors parseModeJson(const QJsonObject &obj) {
    PaletteModeColors m;
    m.primary = QColor(obj.value("mPrimary").toString());
    m.onPrimary = QColor(obj.value("mOnPrimary").toString());
    m.secondary = QColor(obj.value("mSecondary").toString());
    m.onSecondary = QColor(obj.value("mOnSecondary").toString());
    m.tertiary = QColor(obj.value("mTertiary").toString());
    m.onTertiary = QColor(obj.value("mOnTertiary").toString());
    m.error = QColor(obj.value("mError").toString());
    m.onError = QColor(obj.value("mOnError").toString());
    m.surface = QColor(obj.value("mSurface").toString());
    m.onSurface = QColor(obj.value("mOnSurface").toString());
    m.surfaceVariant = QColor(obj.value("mSurfaceVariant").toString());
    m.onSurfaceVariant = QColor(obj.value("mOnSurfaceVariant").toString());
    m.outline = QColor(obj.value("mOutline").toString());
    m.shadow = QColor(obj.value("mShadow").toString());
    m.hover = QColor(obj.value("mHover").toString());
    m.onHover = QColor(obj.value("mOnHover").toString());

    // Fallback if keys don't have 'm' prefix
    if (!m.primary.isValid()) m.primary = QColor(obj.value("primary").toString());
    if (!m.secondary.isValid()) m.secondary = QColor(obj.value("secondary").toString());
    if (!m.tertiary.isValid()) m.tertiary = QColor(obj.value("tertiary").toString());
    if (!m.error.isValid()) m.error = QColor(obj.value("error").toString());
    if (!m.surface.isValid()) m.surface = QColor(obj.value("surface").toString());
    if (!m.surfaceVariant.isValid()) m.surfaceVariant = QColor(obj.value("surfaceVariant").toString());
    if (!m.onSurface.isValid()) m.onSurface = QColor(obj.value("onSurface").toString());

    QJsonObject term = obj.value("terminal").toObject();
    m.terminalFg = QColor(term.value("foreground").toString());
    m.terminalBg = QColor(term.value("background").toString());
    m.terminalSelectionFg = QColor(term.value("selectionFg").toString());
    m.terminalSelectionBg = QColor(term.value("selectionBg").toString());
    m.terminalCursor = QColor(term.value("cursor").toString());
    m.terminalCursorText = QColor(term.value("cursorText").toString());

    static const QStringList colorNames = {
        "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"
    };

    QJsonObject norm = term.value("normal").toObject();
    for (int i = 0; i < 8; ++i) {
        m.terminalNormal[i] = QColor(norm.value(colorNames[i]).toString());
    }

    QJsonObject bright = term.value("bright").toObject();
    for (int i = 0; i < 8; ++i) {
        m.terminalBright[i] = QColor(bright.value(colorNames[i]).toString());
    }

    return m;
}

void PaletteCatalog::loadOfficialPalettes(const QString &filePath) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isObject()) return;
    QJsonObject root = doc.object();

    for (auto it = root.begin(); it != root.end(); ++it) {
        QString name = it.key();
        QJsonObject pObj = it.value().toObject();

        ThemePalette pal;
        pal.id = name.toLower().replace(' ', '-');
        pal.name = name;
        pal.isOfficial = true;
        pal.isUser = false;
        pal.dark = parseModeJson(pObj.value("dark").toObject());
        pal.light = parseModeJson(pObj.value("light").toObject());

        if (!m_lookup.contains(pal.id)) {
            m_lookup[pal.id] = m_palettes.size();
            m_lookup[pal.name.toLower()] = m_palettes.size();
            m_palettes.append(pal);
        }
    }
}

void PaletteCatalog::loadCommunityPalettes(const QString &filePath) {
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    if (!doc.isObject()) return;
    QJsonObject root = doc.object();

    for (auto it = root.begin(); it != root.end(); ++it) {
        QString name = it.key();
        QJsonObject pObj = it.value().toObject();

        ThemePalette pal;
        pal.id = name.toLower().replace(' ', '-');
        pal.name = name;
        pal.isOfficial = false;
        pal.isUser = false;
        pal.dark = parseModeJson(pObj.value("dark").toObject());
        pal.light = parseModeJson(pObj.value("light").toObject());

        if (!m_lookup.contains(pal.id) && !m_lookup.contains(pal.name.toLower())) {
            m_lookup[pal.id] = m_palettes.size();
            m_lookup[pal.name.toLower()] = m_palettes.size();
            m_palettes.append(pal);
        }
    }
}

void PaletteCatalog::loadUserPalettes(const QString &dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) return;

    QStringList jsonFiles = dir.entryList({"*.json"}, QDir::Files);
    for (const QString &fileName : jsonFiles) {
        QString filePath = dir.filePath(fileName);
        QFile f(filePath);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        f.close();

        if (!doc.isObject()) continue;
        QJsonObject root = doc.object();

        if (root.contains("dark") || root.contains("light")) {
            QString name = root.value("name").toString();
            if (name.isEmpty()) {
                name = QFileInfo(filePath).baseName();
            }
            ThemePalette pal;
            pal.id = name.toLower().replace(' ', '-');
            pal.name = name;
            pal.isOfficial = false;
            pal.isUser = true;
            pal.dark = parseModeJson(root.value("dark").toObject());
            pal.light = parseModeJson(root.value("light").toObject());

            if (!m_lookup.contains(pal.id) && !m_lookup.contains(pal.name.toLower())) {
                m_lookup[pal.id] = m_palettes.size();
                m_lookup[pal.name.toLower()] = m_palettes.size();
                m_palettes.append(pal);
            }
        } else {
            for (auto it = root.begin(); it != root.end(); ++it) {
                QString name = it.key();
                QJsonObject pObj = it.value().toObject();

                ThemePalette pal;
                pal.id = name.toLower().replace(' ', '-');
                pal.name = name;
                pal.isOfficial = false;
                pal.isUser = true;
                pal.dark = parseModeJson(pObj.value("dark").toObject());
                pal.light = parseModeJson(pObj.value("light").toObject());

                if (!m_lookup.contains(pal.id) && !m_lookup.contains(pal.name.toLower())) {
                    m_lookup[pal.id] = m_palettes.size();
                    m_lookup[pal.name.toLower()] = m_palettes.size();
                    m_palettes.append(pal);
                }
            }
        }
    }
}

int PaletteCatalog::bundledCount() const {
    int count = 0;
    for (const auto &p : m_palettes) {
        if (!p.isUser) count++;
    }
    return count;
}

int PaletteCatalog::userCount() const {
    int count = 0;
    for (const auto &p : m_palettes) {
        if (p.isUser) count++;
    }
    return count;
}

void PaletteCatalog::reload() {
    m_palettes.clear();
    m_lookup.clear();

    QStringList searchDirs = paletteSearchPaths();

    // 1. Load official palettes
    for (const QString &dir : searchDirs) {
        QString offPath = dir + "/official_palettes.json";
        if (QFile::exists(offPath)) {
            loadOfficialPalettes(offPath);
            break;
        }
    }

    // 2. Load community palettes
    for (const QString &dir : searchDirs) {
        QString commPath = dir + "/community_details.json";
        if (QFile::exists(commPath)) {
            loadCommunityPalettes(commPath);
            break;
        }
    }

    // 3. Load user-created custom palettes
    QString userDir = QDir::homePath() + "/.local/share/plasma-theme-master/palettes";
    loadUserPalettes(userDir);

    Logger::log(QString("PaletteCatalog: Loaded %1 palettes (%2 bundled, %3 user).")
                    .arg(m_palettes.size())
                    .arg(bundledCount())
                    .arg(userCount()),
                Logger::Info);
}

const ThemePalette *PaletteCatalog::findPalette(const QString &nameOrId) const {
    QString key = nameOrId.trimmed().toLower();
    if (m_lookup.contains(key)) {
        int idx = m_lookup.value(key);
        if (idx >= 0 && idx < m_palettes.size()) {
            return &m_palettes[idx];
        }
    }
    return nullptr;
}

QString PaletteCatalog::activePaletteName() const {
    QString name = TemplateConfig::getValue("palette", "current", "");
    if (name.isEmpty()) {
        name = QStringLiteral("Breeze");
    }
    return name;
}

void PaletteCatalog::setActivePaletteName(const QString &name) {
    TemplateConfig::setValue("palette", "current", name);
}

bool PaletteCatalog::applyPalette(const QString &paletteName, const QString &mode, bool syncTemplates) {
    const ThemePalette *pal = findPalette(paletteName);
    if (!pal) {
        pal = findPalette(QStringLiteral("Breeze"));
    }
    if (!pal && !m_palettes.isEmpty()) {
        pal = &m_palettes.first();
    }
    if (!pal) {
        Logger::log("PaletteCatalog: Palette not found: " + paletteName, Logger::Warning);
        return false;
    }

    bool isDark = (mode == "night");
    if (mode == "current") {
        UniversalPalette cur = UniversalThemeExporter::extractColors();
        isDark = (cur.windowBg.lightness() < 128);
    }

    setActivePaletteName(pal->name);

    // 1. Generate KDE Plasma colorscheme (ptm.colors)
    QString colorsDir = QDir::homePath() + "/.local/share/color-schemes";
    QDir().mkpath(colorsDir);
    QString colorsPath = colorsDir + "/ptm.colors";

    UniversalPalette up = pal->toUniversalPalette(isDark);

    // Format RGB csv helper
    auto toCsv = [](const QColor &c) {
        return QString("%1,%2,%3").arg(c.red()).arg(c.green()).arg(c.blue());
    };

    QString kColorsContent = QString(
        "[KDE]\n"
        "contrast=4\n\n"
        "[General]\n"
        "ColorScheme=ptm\n"
        "Name=PTM\n"
        "AccentColor=%5\n"
        "lastUsedCustomAccentColor=%5\n\n"
        "[ColorEffects:Inactive]\n"
        "ChangeSelectionColor=false\n"
        "Color=0,0,0\n"
        "ColorAmount=0\n"
        "ColorEffect=0\n"
        "ContrastAmount=0\n"
        "ContrastEffect=0\n"
        "Enable=false\n"
        "IntensityAmount=0\n"
        "IntensityEffect=0\n\n"
        "[ColorEffects:Disabled]\n"
        "ChangeSelectionColor=false\n"
        "Enable=true\n\n"
        "[WM]\n"
        "activeBackground=%1\n"
        "activeBlend=%2\n"
        "activeForeground=%2\n"
        "inactiveBackground=%1\n"
        "inactiveBlend=%2\n"
        "inactiveForeground=%2\n\n"
        "[Colors:Window]\n"
        "BackgroundNormal=%1\n"
        "ForegroundNormal=%2\n"
        "BackgroundAlternate=%1\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:Window:Inactive]\n"
        "BackgroundNormal=%1\n"
        "ForegroundNormal=%2\n"
        "BackgroundAlternate=%1\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:View]\n"
        "BackgroundNormal=%3\n"
        "ForegroundNormal=%4\n"
        "BackgroundAlternate=%3\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:View:Inactive]\n"
        "BackgroundNormal=%3\n"
        "ForegroundNormal=%4\n"
        "BackgroundAlternate=%3\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:Selection]\n"
        "BackgroundNormal=%5\n"
        "ForegroundNormal=%6\n"
        "BackgroundAlternate=%5\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%6\n\n"
        "[Colors:Selection:Inactive]\n"
        "BackgroundNormal=%5\n"
        "ForegroundNormal=%6\n"
        "BackgroundAlternate=%5\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%6\n\n"
        "[Colors:Button]\n"
        "BackgroundNormal=%7\n"
        "ForegroundNormal=%8\n"
        "BackgroundAlternate=%7\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:Button:Inactive]\n"
        "BackgroundNormal=%7\n"
        "ForegroundNormal=%8\n"
        "BackgroundAlternate=%7\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:Tooltip]\n"
        "BackgroundNormal=%1\n"
        "ForegroundNormal=%2\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:Tooltip:Inactive]\n"
        "BackgroundNormal=%1\n"
        "ForegroundNormal=%2\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:Header]\n"
        "BackgroundNormal=%1\n"
        "ForegroundNormal=%2\n"
        "BackgroundAlternate=%1\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:Header:Inactive]\n"
        "BackgroundNormal=%1\n"
        "ForegroundNormal=%2\n"
        "BackgroundAlternate=%1\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:Complementary]\n"
        "BackgroundNormal=%1\n"
        "ForegroundNormal=%2\n"
        "BackgroundAlternate=%1\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n\n"
        "[Colors:Complementary:Inactive]\n"
        "BackgroundNormal=%1\n"
        "ForegroundNormal=%2\n"
        "BackgroundAlternate=%1\n"
        "DecorationFocus=%5\n"
        "DecorationHover=%5\n"
        "ForegroundActive=%5\n"
    ).arg(toCsv(up.windowBg),
          toCsv(up.windowFg),
          toCsv(up.viewBg),
          toCsv(up.viewFg),
          toCsv(up.selection),
          toCsv(isDark ? Qt::white : Qt::black),
          toCsv(up.buttonBg),
          toCsv(up.buttonFg));

    QSaveFile sf(colorsPath);
    if (sf.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream(&sf) << kColorsContent;
        sf.commit();
        Logger::log("PaletteCatalog: Generated " + colorsPath, Logger::Info);
    }

    // Explicitly update ~/.config/kdeglobals so accent and inactive effects are immediate
    {
        KConfig kdeglobals(QStringLiteral("kdeglobals"), KConfig::SimpleConfig);
        KConfigGroup genGroup = kdeglobals.group(QStringLiteral("General"));
        genGroup.writeEntry(QStringLiteral("AccentColor"), toCsv(up.selection));
        genGroup.writeEntry(QStringLiteral("lastUsedCustomAccentColor"), toCsv(up.selection));

        KConfigGroup inactGroup = kdeglobals.group(QStringLiteral("ColorEffects:Inactive"));
        inactGroup.writeEntry(QStringLiteral("Enable"), false);
        inactGroup.writeEntry(QStringLiteral("ChangeSelectionColor"), false);
        inactGroup.writeEntry(QStringLiteral("ColorAmount"), 0);
        inactGroup.writeEntry(QStringLiteral("ContrastAmount"), 0);
        inactGroup.writeEntry(QStringLiteral("IntensityAmount"), 0);

        KConfigGroup wmGroup = kdeglobals.group(QStringLiteral("WM"));
        wmGroup.writeEntry(QStringLiteral("activeBackground"), toCsv(up.windowBg));
        wmGroup.writeEntry(QStringLiteral("activeBlend"), toCsv(up.windowFg));
        wmGroup.writeEntry(QStringLiteral("activeForeground"), toCsv(up.windowFg));
        wmGroup.writeEntry(QStringLiteral("inactiveBackground"), toCsv(up.windowBg));
        wmGroup.writeEntry(QStringLiteral("inactiveBlend"), toCsv(up.windowFg));
        wmGroup.writeEntry(QStringLiteral("inactiveForeground"), toCsv(up.windowFg));

        kdeglobals.sync();
    }

    // =========================================================================
    // EXECUTION ORDER:
    // Step 1: Force color scheme update with Breeze flip -> ptm, updating app palette
    // Step 2: GTK live reload hook
    // Step 3: All other application templates
    // =========================================================================

    // Step 1: Force color scheme update
    ThemeWriter::applyColorScheme(QStringLiteral("ptm"), true);

    // Step 2 & 3: Trigger Universal Template sync
    if (syncTemplates) {
        if (qobject_cast<QApplication*>(QCoreApplication::instance())) {
            TemplateApplyService::instance().applyAsync(mode);
        } else {
            TemplateApplyService::instance().applySync(mode, 25000);
        }
    }

    Logger::log(QString("PaletteCatalog: Applied palette %1 (%2)").arg(pal->name, mode), Logger::Info);
    return true;
}

bool PaletteCatalog::applyActivePalette(bool isDay, bool syncTemplates) {
    if (Config::isMaterialYouOverrideEnabled()) {
        return false;
    }
    QString active = activePaletteName();
    if (active.isEmpty()) {
        active = QStringLiteral("Breeze");
    }
    return applyPalette(active, isDay ? QStringLiteral("day") : QStringLiteral("night"), syncTemplates);
}

