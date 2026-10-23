#ifndef PALETTECATALOG_H
#define PALETTECATALOG_H

#include <QString>
#include <QStringList>
#include <QColor>
#include <QList>
#include <QMap>
#include "UniversalThemeExporter.h"

struct PaletteModeColors {
    QColor primary;
    QColor onPrimary;
    QColor secondary;
    QColor onSecondary;
    QColor tertiary;
    QColor onTertiary;
    QColor error;
    QColor onError;
    QColor surface;
    QColor onSurface;
    QColor surfaceVariant;
    QColor onSurfaceVariant;
    QColor outline;
    QColor shadow;
    QColor hover;
    QColor onHover;

    // Terminal colors
    QColor terminalFg;
    QColor terminalBg;
    QColor terminalSelectionFg;
    QColor terminalSelectionBg;
    QColor terminalCursor;
    QColor terminalCursorText;
    QColor terminalNormal[8];
    QColor terminalBright[8];
};

struct ThemePalette {
    QString id;
    QString name;
    bool isOfficial = false;
    bool isUser = false;
    PaletteModeColors dark;
    PaletteModeColors light;

    UniversalPalette toUniversalPalette(bool isDark) const;
};

class PaletteCatalog {
public:
    static PaletteCatalog &instance();

    void reload();

    const QList<ThemePalette> &palettes() const { return m_palettes; }
    const ThemePalette *findPalette(const QString &nameOrId) const;

    int bundledCount() const;
    int userCount() const;

    QString activePaletteName() const;
    void setActivePaletteName(const QString &name);

    // Applies this palette to Plasma (generates ptm.colors) and triggers template sync
    bool applyPalette(const QString &paletteName, const QString &mode = "current", bool syncTemplates = true);

    // Applies active palette for day (light) or night (dark)
    bool applyActivePalette(bool isDay, bool syncTemplates = true);

    static QStringList paletteSearchPaths();

private:
    PaletteCatalog();
    void loadOfficialPalettes(const QString &filePath);
    void loadCommunityPalettes(const QString &filePath);
    void loadUserPalettes(const QString &dirPath);

    QList<ThemePalette> m_palettes;
    QMap<QString, int> m_lookup;
};

#endif // PALETTECATALOG_H
