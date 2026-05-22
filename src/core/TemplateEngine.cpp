#include "TemplateEngine.h"
#include "Logger.h"
#include <QColor>
#include <QFile>
#include <QTextStream>

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

QString TemplateEngine::colorHex(const QColor &c) {
    return c.name(); // "#rrggbb"
}

QString TemplateEngine::colorRgb(const QColor &c) {
    return QString("%1, %2, %3").arg(c.red()).arg(c.green()).arg(c.blue());
}

QString TemplateEngine::colorRgbSpace(const QColor &c) {
    return QString("%1 %2 %3").arg(c.red()).arg(c.green()).arg(c.blue());
}

QString TemplateEngine::substitute(const QString &tmpl,
                                    const QMap<QString, QString> &vars) {
    QString result = tmpl;
    for (auto it = vars.cbegin(); it != vars.cend(); ++it) {
        result.replace("{{" + it.key() + "}}", it.value());
    }
    return result;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

QMap<QString, QString> TemplateEngine::buildVars(const UniversalPalette &p) {
    QMap<QString, QString> v;

    // Core semantic colors
    v["WINDOW_BG"]   = colorHex(p.windowBg);
    v["WINDOW_FG"]   = colorHex(p.windowFg);
    v["VIEW_BG"]     = colorHex(p.viewBg);
    v["VIEW_FG"]     = colorHex(p.viewFg);
    v["ACCENT"]      = colorHex(p.accent);
    v["SELECTION"]   = colorHex(p.selection);
    v["SUCCESS"]     = colorHex(p.success);
    v["WARNING"]     = colorHex(p.warning);
    v["ERROR"]       = colorHex(p.error);
    v["TITLEBAR_BG"] = colorHex(p.titleBarBg);
    v["TITLEBAR_FG"] = colorHex(p.titleBarFg);
    v["BUTTON_BG"]   = colorHex(p.buttonBg);
    v["BUTTON_FG"]   = colorHex(p.buttonFg);

    // Derived colors (lighter/darker variants used in existing templates)
    v["ACCENT_LIGHT"]    = colorHex(p.accent.lighter(120));
    v["ACCENT_DARK"]     = colorHex(p.accent.darker(110));
    v["WINDOW_FG_LIGHT"] = colorHex(p.windowFg.lighter(120));
    v["VIEW_FG_MUTED"]   = colorHex(p.viewFg.darker(150));
    v["VIEW_FG_DIM"]     = colorHex(p.viewFg.darker(180));
    v["BUTTON_BG_DARK"]  = colorHex(p.buttonBg.darker(150));
    v["WINDOW_BG_DARK"]  = colorHex(p.windowBg.darker(110));
    v["WINDOW_BG_DARK2"] = colorHex(p.windowBg.darker(120));

    // ANSI 16-color set — hex
    v["ANSI_BLACK"]          = colorHex(p.ansiBlack);
    v["ANSI_RED"]            = colorHex(p.ansiRed);
    v["ANSI_GREEN"]          = colorHex(p.ansiGreen);
    v["ANSI_YELLOW"]         = colorHex(p.ansiYellow);
    v["ANSI_BLUE"]           = colorHex(p.ansiBlue);
    v["ANSI_MAGENTA"]        = colorHex(p.ansiMagenta);
    v["ANSI_CYAN"]           = colorHex(p.ansiCyan);
    v["ANSI_WHITE"]          = colorHex(p.ansiWhite);
    v["ANSI_BLACK_BRIGHT"]   = colorHex(p.ansiBlackBright);
    v["ANSI_RED_BRIGHT"]     = colorHex(p.ansiRedBright);
    v["ANSI_GREEN_BRIGHT"]   = colorHex(p.ansiGreenBright);
    v["ANSI_YELLOW_BRIGHT"]  = colorHex(p.ansiYellowBright);
    v["ANSI_BLUE_BRIGHT"]    = colorHex(p.ansiBlueBright);
    v["ANSI_MAGENTA_BRIGHT"] = colorHex(p.ansiMagentaBright);
    v["ANSI_CYAN_BRIGHT"]    = colorHex(p.ansiCyanBright);
    v["ANSI_WHITE_BRIGHT"]   = colorHex(p.ansiWhiteBright);

    // ANSI colors as RGB tuples (r, g, b) — some formats (Konsole) need this
    v["ANSI_BLACK_RGB"]          = colorRgb(p.ansiBlack);
    v["ANSI_RED_RGB"]            = colorRgb(p.ansiRed);
    v["ANSI_GREEN_RGB"]          = colorRgb(p.ansiGreen);
    v["ANSI_YELLOW_RGB"]         = colorRgb(p.ansiYellow);
    v["ANSI_BLUE_RGB"]           = colorRgb(p.ansiBlue);
    v["ANSI_MAGENTA_RGB"]        = colorRgb(p.ansiMagenta);
    v["ANSI_CYAN_RGB"]           = colorRgb(p.ansiCyan);
    v["ANSI_WHITE_RGB"]          = colorRgb(p.ansiWhite);
    v["ANSI_BLACK_BRIGHT_RGB"]   = colorRgb(p.ansiBlackBright);
    v["ANSI_RED_BRIGHT_RGB"]     = colorRgb(p.ansiRedBright);
    v["ANSI_GREEN_BRIGHT_RGB"]   = colorRgb(p.ansiGreenBright);
    v["ANSI_YELLOW_BRIGHT_RGB"]  = colorRgb(p.ansiYellowBright);
    v["ANSI_BLUE_BRIGHT_RGB"]    = colorRgb(p.ansiBlueBright);
    v["ANSI_MAGENTA_BRIGHT_RGB"] = colorRgb(p.ansiMagentaBright);
    v["ANSI_CYAN_BRIGHT_RGB"]    = colorRgb(p.ansiCyanBright);
    v["ANSI_WHITE_BRIGHT_RGB"]   = colorRgb(p.ansiWhiteBright);

    // Core semantic colors as RGB
    v["WINDOW_BG_RGB"]   = colorRgb(p.windowBg);
    v["WINDOW_FG_RGB"]   = colorRgb(p.windowFg);
    v["VIEW_BG_RGB"]     = colorRgb(p.viewBg);
    v["VIEW_FG_RGB"]     = colorRgb(p.viewFg);
    v["ACCENT_RGB"]      = colorRgb(p.accent);
    v["SELECTION_RGB"]   = colorRgb(p.selection);
    v["SUCCESS_RGB"]     = colorRgb(p.success);
    v["WARNING_RGB"]     = colorRgb(p.warning);
    v["ERROR_RGB"]       = colorRgb(p.error);
    v["TITLEBAR_BG_RGB"] = colorRgb(p.titleBarBg);
    v["TITLEBAR_FG_RGB"] = colorRgb(p.titleBarFg);
    v["BUTTON_BG_RGB"]   = colorRgb(p.buttonBg);
    v["BUTTON_FG_RGB"]   = colorRgb(p.buttonFg);

    // Basic palette colors as RGB space-separated
    v["WINDOW_BG_RGB_SPACE"]   = colorRgbSpace(p.windowBg);
    v["WINDOW_FG_RGB_SPACE"]   = colorRgbSpace(p.windowFg);
    v["VIEW_BG_RGB_SPACE"]     = colorRgbSpace(p.viewBg);
    v["VIEW_FG_RGB_SPACE"]     = colorRgbSpace(p.viewFg);
    v["ACCENT_RGB_SPACE"]      = colorRgbSpace(p.accent);
    v["SELECTION_RGB_SPACE"]   = colorRgbSpace(p.selection);
    v["SUCCESS_RGB_SPACE"]     = colorRgbSpace(p.success);
    v["WARNING_RGB_SPACE"]     = colorRgbSpace(p.warning);
    v["ERROR_RGB_SPACE"]       = colorRgbSpace(p.error);
    v["TITLEBAR_BG_RGB_SPACE"] = colorRgbSpace(p.titleBarBg);
    v["TITLEBAR_FG_RGB_SPACE"] = colorRgbSpace(p.titleBarFg);
    v["BUTTON_BG_RGB_SPACE"]   = colorRgbSpace(p.buttonBg);
    v["BUTTON_FG_RGB_SPACE"]   = colorRgbSpace(p.buttonFg);

    // ANSI Colors as RGB space-separated
    v["ANSI_BLACK_RGB_SPACE"]          = colorRgbSpace(p.ansiBlack);
    v["ANSI_RED_RGB_SPACE"]            = colorRgbSpace(p.ansiRed);
    v["ANSI_GREEN_RGB_SPACE"]          = colorRgbSpace(p.ansiGreen);
    v["ANSI_YELLOW_RGB_SPACE"]         = colorRgbSpace(p.ansiYellow);
    v["ANSI_BLUE_RGB_SPACE"]           = colorRgbSpace(p.ansiBlue);
    v["ANSI_MAGENTA_RGB_SPACE"]        = colorRgbSpace(p.ansiMagenta);
    v["ANSI_CYAN_RGB_SPACE"]           = colorRgbSpace(p.ansiCyan);
    v["ANSI_WHITE_RGB_SPACE"]          = colorRgbSpace(p.ansiWhite);
    v["ANSI_BLACK_BRIGHT_RGB_SPACE"]   = colorRgbSpace(p.ansiBlackBright);
    v["ANSI_RED_BRIGHT_RGB_SPACE"]     = colorRgbSpace(p.ansiRedBright);
    v["ANSI_GREEN_BRIGHT_RGB_SPACE"]   = colorRgbSpace(p.ansiGreenBright);
    v["ANSI_YELLOW_BRIGHT_RGB_SPACE"]  = colorRgbSpace(p.ansiYellowBright);
    v["ANSI_BLUE_BRIGHT_RGB_SPACE"]    = colorRgbSpace(p.ansiBlueBright);
    v["ANSI_MAGENTA_BRIGHT_RGB_SPACE"] = colorRgbSpace(p.ansiMagentaBright);
    v["ANSI_CYAN_BRIGHT_RGB_SPACE"]    = colorRgbSpace(p.ansiCyanBright);
    v["ANSI_WHITE_BRIGHT_RGB_SPACE"]   = colorRgbSpace(p.ansiWhiteBright);

    // Derived colors as RGB
    v["ACCENT_LIGHT_RGB"]    = colorRgb(p.accent.lighter(120));
    v["ACCENT_DARK_RGB"]     = colorRgb(p.accent.darker(110));
    v["WINDOW_FG_LIGHT_RGB"] = colorRgb(p.windowFg.lighter(120));
    v["VIEW_FG_MUTED_RGB"]   = colorRgb(p.viewFg.darker(150));
    v["VIEW_FG_DIM_RGB"]     = colorRgb(p.viewFg.darker(180));
    v["BUTTON_BG_DARK_RGB"]  = colorRgb(p.buttonBg.darker(150));
    v["WINDOW_BG_DARK_RGB"]  = colorRgb(p.windowBg.darker(110));
    v["WINDOW_BG_DARK2_RGB"] = colorRgb(p.windowBg.darker(120));

    // Material Design 3 Color System Generation
    struct Md3Scheme {
        QColor background;
        QColor error;
        QColor errorContainer;
        QColor inverseOnSurface;
        QColor inversePrimary;
        QColor inverseSurface;
        QColor onBackground;
        QColor onError;
        QColor onErrorContainer;
        QColor onPrimary;
        QColor onPrimaryContainer;
        QColor onPrimaryFixed;
        QColor onPrimaryFixedVariant;
        QColor onSecondary;
        QColor onSecondaryContainer;
        QColor onSecondaryFixed;
        QColor onSecondaryFixedVariant;
        QColor onSurface;
        QColor onSurfaceVariant;
        QColor onTertiary;
        QColor onTertiaryContainer;
        QColor onTertiaryFixed;
        QColor onTertiaryFixedVariant;
        QColor outline;
        QColor outlineVariant;
        QColor primary;
        QColor primaryContainer;
        QColor primaryFixed;
        QColor primaryFixedDim;
        QColor scrim;
        QColor secondary;
        QColor secondaryContainer;
        QColor secondaryFixed;
        QColor secondaryFixedDim;
        QColor shadow;
        QColor surface;
        QColor surfaceBright;
        QColor surfaceContainer;
        QColor surfaceContainerHigh;
        QColor surfaceContainerHighest;
        QColor surfaceContainerLow;
        QColor surfaceContainerLowest;
        QColor surfaceDim;
        QColor surfaceVariant;
        QColor tertiary;
        QColor tertiaryContainer;
        QColor tertiaryFixed;
        QColor tertiaryFixedDim;
    };

    auto generateMd3Scheme = [&](bool targetDark) -> Md3Scheme {
        Md3Scheme s;

        auto desaturate = [](const QColor &c, double factor) -> QColor {
            int h, s, v, a;
            c.getHsv(&h, &s, &v, &a);
            return QColor::fromHsv(h, static_cast<int>(s * factor), v, a);
        };

        auto getNeutral = [](const QColor &accent, int val, int sat) -> QColor {
            int h, s, v, a;
            accent.getHsv(&h, &s, &v, &a);
            return QColor::fromHsv(h, (s < sat ? s : sat), val, a);
        };

        QColor mdPrimary = p.accent;
        QColor mdOnPrimary = (mdPrimary.lightness() < 128) ? Qt::white : Qt::black;
        QColor mdPrimaryContainer = targetDark ? mdPrimary.darker(180) : mdPrimary.lighter(150);
        QColor mdOnPrimaryContainer = targetDark ? mdPrimary.lighter(140) : mdPrimary.darker(160);

        QColor mdSecondary = targetDark ? desaturate(p.accent.lighter(110), 0.6) : desaturate(p.accent.darker(110), 0.6);
        QColor mdOnSecondary = (mdSecondary.lightness() < 128) ? Qt::white : Qt::black;
        QColor mdSecondaryContainer = targetDark ? mdSecondary.darker(180) : mdSecondary.lighter(150);
        QColor mdOnSecondaryContainer = targetDark ? mdSecondary.lighter(140) : mdSecondary.darker(160);

        QColor mdTertiary = targetDark ? p.ansiMagenta.lighter(110) : p.ansiMagenta.darker(110);
        QColor mdOnTertiary = (mdTertiary.lightness() < 128) ? Qt::white : Qt::black;
        QColor mdTertiaryContainer = targetDark ? mdTertiary.darker(180) : mdTertiary.lighter(150);
        QColor mdOnTertiaryContainer = targetDark ? mdTertiary.lighter(140) : mdTertiary.darker(160);

        bool palIsDark = (p.windowBg.lightness() < 128);
        QColor mdBackground, mdOnBackground, mdSurface, mdOnSurface;
        if (targetDark == palIsDark) {
            mdBackground = p.windowBg;
            mdOnBackground = p.windowFg;
            mdSurface = p.viewBg;
            mdOnSurface = p.viewFg;
        } else {
            if (targetDark) {
                mdBackground = getNeutral(p.accent, 24, 15);
                mdOnBackground = getNeutral(p.accent, 230, 10);
                mdSurface = getNeutral(p.accent, 24, 15);
                mdOnSurface = getNeutral(p.accent, 230, 10);
            } else {
                mdBackground = getNeutral(p.accent, 250, 15);
                mdOnBackground = getNeutral(p.accent, 28, 10);
                mdSurface = getNeutral(p.accent, 250, 15);
                mdOnSurface = getNeutral(p.accent, 28, 10);
            }
        }

        QColor mdSurfaceVariant = targetDark ? mdSurface.lighter(115) : mdSurface.darker(115);
        QColor mdOnSurfaceVariant = targetDark ? mdOnSurface.darker(115) : mdOnSurface.lighter(115);

        QColor mdOutline = targetDark ? mdSurface.lighter(130) : mdSurface.darker(130);
        QColor mdOutlineVariant = targetDark ? mdSurface.lighter(115) : mdSurface.darker(115);

        QColor mdShadow = Qt::black;
        QColor mdScrim = Qt::black;
        QColor mdInverseSurface = targetDark ? mdOnBackground : mdBackground;
        QColor mdInverseOnSurface = targetDark ? mdBackground : mdOnBackground;
        QColor mdInversePrimary = targetDark ? mdPrimary.darker(120) : mdPrimary.lighter(120);

        QColor mdPrimaryFixed = mdPrimary.lighter(120);
        QColor mdPrimaryFixedDim = mdPrimary;
        QColor mdOnPrimaryFixed = mdOnPrimary;
        QColor mdOnPrimaryFixedVariant = mdOnPrimaryContainer;

        QColor mdSecondaryFixed = mdSecondary.lighter(120);
        QColor mdSecondaryFixedDim = mdSecondary;
        QColor mdOnSecondaryFixed = mdOnSecondary;
        QColor mdOnSecondaryFixedVariant = mdOnSecondaryContainer;

        QColor mdTertiaryFixed = mdTertiary.lighter(120);
        QColor mdTertiaryFixedDim = mdTertiary;
        QColor mdOnTertiaryFixed = mdOnTertiary;
        QColor mdOnTertiaryFixedVariant = mdOnTertiaryContainer;

        QColor mdSurfaceDim = mdSurface;
        QColor mdSurfaceBright = targetDark ? mdSurface.lighter(120) : mdSurface.darker(105);
        QColor mdSurfaceContainerLowest = targetDark ? mdBackground.darker(110) : mdBackground.lighter(110);
        QColor mdSurfaceContainerLow = targetDark ? mdBackground.darker(105) : mdBackground.lighter(105);
        QColor mdSurfaceContainer = targetDark ? mdBackground.lighter(105) : mdBackground.darker(105);
        QColor mdSurfaceContainerHigh = targetDark ? mdBackground.lighter(110) : mdBackground.darker(110);
        QColor mdSurfaceContainerHighest = targetDark ? mdBackground.lighter(115) : mdBackground.darker(115);

        s.background = mdBackground;
        s.error = p.error;
        s.errorContainer = targetDark ? p.error.darker(200) : p.error.lighter(150);
        s.inverseOnSurface = mdInverseOnSurface;
        s.inversePrimary = mdInversePrimary;
        s.inverseSurface = mdInverseSurface;
        s.onBackground = mdOnBackground;
        s.onError = (p.error.lightness() < 128) ? Qt::white : Qt::black;
        s.onErrorContainer = targetDark ? p.error.lighter(140) : p.error.darker(160);
        s.onPrimary = mdOnPrimary;
        s.onPrimaryContainer = mdOnPrimaryContainer;
        s.onPrimaryFixed = mdOnPrimaryFixed;
        s.onPrimaryFixedVariant = mdOnPrimaryFixedVariant;
        s.onSecondary = mdOnSecondary;
        s.onSecondaryContainer = mdOnSecondaryContainer;
        s.onSecondaryFixed = mdOnSecondaryFixed;
        s.onSecondaryFixedVariant = mdOnSecondaryFixedVariant;
        s.onSurface = mdOnSurface;
        s.onSurfaceVariant = mdOnSurfaceVariant;
        s.onTertiary = mdOnTertiary;
        s.onTertiaryContainer = mdOnTertiaryContainer;
        s.onTertiaryFixed = mdOnTertiaryFixed;
        s.onTertiaryFixedVariant = mdOnTertiaryFixedVariant;
        s.outline = mdOutline;
        s.outlineVariant = mdOutlineVariant;
        s.primary = mdPrimary;
        s.primaryContainer = mdPrimaryContainer;
        s.primaryFixed = mdPrimaryFixed;
        s.primaryFixedDim = mdPrimaryFixedDim;
        s.scrim = mdScrim;
        s.secondary = mdSecondary;
        s.secondaryContainer = mdSecondaryContainer;
        s.secondaryFixed = mdSecondaryFixed;
        s.secondaryFixedDim = mdSecondaryFixedDim;
        s.shadow = mdShadow;
        s.surface = mdSurface;
        s.surfaceBright = mdSurfaceBright;
        s.surfaceContainer = mdSurfaceContainer;
        s.surfaceContainerHigh = mdSurfaceContainerHigh;
        s.surfaceContainerHighest = mdSurfaceContainerHighest;
        s.surfaceContainerLow = mdSurfaceContainerLow;
        s.surfaceContainerLowest = mdSurfaceContainerLowest;
        s.surfaceDim = mdSurfaceDim;
        s.surfaceVariant = mdSurfaceVariant;
        s.tertiary = mdTertiary;
        s.tertiaryContainer = mdTertiaryContainer;
        s.tertiaryFixed = mdTertiaryFixed;
        s.tertiaryFixedDim = mdTertiaryFixedDim;

        return s;
    };

    auto addSchemeVars = [&](const QString &suffix, const Md3Scheme &s) {
        auto add = [&](const QString &name, const QColor &c) {
            QString baseName = "MD_SYS_COLOR_" + name + suffix;
            v[baseName] = colorHex(c);
            v[baseName + "_RGB"] = colorRgb(c);
            v[baseName + "_RGB_SPACE"] = colorRgbSpace(c);
        };

        add("BACKGROUND", s.background);
        add("ERROR", s.error);
        add("ERROR_CONTAINER", s.errorContainer);
        add("INVERSE_ON_SURFACE", s.inverseOnSurface);
        add("INVERSE_PRIMARY", s.inversePrimary);
        add("INVERSE_SURFACE", s.inverseSurface);
        add("ON_BACKGROUND", s.onBackground);
        add("ON_ERROR", s.onError);
        add("ON_ERROR_CONTAINER", s.onErrorContainer);
        add("ON_PRIMARY", s.onPrimary);
        add("ON_PRIMARY_CONTAINER", s.onPrimaryContainer);
        add("ON_PRIMARY_FIXED", s.onPrimaryFixed);
        add("ON_PRIMARY_FIXED_VARIANT", s.onPrimaryFixedVariant);
        add("ON_SECONDARY", s.onSecondary);
        add("ON_SECONDARY_CONTAINER", s.onSecondaryContainer);
        add("ON_SECONDARY_FIXED", s.onSecondaryFixed);
        add("ON_SECONDARY_FIXED_VARIANT", s.onSecondaryFixedVariant);
        add("ON_SURFACE", s.onSurface);
        add("ON_SURFACE_VARIANT", s.onSurfaceVariant);
        add("ON_TERTIARY", s.onTertiary);
        add("ON_TERTIARY_CONTAINER", s.onTertiaryContainer);
        add("ON_TERTIARY_FIXED", s.onTertiaryFixed);
        add("ON_TERTIARY_FIXED_VARIANT", s.onTertiaryFixedVariant);
        add("OUTLINE", s.outline);
        add("OUTLINE_VARIANT", s.outlineVariant);
        add("PRIMARY", s.primary);
        add("PRIMARY_CONTAINER", s.primaryContainer);
        add("PRIMARY_FIXED", s.primaryFixed);
        add("PRIMARY_FIXED_DIM", s.primaryFixedDim);
        add("SCRIM", s.scrim);
        add("SECONDARY", s.secondary);
        add("SECONDARY_CONTAINER", s.secondaryContainer);
        add("SECONDARY_FIXED", s.secondaryFixed);
        add("SECONDARY_FIXED_DIM", s.secondaryFixedDim);
        add("SHADOW", s.shadow);
        add("SOURCE_COLOR", s.primary);
        add("SURFACE", s.surface);
        add("SURFACE_BRIGHT", s.surfaceBright);
        add("SURFACE_CONTAINER", s.surfaceContainer);
        add("SURFACE_CONTAINER_HIGH", s.surfaceContainerHigh);
        add("SURFACE_CONTAINER_HIGHEST", s.surfaceContainerHighest);
        add("SURFACE_CONTAINER_LOW", s.surfaceContainerLow);
        add("SURFACE_CONTAINER_LOWEST", s.surfaceContainerLowest);
        add("SURFACE_DIM", s.surfaceDim);
        add("SURFACE_TINT", s.primary);
        add("SURFACE_VARIANT", s.surfaceVariant);
        add("TERTIARY", s.tertiary);
        add("TERTIARY_CONTAINER", s.tertiaryContainer);
        add("TERTIARY_FIXED", s.tertiaryFixed);
        add("TERTIARY_FIXED_DIM", s.tertiaryFixedDim);
    };

    bool isDark = (p.windowBg.lightness() < 128);
    Md3Scheme currentScheme = generateMd3Scheme(isDark);
    Md3Scheme lightScheme = generateMd3Scheme(false);
    Md3Scheme darkScheme = generateMd3Scheme(true);

    addSchemeVars("", currentScheme);
    addSchemeVars("_LIGHT", lightScheme);
    addSchemeVars("_DARK", darkScheme);

    // Basic palette colors and ANSI colors in _LIGHT and _DARK versions
    auto adjustForDarkness = [](const QColor &c, bool targetDark) -> QColor {
        bool originalIsDark = (c.lightness() < 128);
        if (originalIsDark == targetDark) return c;
        return targetDark ? c.lighter(120) : c.darker(120);
    };

    auto addAnsiAndSemanticVars = [&](const QString &suffix, bool targetDark) {
        auto add = [&](const QString &name, const QColor &c) {
            QColor adjusted = adjustForDarkness(c, targetDark);
            QString baseName = name + suffix;
            v[baseName] = colorHex(adjusted);
            v[baseName + "_RGB"] = colorRgb(adjusted);
            v[baseName + "_RGB_SPACE"] = colorRgbSpace(adjusted);
        };

        add("SUCCESS", p.success);
        add("WARNING", p.warning);
        add("ERROR", p.error);
        add("ACCENT", p.accent);
        add("SELECTION", p.selection);
        add("TITLEBAR_BG", p.titleBarBg);
        add("TITLEBAR_FG", p.titleBarFg);
        add("BUTTON_BG", p.buttonBg);
        add("BUTTON_FG", p.buttonFg);

        add("ANSI_BLACK", p.ansiBlack);
        add("ANSI_RED", p.ansiRed);
        add("ANSI_GREEN", p.ansiGreen);
        add("ANSI_YELLOW", p.ansiYellow);
        add("ANSI_BLUE", p.ansiBlue);
        add("ANSI_MAGENTA", p.ansiMagenta);
        add("ANSI_CYAN", p.ansiCyan);
        add("ANSI_WHITE", p.ansiWhite);

        add("ANSI_BLACK_BRIGHT", p.ansiBlackBright);
        add("ANSI_RED_BRIGHT", p.ansiRedBright);
        add("ANSI_GREEN_BRIGHT", p.ansiGreenBright);
        add("ANSI_YELLOW_BRIGHT", p.ansiYellowBright);
        add("ANSI_BLUE_BRIGHT", p.ansiBlueBright);
        add("ANSI_MAGENTA_BRIGHT", p.ansiMagentaBright);
        add("ANSI_CYAN_BRIGHT", p.ansiCyanBright);
        add("ANSI_WHITE_BRIGHT", p.ansiWhiteBright);
    };

    addAnsiAndSemanticVars("_LIGHT", false);
    addAnsiAndSemanticVars("_DARK", true);

    // Semantic helper
    v["IS_DARK"] = (p.windowBg.lightness() < 128) ? "dark" : "light";
    v["VARIANT"]  = v["IS_DARK"];
    v["INHERITS"] = "vicinae-" + v["IS_DARK"];

    return v;
}

QString TemplateEngine::render(const QString &templatePath,
                                const UniversalPalette &palette,
                                const QMap<QString, QString> &extra) {
    QFile f(templatePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        Logger::log("TemplateEngine: Cannot open template: " + templatePath,
                    Logger::Error);
        return {};
    }
    QTextStream in(&f);
    QString tmpl = in.readAll();
    f.close();

    QMap<QString, QString> vars = buildVars(palette);
    // Extra values override / extend (allows app-specific tokens like {{IMPORTS}})
    for (auto it = extra.cbegin(); it != extra.cend(); ++it) {
        vars[it.key()] = it.value();
    }

    return substitute(tmpl, vars);
}
