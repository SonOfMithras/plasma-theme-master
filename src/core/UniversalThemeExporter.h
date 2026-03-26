#ifndef UNIVERSALTHEMEEXPORTER_H
#define UNIVERSALTHEMEEXPORTER_H

#include <QColor>
#include <QMap>
#include <QString>
#include <KSharedConfig>

struct UniversalPalette {
    QColor windowBg;
    QColor windowFg;
    QColor viewBg;
    QColor viewFg;
    QColor accent;
    QColor selection;
    QColor success;
    QColor warning;
    QColor error;

    // Derived/Extra
    QColor titleBarBg;
    QColor titleBarFg;
    QColor buttonBg;
    QColor buttonFg;

    // ANSI Colors (Terminal / Code)
    QColor ansiBlack;
    QColor ansiRed;
    QColor ansiGreen;
    QColor ansiYellow;
    QColor ansiBlue;
    QColor ansiMagenta;
    QColor ansiCyan;
    QColor ansiWhite;

    QColor ansiBlackBright;
    QColor ansiRedBright;
    QColor ansiGreenBright;
    QColor ansiYellowBright;
    QColor ansiBlueBright;
    QColor ansiMagentaBright;
    QColor ansiCyanBright;
    QColor ansiWhiteBright;

    QMap<QString, QString> specialColors;
};

class UniversalThemeExporter {
public:
    // Color extraction
    static UniversalPalette extractColors();
    static UniversalPalette extractColors(const QString &configPath);

    // Main sync entry point — drives all template rendering
    static void syncAll();
    static void syncTemplates();

    // Backup / restore utilities (still used for legacy cleanup on restore btn)
    static bool backupFile(const QString &path);
    static bool restoreFile(const QString &path);

private:
    static UniversalPalette extractColorsFromConfig(KSharedConfig::Ptr config);

    // Environment variable injection for helper binaries
    static void injectPaletteEnv(const UniversalPalette &palette);

    // Helpers for syncTemplates
    static QString buildImports(const QString &appName);
    static QMap<QString, QString> buildZedExtras(const UniversalPalette &palette);
    static UniversalPalette paletteForEntry(const QString &paletteMode);
};

#endif // UNIVERSALTHEMEEXPORTER_H
