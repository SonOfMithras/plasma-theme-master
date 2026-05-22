#ifndef SOLAR_H
#define SOLAR_H

#include <QDateTime>
#include <QPair>

class Solar {
public:
    // Returns pair of <Sunrise, Sunset> in UTC
    static QPair<QDateTime, QDateTime> calculateSunTimes(double lat, double lon, const QDate &date);
    
    // Convenience function to check if it's currently daytime at the given location
    // dayOffsetMinutes: shift sunrise earlier (positive) or later (negative)
    // nightOffsetMinutes: shift sunset later (positive) or earlier (negative)
    static bool isDaytime(double lat, double lon, int dayOffsetMinutes = 0, int nightOffsetMinutes = 0);
};

#endif // SOLAR_H
