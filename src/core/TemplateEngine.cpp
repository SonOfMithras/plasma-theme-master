#include "TemplateEngine.h"
#include "Logger.h"
#include <QColor>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>

// ---------------------------------------------------------------------------
// Material Design 3 Structure & Generator
// ---------------------------------------------------------------------------

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

static Md3Scheme generateMd3Scheme(const UniversalPalette &p, bool targetDark) {
    Md3Scheme s;

    auto desaturate = [](const QColor &c, double factor) -> QColor {
        int h, sat, val, a;
        c.getHsv(&h, &sat, &val, &a);
        return QColor::fromHsv(h, static_cast<int>(sat * factor), val, a);
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
    auto mixColors = [](const QColor &c1, const QColor &c2, double factor) -> QColor {
        return QColor::fromRgbF(
            c1.redF() * (1.0 - factor) + c2.redF() * factor,
            c1.greenF() * (1.0 - factor) + c2.greenF() * factor,
            c1.blueF() * (1.0 - factor) + c2.blueF() * factor,
            c1.alphaF() * (1.0 - factor) + c2.alphaF() * factor
        );
    };

    QColor mdOutline = mixColors(mdSurface, mdOnSurface, 0.5);
    QColor mdOutlineVariant = mixColors(mdSurface, mdOnSurface, 0.3);

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
}

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

QColor TemplateEngine::resolveColorToken(const QString &tokenName,
                                        const QString &mode,
                                        const UniversalPalette &palette) {
    bool isDark = (mode == "dark") ? true : ((mode == "light") ? false : (palette.windowBg.lightness() < 128));
    Md3Scheme s = generateMd3Scheme(palette, isDark);

    QString name = tokenName.toLower();

    // Material 3 Tokens
    if (name == "surface") return s.surface;
    if (name == "on_surface") return s.onSurface;
    if (name == "surface_variant") return s.surfaceVariant;
    if (name == "on_surface_variant") return s.onSurfaceVariant;
    if (name == "surface_container") return s.surfaceContainer;
    if (name == "surface_container_low") return s.surfaceContainerLow;
    if (name == "surface_container_lowest") return s.surfaceContainerLowest;
    if (name == "surface_container_high") return s.surfaceContainerHigh;
    if (name == "surface_container_highest") return s.surfaceContainerHighest;
    if (name == "surface_bright") return s.surfaceBright;
    if (name == "surface_dim") return s.surfaceDim;
    if (name == "primary") return s.primary;
    if (name == "on_primary") return s.onPrimary;
    if (name == "primary_container") return s.primaryContainer;
    if (name == "on_primary_container") return s.onPrimaryContainer;
    if (name == "secondary") return s.secondary;
    if (name == "on_secondary") return s.onSecondary;
    if (name == "secondary_container") return s.secondaryContainer;
    if (name == "on_secondary_container") return s.onSecondaryContainer;
    if (name == "tertiary") return s.tertiary;
    if (name == "on_tertiary") return s.onTertiary;
    if (name == "tertiary_container") return s.tertiaryContainer;
    if (name == "on_tertiary_container") return s.onTertiaryContainer;
    if (name == "error") return s.error;
    if (name == "on_error") return s.onError;
    if (name == "error_container") return s.errorContainer;
    if (name == "on_error_container") return s.onErrorContainer;
    if (name == "background") return s.background;
    if (name == "on_background") return s.onBackground;
    if (name == "inverse_surface") return s.inverseSurface;
    if (name == "inverse_on_surface") return s.inverseOnSurface;
    if (name == "inverse_primary") return s.inversePrimary;
    if (name == "outline") return s.outline;
    if (name == "outline_variant") return s.outlineVariant;
    if (name == "shadow") return s.shadow;
    if (name == "scrim") return s.scrim;

    if (name == "primary_fixed") return s.primaryFixed;
    if (name == "primary_fixed_dim") return s.primaryFixedDim;
    if (name == "on_primary_fixed") return s.onPrimaryFixed;
    if (name == "on_primary_fixed_variant") return s.onPrimaryFixedVariant;

    if (name == "secondary_fixed") return s.secondaryFixed;
    if (name == "secondary_fixed_dim") return s.secondaryFixedDim;
    if (name == "on_secondary_fixed") return s.onSecondaryFixed;
    if (name == "on_secondary_fixed_variant") return s.onSecondaryFixedVariant;

    if (name == "tertiary_fixed") return s.tertiaryFixed;
    if (name == "tertiary_fixed_dim") return s.tertiaryFixedDim;
    if (name == "on_tertiary_fixed") return s.onTertiaryFixed;
    if (name == "on_tertiary_fixed_variant") return s.onTertiaryFixedVariant;

    if (name == "surface_tint" || name == "source_color" || name == "hover") return s.primary;
    if (name == "on_hover") return s.onPrimary;

    // Terminal ANSI tokens
    if (name == "terminal_normal_black") return palette.ansiBlack;
    if (name == "terminal_normal_red") return palette.ansiRed;
    if (name == "terminal_normal_green") return palette.ansiGreen;
    if (name == "terminal_normal_yellow") return palette.ansiYellow;
    if (name == "terminal_normal_blue") return palette.ansiBlue;
    if (name == "terminal_normal_magenta") return palette.ansiMagenta;
    if (name == "terminal_normal_cyan") return palette.ansiCyan;
    if (name == "terminal_normal_white") return palette.ansiWhite;

    if (name == "terminal_bright_black") return palette.ansiBlackBright;
    if (name == "terminal_bright_red") return palette.ansiRedBright;
    if (name == "terminal_bright_green") return palette.ansiGreenBright;
    if (name == "terminal_bright_yellow") return palette.ansiYellowBright;
    if (name == "terminal_bright_blue") return palette.ansiBlueBright;
    if (name == "terminal_bright_magenta") return palette.ansiMagentaBright;
    if (name == "terminal_bright_cyan") return palette.ansiCyanBright;
    if (name == "terminal_bright_white") return palette.ansiWhiteBright;

    // Terminal Semantic Tokens
    if (name == "terminal_background" || name == "terminal_bg") {
        if (palette.specialColors.contains("terminal_bg")) return QColor(palette.specialColors.value("terminal_bg"));
        if (palette.specialColors.contains("terminal_background")) return QColor(palette.specialColors.value("terminal_background"));
        return palette.windowBg;
    }
    if (name == "terminal_foreground" || name == "terminal_fg") {
        if (palette.specialColors.contains("terminal_fg")) return QColor(palette.specialColors.value("terminal_fg"));
        if (palette.specialColors.contains("terminal_foreground")) return QColor(palette.specialColors.value("terminal_foreground"));
        return palette.windowFg;
    }
    if (name == "terminal_cursor" || name == "cursor") {
        if (palette.specialColors.contains("cursor")) return QColor(palette.specialColors.value("cursor"));
        if (palette.specialColors.contains("terminal_cursor")) return QColor(palette.specialColors.value("terminal_cursor"));
        return palette.accent;
    }
    if (name == "terminal_cursor_text" || name == "cursor_text") {
        if (palette.specialColors.contains("cursor_text")) return QColor(palette.specialColors.value("cursor_text"));
        return palette.windowBg;
    }
    if (name == "terminal_selection_bg" || name == "selection_bg") {
        if (palette.specialColors.contains("terminal_selection_bg")) return QColor(palette.specialColors.value("terminal_selection_bg"));
        return palette.selection.isValid() ? palette.selection : palette.accent;
    }
    if (name == "terminal_selection_fg" || name == "selection_fg") {
        if (palette.specialColors.contains("terminal_selection_fg")) return QColor(palette.specialColors.value("terminal_selection_fg"));
        return palette.windowFg;
    }

    // Legacy semantic shortcuts
    if (name == "accent") return palette.accent;
    if (name == "window_bg") return palette.windowBg;
    if (name == "window_fg") return palette.windowFg;
    if (name == "view_bg") return palette.viewBg;
    if (name == "view_fg") return palette.viewFg;

    return palette.accent;
}

QString TemplateEngine::evaluateExpression(const QString &rawExpr,
                                           const UniversalPalette &palette,
                                           const QMap<QString, QString> &vars) {
    QString expr = rawExpr.trimmed();
    if (expr.isEmpty()) return {};

    // Check direct match in vars (e.g. config_dir, WINDOW_BG, IMPORTS)
    if (vars.contains(expr)) {
        return vars.value(expr);
    }

    // Split pipeline filters: "colors.primary.default.rgba | set_alpha 0.15"
    QStringList parts = expr.split('|');
    QString base = parts.first().trimmed();

    if (base.startsWith("colors.")) {
        QString remainder = base.mid(7);
        QStringList segments = remainder.split('.');
        QString tokenName = "surface";
        QString mode = "default";
        QString format = "hex";

        if (segments.size() >= 3) {
            tokenName = segments[0];
            mode = segments[1];
            format = segments[2];
        } else if (segments.size() == 2) {
            tokenName = segments[0];
            format = segments[1];
        } else if (segments.size() == 1) {
            tokenName = segments[0];
        }

        QColor c = resolveColorToken(tokenName, mode, palette);
        double alpha = 1.0;

        // Apply filters
        for (int i = 1; i < parts.size(); ++i) {
            QString filter = parts[i].trimmed();
            if (filter.startsWith("set_alpha")) {
                QString arg = filter.mid(9).trimmed();
                if (arg.startsWith(':')) arg = arg.mid(1).trimmed();
                double a = arg.toDouble();
                if (a > 1.0 && a <= 255.0) a /= 255.0;
                alpha = qBound(0.0, a, 1.0);
            } else if (filter.startsWith("lighten")) {
                QString arg = filter.mid(7).trimmed();
                if (arg.startsWith(':')) arg = arg.mid(1).trimmed();
                double val = arg.toDouble();
                int factor = 100 + (val <= 1.0 ? qRound(val * 100.0) : qRound(val * 10.0));
                c = c.lighter(factor);
            } else if (filter.startsWith("darken")) {
                QString arg = filter.mid(6).trimmed();
                if (arg.startsWith(':')) arg = arg.mid(1).trimmed();
                double val = arg.toDouble();
                int factor = 100 + (val <= 1.0 ? qRound(val * 100.0) : qRound(val * 10.0));
                c = c.darker(factor);
            } else if (filter.startsWith("rotate_hue")) {
                QString arg = filter.mid(10).trimmed();
                if (arg.startsWith(':')) arg = arg.mid(1).trimmed();
                int deg = arg.toInt();
                int h = (qMax(0, c.hslHue()) + deg + 360) % 360;
                c = QColor::fromHsl(h, c.hslSaturation(), c.lightness());
            } else if (filter == "hex_stripped") {
                format = "hex_stripped";
            }
        }

        // Output formatting
        if (format == "hex") {
            if (alpha < 0.999) {
                return QString("#%1%2%3%4")
                    .arg(c.red(), 2, 16, QChar('0'))
                    .arg(c.green(), 2, 16, QChar('0'))
                    .arg(c.blue(), 2, 16, QChar('0'))
                    .arg(qBound(0, qRound(alpha * 255.0), 255), 2, 16, QChar('0'));
            }
            return c.name();
        }
        if (format == "hex_stripped") {
            QString hexStr = c.name().mid(1);
            if (alpha < 0.999) {
                hexStr += QString("%1").arg(qBound(0, qRound(alpha * 255.0), 255), 2, 16, QChar('0'));
            }
            return hexStr;
        }
        if (format == "rgb") {
            return QString("rgb(%1, %2, %3)").arg(c.red()).arg(c.green()).arg(c.blue());
        }
        if (format == "rgb_csv") {
            return QString("%1, %2, %3").arg(c.red()).arg(c.green()).arg(c.blue());
        }
        if (format == "rgba") {
            return QString("rgba(%1, %2, %3, %4)")
                .arg(c.red()).arg(c.green()).arg(c.blue())
                .arg(alpha, 0, 'f', (alpha == 1.0 || alpha == 0.0 ? 1 : 2));
        }
        if (format == "hsl") {
            return QString("hsl(%1, %2%, %3%)")
                .arg(qMax(0, c.hslHue()))
                .arg(qRound(c.hslSaturationF() * 100))
                .arg(qRound(c.lightnessF() * 100));
        }
        if (format == "hsla") {
            return QString("hsla(%1, %2%, %3%, %4)")
                .arg(qMax(0, c.hslHue()))
                .arg(qRound(c.hslSaturationF() * 100))
                .arg(qRound(c.lightnessF() * 100))
                .arg(alpha, 0, 'f', 2);
        }
        if (format == "red") return QString::number(c.red());
        if (format == "green") return QString::number(c.green());
        if (format == "blue") return QString::number(c.blue());
        if (format == "alpha") return QString::number(alpha, 'f', 2);

        return c.name();
    }

    if (base.startsWith("palettes.")) {
        QString remainder = base.mid(9);
        QStringList segments = remainder.split('.');
        QString group = segments.value(0, "primary");
        int tone = segments.value(1, "50").toInt();
        QString format = segments.value(2, "hex");

        QColor baseColor = palette.accent;
        if (group == "primary") baseColor = palette.accent;
        else if (group == "secondary") baseColor = palette.selection.isValid() ? palette.selection : palette.accent;
        else if (group == "tertiary") baseColor = palette.ansiCyan;
        else if (group == "error") baseColor = palette.error;
        else if (group == "neutral" || group == "neutral_variant") baseColor = palette.windowBg;

        int lightness = qBound(0, qRound(tone * 2.55), 255);
        QColor c = QColor::fromHsl(qMax(0, baseColor.hslHue()), baseColor.hslSaturation(), lightness);
        if (format == "hex") return c.name();
        if (format == "hex_stripped") return c.name().mid(1);
        if (format == "rgb") return QString("rgb(%1, %2, %3)").arg(c.red()).arg(c.green()).arg(c.blue());
        return c.name();
    }

    // Fallback: check vars for base
    if (vars.contains(base)) {
        return vars.value(base);
    }

    return QString();
}

QString TemplateEngine::substitute(const QString &tmpl,
                                    const UniversalPalette &palette,
                                    const QMap<QString, QString> &vars) {
    static const QRegularExpression re(QStringLiteral(R"(\{\{\s*([^{}]+?)\s*\}\})"));
    QString result = tmpl;

    // Fast path: replace existing exact variables first if present
    for (auto it = vars.cbegin(); it != vars.cend(); ++it) {
        result.replace("{{" + it.key() + "}}", it.value());
    }

    // Full regex pass for Noctalia tokens, whitespace-padded tags, and filter pipelines
    QRegularExpressionMatchIterator it = re.globalMatch(result);
    int offset = 0;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString inner = match.captured(1);
        QString replacement = evaluateExpression(inner, palette, vars);
        if (!replacement.isEmpty()) {
            int start = match.capturedStart() + offset;
            int len = match.capturedLength();
            result.replace(start, len, replacement);
            offset += replacement.length() - len;
        }
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
    auto genScheme = [&](bool targetDark) -> Md3Scheme {
        return generateMd3Scheme(p, targetDark);
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
    Md3Scheme currentScheme = genScheme(isDark);
    Md3Scheme lightScheme = genScheme(false);
    Md3Scheme darkScheme = genScheme(true);

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
    v["is_dark"]  = v["IS_DARK"];
    v["theme_type"] = v["IS_DARK"];
    v["VARIANT"]  = v["IS_DARK"];
    v["INHERITS"] = "vicinae-" + v["IS_DARK"];
    v["mode"]     = v["IS_DARK"];
    v["MODE"]     = v["IS_DARK"];

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

    QFileInfo fi(templatePath);
    QMap<QString, QString> mergedExtra = extra;
    if (!mergedExtra.contains("config_dir")) {
        mergedExtra["config_dir"] = fi.absolutePath();
    }

    return renderString(tmpl, palette, mergedExtra);
}

QString TemplateEngine::renderString(const QString &templateContent,
                                      const UniversalPalette &palette,
                                      const QMap<QString, QString> &extra) {
    QMap<QString, QString> vars = buildVars(palette);
    // Extra values override / extend
    for (auto it = extra.cbegin(); it != extra.cend(); ++it) {
        vars[it.key()] = it.value();
    }

    return substitute(templateContent, palette, vars);
}
