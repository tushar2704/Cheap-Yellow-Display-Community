# 🧪 Test Report: Project 13 - HamClock Mini Dashboard

> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Author**: Tushar Aggarwal ([@tushar2704](https://github.com/tushar2704))  
> **Target Device**: Elecrow CrowPanel 2.8" / ESP32-2432S028R (COM5)  
> **Status**: Verified & Hardware Tested on Physical Device  
> **Telemetry Verification**: **100% REAL LIVE DATA CONFIRMED**

---

## 1. Hardware Verification & Calibration

| Subsystem | Target Pin / Bus | Verification Result | Notes |
|---|---|---|---|
| **Display Panel** | ILI9341 SPI (13, 12, 14, 15, 2) | **PASS** | 320x240 crisp solar dashboard & POTA spot list |
| **Backlight** | GPIO 21 | **PASS** | Active HIGH full brightness |
| **Resistive Touch** | SPI (MOSI: 32, MISO: 39, CLK: 25, CS: 33) | **PASS** | Non-inverted landscape Y mapping (`240..3800 -> 0..240`), polling mode (zero IRQ lockups) |
| **Audio Feedback** | GPIO 26 | **PASS** | Clear audible tone feedback on tab switch and spot selection |
| **RGB LED** | GPIO 4, 16, 17 | **PASS** | Dynamic geomagnetic K-index storm alert (Green/Yellow/Red) |
| **Wi-Fi & NTP** | ESP32 802.11 b/g/n | **PASS** | Connected to local AP, synchronized UTC clock with `pool.ntp.org` |

---

## 2. Real Telemetry Ingestion Audit (COM5 Serial Log)

```text
[SOLAR] Live: SFI=90, SSN=24, A=10, K=3, Wind=541.5 km/s
[POTA] Fetching live spots from api.pota.app...
[POTA] Spot 1: WA9TT on 18.100 MHz (FT8) @ US-4306 [6517 km @ 304 deg]
[POTA] Spot 2: KF8BWD on 14.346 MHz (SSB) @ US-1933 [6467 km @ 297 deg]
[POTA] Spot 3: WC1D on 14.033 MHz (CW) @ US-11601 [5542 km @ 293 deg]
[POTA] Spot 4: W4LOO on 14.053 MHz (CW) @ US-1872 [7226 km @ 284 deg]
[POTA] Successfully parsed 4 real live spots.
```

- **NOAA / HamQSL Feed**: Retrieved live SFI (90), SSN (24), A-Index (10), K-Index (3), and Solar Wind (541.5 km/s).
- **POTA API Feed**: Stream-decoded active real activators with call signs, frequencies, modes, and park designations.
- **Great Circle Distance & Bearing Math**: Computed live distances and azimuth angles from Paris (`JN18eu`).

---

## 3. PlatformIO Compilation & Memory Footprint

- **Environment**: `esp32dev`
- **Framework**: Arduino
- **Build Status**: **SUCCESS (0 errors, 0 warnings)**
- **Memory Footprint**:
  - RAM: `48,592 bytes` (14.8% of 320 KB)
  - Flash: `1,008,149 bytes` (76.9% of 1.31 MB)
- **Flashing Speed**: 921600 baud via `esptool.py` on `COM5`
