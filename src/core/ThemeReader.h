#ifndef THEME_READER_H
#define THEME_READER_H

#include <QString>
#include <QStringList> // Add include for QStringList

class ThemeReader {
public:
    static QString currentGlobalTheme();
    static QString currentKvantumTheme();
    static QString currentGtkTheme();
    static bool isAutoLookAndFeel();
    
    // Native Day-Night Location
    static double nativeLatitude();
    static double nativeLongitude();

    // Listings
    static QStringList listKvantumThemes();
    static QStringList listGlobalThemes();
    static QStringList listGtkThemes();    
    
    // Configured Defaults
    static QString defaultDarkTheme();
    static QString defaultLightTheme();
    
    // App Config (plasma-theme-masterrc)
    static int solarPadding();
    static QString dayKvantumTheme();
    static QString nightKvantumTheme();
    static QString dayGtkTheme();
    static QString nightGtkTheme();
};

#endif // THEME_READER_H
