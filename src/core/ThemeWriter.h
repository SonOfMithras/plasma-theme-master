#ifndef THEME_WRITER_H
#define THEME_WRITER_H

#include <QString>

class ThemeWriter {
public:
    static void setAutoLookAndFeel(bool enabled);
    static bool setKvantumTheme(const QString &themeName);
    static bool setGtkTheme(const QString &themeName);
    
    // Global Theme Setters
    static void setDefaultDarkTheme(const QString &themeName);
    static void setDefaultLightTheme(const QString &themeName);
    static bool applyGlobalTheme(const QString &themeName);
    
    // App Config (plasma-theme-masterrc)
    static void setSolarPadding(int minutes);
    static void setDayKvantumTheme(const QString &themeName);
    static void setNightKvantumTheme(const QString &themeName);
    static void setDayGtkTheme(const QString &themeName);
    static void setNightGtkTheme(const QString &themeName);
};

#endif // THEME_WRITER_H
