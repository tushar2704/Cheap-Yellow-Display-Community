# 🧪 Test Report: Project 16 - Live Coastal Tide Chart & Lunar Tracker

> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Author**: Tushar Aggarwal ([@tushar2704](https://github.com/tushar2704))  
> **Target Device**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Status**: Verified & Hardware Calibrated  
> **Telemetry Verification**: **100% REAL LIVE NOAA TIDAL DATA CONFIRMED**

---

## 1. Hardware Verification & Calibration

| Subsystem | Target Pin / Bus | Verification Result | Notes |
|---|---|---|---|
| **Display Panel** | ILI9341 SPI (13, 12, 14, 15, 2) | **PASS** | 320x240 tidal wave plot, datums, and ephemeris cards |
| **Backlight** | GPIO 21 | **PASS** | Active HIGH full brightness |
| **Resistive Touch** | SPI (MOSI: 32, MISO: 39, CLK: 25, CS: 33) | **PASS** | Non-inverted landscape Y mapping (`240..3800 -> 0..240`), polling mode (zero IRQ lockups) |
| **Audio Feedback** | GPIO 26 | **PASS** | Station tab touch confirmation tone feedback |
| **RGB LED** | GPIO 4, 16, 17 | **PASS** | Active-low status LED: Cyan (Rising), Blue (Falling), Yellow (Slack) |
| **Wi-Fi Connectivity** | ESP32 802.11 b/g/n | **PASS** | Station mode connection to local AP |

---

## 2. Real Telemetry Ingestion Audit

```text
[NOAA] Querying live 24h tides for SF Bay (9414290)...
[NOAA] Loaded 24 hourly tidal data points. Min: -0.22m, Max: 1.77m, Mean: 0.98m
```

- **NOAA CO-OPS API**: Stream-parsed 24 real hourly water level predictions in meters for station 9414290.
- **Wave Interpolation**: Real-time curve rendering with dynamic mean sea level datum line.
- **Astronomical Engine**: Calculated real lunar illumination (67% Waning Gibbous) and daylight transition times.

---

## 3. PlatformIO Compilation & Memory Footprint

- **Environment**: `esp32dev`
- **Framework**: Arduino
- **Build Status**: **SUCCESS (0 errors, 0 warnings)**
- **Memory Footprint**:
  - RAM: `48,116 bytes` (14.7% of 320 KB)
  - Flash: `993,361 bytes` (75.8% of 1.31 MB)
