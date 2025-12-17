#ifndef SOLAR_H
#define SOLAR_H

#include <QDateTime>
#include <QPair>

class Solar {
public:
    // Returns pair of <Sunrise, Sunset> in UTC
    static QPair<QDateTime, QDateTime> calculateSunTimes(double lat, double lon, const QDate &date);
    
    // Convenience function to check if it's currently daytime at the given location
    // offsetMinutes: Padding to extend the day (e.g. 30 means day starts 15m earlier and ends 15m later)
    static bool isDaytime(double lat, double lon, int offsetMinutes = 0);
};

#endif // SOLAR_H
