#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

class Config {
public:
    static double latitude();
    static double longitude();
    static void setLatitude(double val);
    static void setLongitude(double val);

    // Material You
    static bool isMaterialYouOverrideEnabled();
    static void setMaterialYouOverrideEnabled(bool enabled);

    static double materialYouChroma();
    static void setMaterialYouChroma(double chroma);

    static double materialYouTone();
    static void setMaterialYouTone(double tone);

    static int materialYouSchemeVariant();
    static void setMaterialYouSchemeVariant(int variant);

    static int materialYouNColor();
    static void setMaterialYouNColor(int ncolor);
};

#endif // CONFIG_H
