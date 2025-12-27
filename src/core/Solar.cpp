#include "Solar.h"
#include <QtMath>
#include <cmath>
#include <QDebug>
#include <QTimeZone>

// Helpers
static double rad(double deg) {
    return deg * M_PI / 180.0;
}

static double deg(double rad) {
    return rad * 180.0 / M_PI;
}

static double wrap360(double val) {
    val = fmod(val, 360.0);
    if (val < 0) val += 360.0;
    return val;
}

QPair<QDateTime, QDateTime> Solar::calculateSunTimes(double lat, double lon, const QDate &date) {
    // Zenith: Official = 90 degrees 50' = 90.8333
    const double zenith = 90.8333;
    
    int N = date.dayOfYear();
    
    // 2. Convert longitude to hour value and calculate approximate time
    double lngHour = lon / 15.0;
    
    QDateTime sunrise;
    QDateTime sunset;
    
    const QStringList events = { "sunrise", "sunset" };
    
    for (const QString &event : events) {
        double t;
        if (event == "sunrise") {
            t = N + ((6.0 - lngHour) / 24.0);
        } else {
            t = N + ((18.0 - lngHour) / 24.0);
        }
        
        // 3. Sun's mean anomaly
        double M = (0.9856 * t) - 3.289;
        
        // 4. Sun's true longitude
        double L = M + (1.916 * std::sin(rad(M))) + (0.020 * std::sin(rad(2 * M))) + 282.634;
        L = wrap360(L);
        
        // 5a. Sun's right ascension
        double RA = deg(std::atan(0.91764 * std::tan(rad(L))));
        RA = wrap360(RA);
        
        // 5b. RA value needs to be in the same quadrant as L
        double Lquadrant = std::floor(L / 90.0) * 90.0;
        double RAquadrant = std::floor(RA / 90.0) * 90.0;
        RA = RA + (Lquadrant - RAquadrant);
        
        // 5c. RA value needs to be converted into hours
        RA = RA / 15.0;
        
        // 6. Sun's declination
        double sinDec = 0.39782 * std::sin(rad(L));
        double cosDec = std::cos(std::asin(sinDec));
        
        // 7a. Sun's local hour angle
        double cosH = (std::cos(rad(zenith)) - (sinDec * std::sin(rad(lat)))) / (cosDec * std::cos(rad(lat)));
        
        if (cosH > 1) {
            // Sun never rises
            return qMakePair(QDateTime(), QDateTime());
        }
        if (cosH < -1) {
            // Sun never sets
            return qMakePair(QDateTime(), QDateTime());
        }
        
        // 7b. Finish calculating H and convert into hours
        double H;
        if (event == "sunrise") {
            H = 360.0 - deg(std::acos(cosH));
        } else {
            H = deg(std::acos(cosH));
        }
        
        H = H / 15.0;
        
        // 8. Calculate local mean time of rising/setting
        double T = H + RA - (0.06571 * t) - 6.622;
        
        // 9. Adjust back to UTC
        double UT = T - lngHour;
        UT = fmod(UT, 24.0);
        if (UT < 0) UT += 24.0;
        
        // Convert to QDateTime (UTC)
        int totalSeconds = static_cast<int>(UT * 3600.0);
        QTime time(0, 0, 0);
        time = time.addSecs(totalSeconds);
        
        QDateTime dt(date, time, QTimeZone::utc());
        
        if (event == "sunrise") sunrise = dt;
        else sunset = dt;
    }
    
    return qMakePair(sunrise, sunset);
}

bool Solar::isDaytime(double lat, double lon, int offsetMinutes) {
    QDateTime now = QDateTime::currentDateTime().toUTC();
    QPair<QDateTime, QDateTime> times = calculateSunTimes(lat, lon, now.date());
    
    if (!times.first.isValid() || !times.second.isValid()) {
        qWarning() << "Solar calc failed (Polar region?), defaulting to Day.";
        return true; 
    }
    
    QDateTime sunrise = times.first;
    QDateTime sunset = times.second;
    
    // Apply padding/offset
    // Offset is total expansion of day. so half at start, half at end.
    // e.g. 30 mins -> -15 at sunrise, +15 at sunset.
    int shiftSecs = (offsetMinutes * 60) / 2;
    sunrise = sunrise.addSecs(-shiftSecs);
    sunset = sunset.addSecs(shiftSecs);
    
    return (now >= sunrise && now < sunset);
}
