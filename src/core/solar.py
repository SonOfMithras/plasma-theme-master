import math
import datetime

# --- Solar Calculation Utils ---

def rad(deg):
    return deg * math.pi / 180.0

def deg(rad):
    return rad * 180.0 / math.pi

def get_solar_times(date, lat, lon, zenith=90.8333):
    """
    Calculate sunrise and sunset times for a given date, lat, lon.
    Returns datetime objects in UTC.
    Algorithm source: Almanac for Computers, 1990
    published by Nautical Almanac Office, US Naval Observatory
    """
    # 1. first the day of the year
    try:
        N = date.timetuple().tm_yday
    except AttributeError:
        # fallback if passed datetime instead of date
        N = date.timetuple().tm_yday

    # 2. convert the longitude to hour value and calculate an approximate time
    lngHour = lon / 15.0
    
    times = {}
    for event in ['sunrise', 'sunset']:
        if event == 'sunrise':
            t = N + ((6.0 - lngHour) / 24.0)
        else:
            t = N + ((18.0 - lngHour) / 24.0)

        # 3. calculate the Sun's mean anomaly
        M = (0.9856 * t) - 3.289

        # 4. calculate the Sun's true longitude
        L = M + (1.916 * math.sin(rad(M))) + (0.020 * math.sin(rad(2 * M))) + 282.634
        L = L % 360.0

        # 5a. calculate the Sun's right ascension
        RA = deg(math.atan(0.91764 * math.tan(rad(L))))
        RA = RA % 360.0

        # 5b. right ascension value needs to be in the same quadrant as L
        Lquadrant  = (math.floor(L/90.0)) * 90.0
        RAquadrant = (math.floor(RA/90.0)) * 90.0
        RA = RA + (Lquadrant - RAquadrant)

        # 5c. right ascension value needs to be converted into hours
        RA = RA / 15.0

        # 6. calculate the Sun's declination
        sinDec = 0.39782 * math.sin(rad(L))
        cosDec = math.cos(math.asin(sinDec))

        # 7a. calculate the Sun's local hour angle
        cosH = (math.cos(rad(zenith)) - (sinDec * math.sin(rad(lat)))) / (cosDec * math.cos(rad(lat)))

        if cosH >  1:
            return None # The sun never rises on this location (on the specified date)
        if cosH < -1:
            return None # The sun never sets on this location (on the specified date)

        # 7b. finish calculating H and convert into hours
        if event == 'sunrise':
            H = 360.0 - deg(math.acos(cosH))
        else:
            H = deg(math.acos(cosH))
        
        H = H / 15.0

        # 8. calculate local mean time of rising/setting
        T = H + RA - (0.06571 * t) - 6.622

        # 9. adjust back to UTC
        UT = T - lngHour
        UT = UT % 24.0
        
        # Convert to datetime (UTC)
        total_seconds = int(UT * 3600)
        hours = total_seconds // 3600
        minutes = (total_seconds % 3600) // 60
        seconds = total_seconds % 60
        
        dt_utc = datetime.datetime(date.year, date.month, date.day, hours, minutes, seconds, tzinfo=datetime.timezone.utc)
        times[event] = dt_utc

    return times
