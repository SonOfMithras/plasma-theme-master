#ifndef UNIVERSALTHEMEEXPORTER_H
#define UNIVERSALTHEMEEXPORTER_H

#include <QString>
#include <QColor>
#include <QMap>
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

    QMap<QString, QString> specialColors; // For other specific roles
};

class UniversalThemeExporter {
public:
    static UniversalPalette extractColors();
    static UniversalPalette extractColors(const QString &configPath);
    static void syncAll();
    
    // Exporters
    static bool exportToVSCode(const UniversalPalette &palette);
    static bool exportToVSCodeJSON(const QString &path, const UniversalPalette &palette);
    static bool exportToFirefox(const UniversalPalette &palette);
    static bool exportToBetterDiscord(const UniversalPalette &palette);
    static bool exportToObsidian(const UniversalPalette &palette, const QString &vaultPath);
    static bool exportToKitty(const UniversalPalette &palette);
    static bool exportToKonsole(const UniversalPalette &palette);
    static bool exportToVencord(const UniversalPalette &palette);
    static bool exportToBtop(const UniversalPalette &palette);
    static bool exportToVicinae(const UniversalPalette &palette);
    
    // Restore Methods
    static bool restoreVSCode();
    static bool restoreFirefox();
    static bool restoreBetterDiscord();
    static bool restoreKitty();
    static bool restoreKonsole();
    static bool restoreObsidian();
    static bool restoreVencord();
    static bool restoreBtop();
    static bool restoreVicinae();

    // Helpers
    static QStringList scanBetterDiscordImports();
    static QStringList scanVencordImports();

    static bool backupFile(const QString &path);
    static bool restoreFile(const QString &path);

private:
    static QString colorToHex(const QColor &color);
    static QString colorToRgb(const QColor &color); // "r, g, b"
    static bool writeToFile(const QString &path, const QString &content);
    
    // Internal helper
    static UniversalPalette extractColorsFromConfig(KSharedConfig::Ptr config);
};

#endif // UNIVERSALTHEMEEXPORTER_H
