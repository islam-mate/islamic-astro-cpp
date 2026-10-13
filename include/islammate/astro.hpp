/**
 * @file astro.hpp
 * @author IslamMate Community
 * @brief High-precision, zero-dependency modern C++ library for Islamic astronomical calculations.
 * @version 1.0.0
 * 
 * Provides offline mathematical calculations for Islamic prayer times (Fajr, Sunrise,
 * Dhuhr, Asr, Maghrib, Isha) and Qibla direction/distance to the Holy Kaaba.
 * Suitable for embedded microcontrollers (ESP32, STM32, Arduino), IoT azan clocks,
 * game engines (Unreal, Godot), and high-performance server backends.
 */

#ifndef ISLAMMATE_ASTRO_HPP
#define ISLAMMATE_ASTRO_HPP

#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace islammate {
namespace astro {

/**
 * @brief Coordinates of the Holy Kaaba in Mecca, Saudi Arabia.
 */
inline constexpr double KAABA_LATITUDE = 21.4225;
inline constexpr double KAABA_LONGITUDE = 39.8262;
inline constexpr double EARTH_RADIUS_KM = 6371.0;
inline constexpr double PI = 3.14159265358979323846;

/**
 * @brief Recognized astronomical calculation conventions across the Islamic world.
 */
enum class CalculationMethod {
    EGYPT,   /**< Egyptian General Authority of Survey (Fajr 19.5°, Isha 17.5°) */
    MWL,     /**< Muslim World League (Fajr 18.0°, Isha 17.0°) */
    ISNA,    /**< Islamic Society of North America (Fajr 15.0°, Isha 15.0°) */
    MAKKAH,  /**< Umm al-Qura University, Makkah (Fajr 18.5°, Isha 90min after Maghrib) */
    KARACHI, /**< University of Islamic Sciences, Karachi (Fajr 18.0°, Isha 18.0°) */
    TEHRAN,  /**< Institute of Geophysics, University of Tehran (Fajr 17.7°, Maghrib 4.5°, Isha 14.0°) */
    JAFARI   /**< Shia Ithna Ashari, Leva Institute, Qum (Fajr 16.0°, Maghrib 4.0°, Isha 14.0°) */
};

/**
 * @brief Juristic school for Asr prayer shadow factor.
 */
enum class AsrMethod {
    STANDARD, /**< Shafi'i, Maliki, Hanbali (Shadow length = object height) */
    HANAFI    /**< Hanafi school (Shadow length = 2x object height) */
};

/**
 * @brief Geographic coordinate representation.
 */
struct Coordinates {
    double latitude;  /**< Latitude in degrees [-90, +90] */
    double longitude; /**< Longitude in degrees [-180, +180] */

    constexpr Coordinates(double lat, double lng) noexcept
        : latitude(lat), longitude(lng) {}
};

/**
 * @brief Container holding the 6 computed daily solar times (hours in decimal: 0.0 - 24.0).
 */
struct PrayerTimes {
    double fajr;     /**< Fajr dawn time in local decimal hours (e.g. 5.5 = 05:30) */
    double sunrise;  /**< Sunrise time */
    double dhuhr;    /**< Solar noon / Dhuhr time */
    double asr;      /**< Asr afternoon time */
    double maghrib;  /**< Sunset / Maghrib time */
    double isha;     /**< Night / Isha time */

    /**
     * @brief Helper to convert decimal hours into a formatted "HH:MM" string.
     * @param decimalHours Time in decimal hours (0.0 - 24.0).
     * @return Formatted string "HH:MM".
     */
    static std::string formatTime(double decimalHours) {
        if (std::isnan(decimalHours) || decimalHours < 0.0) {
            return "--:--";
        }
        int totalSeconds = static_cast<int>(std::round(decimalHours * 3600.0));
        int hours = (totalSeconds / 3600) % 24;
        int minutes = (totalSeconds % 3600) / 60;

        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << hours << ":"
            << std::setfill('0') << std::setw(2) << minutes;
        return oss.str();
    }
};

/**
 * @brief Qibla navigation data containing forward azimuth bearing and distance.
 */
struct QiblaResult {
    double bearing;         /**< Azimuth angle in degrees from True North [0°, 360°] */
    const char* direction;  /**< 8-point cardinal direction string (e.g. "SE", "NW") */
    double distanceKm;      /**< Great-circle distance to Holy Kaaba in kilometers */
};

namespace detail {

    inline constexpr double deg2rad(double deg) noexcept {
        return deg * (PI / 180.0);
    }

    inline constexpr double rad2deg(double rad) noexcept {
        return rad * (180.0 / PI);
    }

    /**
     * @brief Solves Kepler's Equation M = E - e * sin(E) using Newton-Raphson iteration.
     * @param M Mean anomaly in radians.
     * @param e Orbital eccentricity.
     * @return Eccentric anomaly E in radians.
     */
    inline double keplerSolve(double M, double e) noexcept {
        double E = M;
        for (int i = 0; i < 100; ++i) {
            double delta = (E - e * std::sin(E) - M) / (1.0 - e * std::cos(E));
            E -= delta;
            if (std::abs(delta) < 1e-9) break;
        }
        return E;
    }

    /**
     * @brief Computes Equation of Time (EoT in hours) and Solar Declination (radians).
     * @param daysSinceEpoch Days elapsed since 2000-01-01 12:00:00 UTC.
     * @param[out] outEotHours Equation of time in decimal hours.
     * @param[out] outDeclRad Solar declination in radians.
     */
    inline void computeEotDecl(double daysSinceEpoch, double& outEotHours, double& outDeclRad) noexcept {
        double y100 = daysSinceEpoch / 36525.0;

        double e = 1.6709e-2 - 4.193e-5 * y100 - 1.26e-7 * std::pow(y100, 2);
        double lamP = deg2rad(282.93807 + 1.7195 * y100 + 3.025e-4 * std::pow(y100, 2));
        double epsilon = deg2rad(23.4393 - 0.013 * y100 - 2e-7 * std::pow(y100, 2) + 5e-7 * std::pow(y100, 3));

        double MD = 6.24004077;
        double TY = 365.2596358;
        double D = std::fmod(daysSinceEpoch, TY);
        double M = std::fmod(MD + (2.0 * PI * D) / TY, 2.0 * PI);
        if (M < 0.0) M += 2.0 * PI;

        double E = keplerSolve(M, e);
        double nu = std::acos((std::cos(E) - e) / (1.0 - e * std::cos(E)));
        if (E > PI) nu = 2.0 * PI - nu;

        double lam = std::fmod(nu + lamP, 2.0 * PI);
        if (lam < 0.0) lam += 2.0 * PI;

        double alpha;
        if (std::abs(std::cos(lam)) < 1e-9) {
            alpha = lam;
        } else {
            alpha = std::atan(std::cos(epsilon) * std::tan(lam));
            if (lam < PI / 2.0) {
                // Quadrant 1
            } else if (lam < PI * 1.5) {
                alpha += PI;
            } else {
                alpha += 2.0 * PI;
            }
        }

        double eotRad = M + lamP - alpha;
        if (eotRad > PI) eotRad -= 2.0 * PI;
        else if (eotRad < -PI) eotRad += 2.0 * PI;

        outEotHours = (eotRad / (2.0 * PI)) * 24.0;
        outDeclRad = std::asin(std::sin(epsilon) * std::sin(lam));
    }

    /**
     * @brief Computes hour angle delta in hours for a given solar altitude angle.
     */
    inline double hourAngleDelta(double altitudeRad, double declRad, double latRad) {
        double num = std::sin(altitudeRad) - std::sin(latRad) * std::sin(declRad);
        double den = std::cos(latRad) * std::cos(declRad);
        if (std::abs(den) < 1e-9) return 0.0;

        double cosH = num / den;
        if (cosH < -1.0 || cosH > 1.0) {
            return -1.0; // Sun does not reach altitude
        }
        return rad2deg(std::acos(cosH)) / 15.0;
    }

    /**
     * @brief Solar altitude for shadow factor calculation (Asr).
     */
    inline double asrAltitude(double shadowFactor, double declRad, double latRad) noexcept {
        double zenithShadow = std::abs(std::tan(latRad - declRad));
        return std::atan(1.0 / (shadowFactor + zenithShadow));
    }

    /**
     * @brief Helper to calculate Julian Day number from Gregorian date.
     */
    inline double julianDaysSince2000(int year, int month, int day) noexcept {
        if (month <= 2) {
            year -= 1;
            month += 12;
        }
        int A = year / 100;
        int B = 2 - A + (A / 4);
        double jd = std::floor(365.25 * (year + 4716)) +
                    std::floor(30.6001 * (month + 1)) +
                    day + B - 1524.5;
        // J2000.0 epoch = 2451545.0
        return jd - 2451545.0;
    }

} // namespace detail

/**
 * @brief Calculates Great-Circle forward azimuth bearing and distance to the Holy Kaaba.
 * 
 * @param latitude Geographic latitude in degrees [-90.0, +90.0].
 * @param longitude Geographic longitude in degrees [-180.0, +180.0].
 * @return QiblaResult struct containing bearing, 8-point direction, and distance in km.
 * 
 * @throws std::invalid_argument if coordinates are out of valid bounds.
 */
inline QiblaResult calculateQibla(double latitude, double longitude) {
    if (latitude < -90.0 || latitude > 90.0) {
        throw std::invalid_argument("Latitude must be between -90 and +90 degrees");
    }
    if (longitude < -180.0 || longitude > 180.0) {
        throw std::invalid_argument("Longitude must be between -180 and +180 degrees");
    }

    double lat1 = detail::deg2rad(latitude);
    double lat2 = detail::deg2rad(KAABA_LATITUDE);
    double dLng = detail::deg2rad(KAABA_LONGITUDE - longitude);

    double x = std::sin(dLng) * std::cos(lat2);
    double y = std::cos(lat1) * std::sin(lat2) - std::sin(lat1) * std::cos(lat2) * std::cos(dLng);

    double bearing = detail::rad2deg(std::atan2(x, y));
    bearing = std::fmod(bearing + 360.0, 360.0);

    // Haversine distance
    double dLat = detail::deg2rad(KAABA_LATITUDE - latitude);
    double a = std::pow(std::sin(dLat / 2.0), 2.0) +
               std::cos(lat1) * std::cos(lat2) * std::pow(std::sin(dLng / 2.0), 2.0);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(std::max(0.0, 1.0 - a)));
    double distanceKm = EARTH_RADIUS_KM * c;

    // 8-point Cardinal direction
    static const char* DIRECTIONS[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    int idx = static_cast<int>(std::round(bearing / 45.0)) % 8;

    return QiblaResult{bearing, DIRECTIONS[idx], distanceKm};
}

/**
 * @brief Computes daily Islamic prayer times for a given date, coordinates, and timezone.
 * 
 * @param year Gregorian year (e.g. 2026).
 * @param month Gregorian month (1 - 12).
 * @param day Gregorian day of month (1 - 31).
 * @param latitude Geographic latitude in degrees [-90.0, +90.0].
 * @param longitude Geographic longitude in degrees [-180.0, +180.0].
 * @param timezoneOffsetHours UTC timezone offset in hours (e.g. +3.0 for Cairo/Riyadh, +0.0 for UTC).
 * @param method Astronomical convention method (default: EGYPT).
 * @param asr Juristic school for Asr shadow length (default: STANDARD).
 * @param isRamadan Optional flag for Ramadan Isha extension in Makkah convention.
 * 
 * @return PrayerTimes struct containing the 6 solar times in local decimal hours.
 */
inline PrayerTimes calculatePrayerTimes(
    int year, int month, int day,
    double latitude, double longitude,
    double timezoneOffsetHours,
    CalculationMethod method = CalculationMethod::EGYPT,
    AsrMethod asr = AsrMethod::STANDARD,
    bool isRamadan = false
) {
    double daysSinceEpoch = detail::julianDaysSince2000(year, month, day);

    double eotHours = 0.0;
    double declRad = 0.0;
    detail::computeEotDecl(daysSinceEpoch, eotHours, declRad);

    double latRad = detail::deg2rad(latitude);

    // Solar noon (Dhuhr base) in local hours
    double dhuhr = 12.0 + timezoneOffsetHours - (longitude / 15.0) - eotHours;

    // Standard atmospheric refraction + solar semi-diameter = -0.833 degrees
    double sunsetAltRad = detail::deg2rad(-0.833);
    double sunriseDelta = detail::hourAngleDelta(sunsetAltRad, declRad, latRad);

    double sunrise = dhuhr - sunriseDelta;
    double maghrib = dhuhr + sunriseDelta;

    // Calculation Method Angles
    double fajrAngle = 19.5;
    double ishaAngle = 17.5;
    double ishaOffsetMinutes = -1.0;

    switch (method) {
        case CalculationMethod::MWL:
            fajrAngle = 18.0; ishaAngle = 17.0; break;
        case CalculationMethod::ISNA:
            fajrAngle = 15.0; ishaAngle = 15.0; break;
        case CalculationMethod::EGYPT:
            fajrAngle = 19.5; ishaAngle = 17.5; break;
        case CalculationMethod::KARACHI:
            fajrAngle = 18.0; ishaAngle = 18.0; break;
        case CalculationMethod::MAKKAH:
            fajrAngle = 18.5; ishaOffsetMinutes = isRamadan ? 120.0 : 90.0; break;
        case CalculationMethod::TEHRAN:
            fajrAngle = 17.7; ishaAngle = 14.0;
            maghrib = dhuhr + detail::hourAngleDelta(detail::deg2rad(-4.5), declRad, latRad);
            break;
        case CalculationMethod::JAFARI:
            fajrAngle = 16.0; ishaAngle = 14.0;
            maghrib = dhuhr + detail::hourAngleDelta(detail::deg2rad(-4.0), declRad, latRad);
            break;
    }

    double fajrDelta = detail::hourAngleDelta(detail::deg2rad(-fajrAngle), declRad, latRad);
    double fajr = dhuhr - fajrDelta;

    double isha;
    if (ishaOffsetMinutes > 0.0) {
        isha = maghrib + (ishaOffsetMinutes / 60.0);
    } else {
        double ishaDelta = detail::hourAngleDelta(detail::deg2rad(-ishaAngle), declRad, latRad);
        isha = dhuhr + ishaDelta;
    }

    // Asr calculation
    double shadowFactor = (asr == AsrMethod::HANAFI) ? 2.0 : 1.0;
    double asrAlt = detail::asrAltitude(shadowFactor, declRad, latRad);
    double asrDelta = detail::hourAngleDelta(asrAlt, declRad, latRad);
    double asrTime = dhuhr + asrDelta;

    return PrayerTimes{fajr, sunrise, dhuhr, asrTime, maghrib, isha};
}

} // namespace astro
} // namespace islammate

#endif // ISLAMMATE_ASTRO_HPP
