/**
 * @file test_main.cpp
 * @brief Unit verification test for islammate-astro-cpp.
 */

#include <iostream>
#include <cassert>
#include <islammate/astro.hpp>

int main() {
    using namespace islammate::astro;

    std::cout << "Testing islammate-astro-cpp..." << std::endl;

    // 1. Prayer Times calculation for Cairo (30.0444, 31.2357, UTC+3)
    PrayerTimes times = calculatePrayerTimes(
        2026, 1, 15,
        30.0444, 31.2357,
        2.0, // Cairo standard winter time is UTC+2
        CalculationMethod::EGYPT
    );

    std::cout << "Cairo 2026-01-15 Prayer Times:" << std::endl;
    std::cout << "  Fajr    : " << PrayerTimes::formatTime(times.fajr) << std::endl;
    std::cout << "  Sunrise : " << PrayerTimes::formatTime(times.sunrise) << std::endl;
    std::cout << "  Dhuhr   : " << PrayerTimes::formatTime(times.dhuhr) << std::endl;
    std::cout << "  Asr     : " << PrayerTimes::formatTime(times.asr) << std::endl;
    std::cout << "  Maghrib : " << PrayerTimes::formatTime(times.maghrib) << std::endl;
    std::cout << "  Isha    : " << PrayerTimes::formatTime(times.isha) << std::endl;

    assert(times.fajr < times.sunrise);
    assert(times.sunrise < times.dhuhr);
    assert(times.dhuhr < times.asr);
    assert(times.asr < times.maghrib);
    assert(times.maghrib < times.isha);

    // 2. Qibla Calculation
    QiblaResult qibla = calculateQibla(30.0444, 31.2357);
    std::cout << "\nQibla for Cairo:" << std::endl;
    std::cout << "  Bearing  : " << qibla.bearing << " deg" << std::endl;
    std::cout << "  Direction: " << qibla.direction << std::endl;
    std::cout << "  Distance : " << qibla.distanceKm << " km" << std::endl;

    assert(qibla.bearing >= 130.0 && qibla.bearing <= 140.0);
    assert(std::string(qibla.direction) == "SE");
    assert(qibla.distanceKm >= 1200.0 && qibla.distanceKm <= 1400.0);

    std::cout << "\nALL C++ TESTS PASSED SUCCESSFULLY!" << std::endl;
    return 0;
}
