#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

class Config {
public:
    static double latitude();
    static double longitude();
    static void setLatitude(double val);
    static void setLongitude(double val);
};

#endif // CONFIG_H
