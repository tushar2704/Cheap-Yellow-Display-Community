# 📻 Project 13: HamClock Mini Dashboard

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Data Integrity**: **100% REAL LIVE TELEMETRY** (Zero simulated/fake data)

---

## 🚀 Key Features

1. **100% Real Live Space Weather & Geomagnetic Conditions**:
   - Live stream directly from NOAA / HamQSL (`https://www.hamqsl.com/solarxml.php`).
   - Real-time Solar Flux Index (SFI), Sunspot Number (SSN), A-Index, K-Index, Aurora activity, and Solar Wind speed (km/s).
   - Dynamic RGB LED color alert based on live geomagnetic storm thresholds:
     - **Green**: Quiet ($K \le 2$)
     - **Yellow**: Unsettled ($K = 3 \text{ or } 4$)
     - **Red**: Geomagnetic Storm Warning ($K \ge 5$)

2. **Real Live HF Band Propagation Matrix**:
   - Real calculated Day & Night condition indicators across 80m, 40m, 20m, 15m, and 10m amateur bands.
   - Dynamic color-coded ratings (Good = Green, Fair = Yellow, Poor = Red).

3. **Real-Time POTA (Parks on the Air) Live DX Spots**:
   - Streaming live spots directly from the official POTA API (`https://api.pota.app/spot/activator`).
   - Live activator callsign, operating frequency (in MHz), mode (SSB, CW, FT8, FT4), and official park reference.
   - Real-time Great Circle Haversine distance in kilometers and forward azimuth beam heading relative to station location (Paris Eiffel Tower / `JN18eu`).

4. **Synchronized UTC Atomic Clock**:
   - NTP time synchronization via `pool.ntp.org`.
   - Flicker-free 1-second differential clock update in header bar.

5. **Calibrated Touch Controls**:
   - Quick bottom tab switcher between Solar Conditions and POTA live spots.
   - Tap any spot card for acoustic feedback on GPIO 26 and visual highlight selection.
   - Dedicated VSPI hardware touch bus (`MOSI: 32`, `MISO: 39`, `CLK: 25`, `CS: 33`) with polling mode and calibrated non-inverted Y mapping.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC (HSPI) |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 25, 39, 32** | XPT2046 Touch SPI | CS, CLK, MISO, MOSI (Dedicated VSPI, Polling mode) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |

---

## ⚙️ Configuration (`secrets.h`)

Create a local `secrets.h` file (git-ignored for security):

```cpp
#pragma once

#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Station Reference Coordinates (Default: Paris Eiffel Tower)
#define STATION_LAT   48.8584
#define STATION_LON   2.2945
#define QTH_LOCATOR   "JN18eu"
```

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/13-hamclock-mini
pio run -t upload --upload-port COM5
```
