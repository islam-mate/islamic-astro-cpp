# islammate-astro-cpp ⚡🕌

**Modern, zero-dependency, header-only C++17/20 library for Islamic astronomical prayer times and Qibla direction.**

Perfect for **embedded systems (ESP32, STM32, Arduino Azan clocks), smartwatches, IoT, Unreal Engine, Godot, Qt, and high-performance native servers.**

---

## ✨ Features
- ⚡ **Header-Only**: Just drop `#include <islammate/astro.hpp>` into your project.
- 🚀 **Zero External Dependencies**: Pure standard C++ math (`<cmath>`).
- 🕌 **7 Calculation Conventions**: MWL, ISNA, Egypt, Umm al-Qura (Makkah), Karachi, Tehran, Jafari.
- 📐 **Juristic Asr**: Standard (Shafi'i/Maliki/Hanbali) & Hanafi school.
- 🧭 **High-Precision Qibla**: Forward azimuth bearing and Haversine distance in kilometers.
- 📖 **Full Doxygen Documentation**: Every function, parameter, and struct is thoroughly documented.

---

## 🚀 Quick Start (C++17+)

```cpp
#include <iostream>
#include <islammate/astro.hpp>

int main() {
    using namespace islammate::astro;

    // Calculate prayer times for Cairo on 2026-01-15 (UTC+2)
    PrayerTimes times = calculatePrayerTimes(
        2026, 1, 15,
        30.0444, 31.2357, // Latitude, Longitude
        2.0,              // Timezone offset hours (UTC+2)
        CalculationMethod::EGYPT
    );

    std::cout << "Fajr    : " << PrayerTimes::formatTime(times.fajr) << "\n";
    std::cout << "Sunrise : " << PrayerTimes::formatTime(times.sunrise) << "\n";
    std::cout << "Dhuhr   : " << PrayerTimes::formatTime(times.dhuhr) << "\n";
    std::cout << "Asr     : " << PrayerTimes::formatTime(times.asr) << "\n";
    std::cout << "Maghrib : " << PrayerTimes::formatTime(times.maghrib) << "\n";
    std::cout << "Isha    : " << PrayerTimes::formatTime(times.isha) << "\n";

    // Calculate Qibla bearing and distance
    QiblaResult qibla = calculateQibla(30.0444, 31.2357);
    std::cout << "Qibla Bearing : " << qibla.bearing << "° (" << qibla.direction << ")\n";
    std::cout << "Distance      : " << qibla.distanceKm << " km\n";

    return 0;
}
```

---

## 🛠️ Build with CMake
```cmake
add_subdirectory(packages/islammate-astro-cpp)
target_link_libraries(my_app PRIVATE islammate_astro)
```

---

## 📄 License
MIT License.
