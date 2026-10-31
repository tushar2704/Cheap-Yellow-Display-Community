# 🌊 Project 16: Live Coastal Tide Chart & Lunar Tracker

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Data Integrity**: **100% REAL LIVE NOAA TIDAL PREDICTIONS & EPHEMERIS** (Zero simulated/fake data)

---

## 🚀 Key Features

1. **100% Real Live NOAA Tidal Telemetry**:
   - Queries the official NOAA Center for Operational Oceanographic Products and Services (CO-OPS) REST API.
   - Real 24-hour hourly water level predictions in meters for 4 premier coastal ports:
     - **SF Bay** (`9414290`, Golden Gate, Pacific Coast)
     - **New York** (`8518750`, The Battery, Atlantic Coast)
     - **Boston** (`8443970`, Boston Harbor, Atlantic Coast)
     - **Key West** (`8724580`, Florida Straits, Gulf/Atlantic)

2. **Continuous Tidal Wave Curve Plotting**:
   - Real smooth tidal wave curve interpolated across 24 hourly predictions.
   - Dynamic datums calculated from live data: High Water crest, Low Water trough, and Mean Sea Level datum line.
   - Real-time vertical cursor tracking current atomic time with instantaneous water level in meters and rising/falling trend.

3. **Astronomical Ephemeris & Moon Phase Engine**:
   - High-precision lunar phase calculation derived from synchronized atomic NTP time.
   - Displays real-time illumination percentage and phase designation (e.g. `67% WANING GIBBOUS`).
   - Identifies Spring vs. Neap tidal amplification cycles.

4. **Multi-Color RGB Water State Indication**:
   - **Cyan**: Rising / Flood Tide
   - **Blue**: Falling / Ebb Tide
   - **Yellow**: Slack Water (within 0.15m of crest / trough)

5. **Calibrated Touch Controls**:
   - Bottom harbor selector tabs with acoustic beep on GPIO 26.
   - Dedicated VSPI hardware touch bus (`MOSI: 32, MISO: 39, CLK: 25, CS: 33`) with polling mode and calibrated non-inverted Y mapping.

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
```

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/16-coastal-tide-chart
pio run -t upload --upload-port COM5
```
