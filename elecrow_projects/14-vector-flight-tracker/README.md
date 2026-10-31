# ✈️ Project 14: High-Contrast Vector Flight Tracker

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Data Integrity**: **100% REAL LIVE ADS-B RADAR FEED** (Zero mock/fake data)

---

## 🚀 Key Features

1. **100% Real Live ADS-B Flight Tracking**:
   - Streams live overhead commercial aircraft within a 35 NM radius of Paris (Eiffel Tower / `48.8584° N, 2.2945° E`) via `api.adsb.lol`.
   - Real-time callsigns, decoded airlines (Air France, British Airways, Lufthansa, Ryanair, easyJet, etc.), and aircraft types (A320neo, B737 MAX 8, A220-300, etc.).

2. **Kinematic Vector Metrics Grid**:
   - Digital readouts for Altitude (thousands of feet), Ground Speed (knots and Mach), and 360° Heading Vector with cardinal direction.
   - Dynamic climb / descend state (+CLIMB, -DESC, CRUISE).
   - Dynamic flight progress bar based on live aircraft distance from the 35 NM airspace boundary.

3. **Active RGB Status LED**:
   - **Green**: Aircraft climbing ($> 500 \text{ fpm}$)
   - **Yellow**: Aircraft descending ($< -500 \text{ fpm}$)
   - **Blue / Cyan**: Aircraft level / cruising

4. **Calibrated Touchpad Controls**:
   - Dedicated buttons: `< PREV`, `AUTO ON / PAUSED`, and `NEXT >` for cycling through overhead flights.
   - Tap flight card body to advance to next target with audible beep feedback on GPIO 26.
   - Dedicated VSPI hardware touch bus (`MOSI: 32, MISO: 39, CLK: 25, CS: 33`) with polling mode and calibrated non-inverted Y mapping.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC (HSPI) |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 25, 39, 32** | XPT2046 Touch SPI | CS, CLK, MISO, MOSI (Dedicated VSPI, Polling mode) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone feedback |

---

## ⚙️ Configuration (`secrets.h`)

Create a local `secrets.h` file (git-ignored for security):

```cpp
#pragma once

#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Station Reference Coordinates (Default: Paris Eiffel Tower)
#define RADAR_LAT     48.8584
#define RADAR_LON     2.2945
#define RADAR_RANGE   35
```

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/14-vector-flight-tracker
pio run -t upload --upload-port COM5
```
