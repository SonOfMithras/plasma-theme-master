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
