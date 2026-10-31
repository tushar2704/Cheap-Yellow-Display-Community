# 🧪 Test Report: Project 14 - High-Contrast Vector Flight Tracker

> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Author**: Tushar Aggarwal ([@tushar2704](https://github.com/tushar2704))  
> **Target Device**: Elecrow CrowPanel 2.8" / ESP32-2432S028R (COM5)  
> **Status**: Verified & Hardware Tested on Physical Device  
> **Telemetry Verification**: **100% REAL LIVE ADS-B DATA CONFIRMED**

---

## 1. Hardware Verification & Calibration

| Subsystem | Target Pin / Bus | Verification Result | Notes |
|---|---|---|---|
| **Display Panel** | ILI9341 SPI (13, 12, 14, 15, 2) | **PASS** | 320x240 high-contrast vector flight cards |
| **Backlight** | GPIO 21 | **PASS** | Active HIGH full brightness |
| **Resistive Touch** | SPI (MOSI: 32, MISO: 39, CLK: 25, CS: 33) | **PASS** | Non-inverted landscape Y mapping (`240..3800 -> 0..240`), polling mode (zero IRQ lockups) |
| **Audio Feedback** | GPIO 26 | **PASS** | Button and auto-cycle confirmation tone feedback |
| **RGB LED** | GPIO 4, 16, 17 | **PASS** | Dynamic climb (Green) / descend (Yellow) / cruise (Blue) indication |
| **Wi-Fi Connectivity** | ESP32 802.11 b/g/n | **PASS** | Station mode connection to local AP |

---

## 2. Real Telemetry Ingestion Audit (COM5 Serial Log)

```text
[ADS-B] Flight 1: AFR7526 (Air France, Airbus A220-300) Alt: 21600 ft, Speed: 423 kts, Hdg: 264 deg, Dist: 28.8 NM
[ADS-B] Flight 2: BAW322 (British Airways, Airbus A321neo) Alt: 8475 ft, Speed: 249 kts, Hdg: 150 deg, Dist: 29.2 NM
[ADS-B] Flight 3: EWG80F (Commercial Jet, Airbus A320) Alt: 37925 ft, Speed: 417 kts, Hdg: 53 deg, Dist: 26.6 NM
[ADS-B] Flight 4: RYR88SX (Ryanair, B737 MAX 8) Alt: 36000 ft, Speed: 435 kts, Hdg: 29 deg, Dist: 20.7 NM
[ADS-B] Flight 5: AFR71TJ (Air France, Airbus A220-300) Alt: 6125 ft, Speed: 216 kts, Hdg: 71 deg, Dist: 16.6 NM
[ADS-B] Flight 6: RYR57MK (Ryanair, Boeing 737-800) Alt: 36000 ft, Speed: 421 kts, Hdg: 55 deg, Dist: 29.1 NM
[ADS-B] Flight 7: TUI89A (Commercial Jet, Boeing 737-800) Alt: 36000 ft, Speed: 424 kts, Hdg: 56 deg, Dist: 14.2 NM
[ADS-B] Flight 8: JAF6JE (TUI fly Belgium, B737 MAX 8) Alt: 34000 ft, Speed: 424 kts, Hdg: 33 deg, Dist: 20.7 NM
[ADS-B] Flight 9: DAH1230 (Air Algerie, Boeing 737-800) Alt: 4775 ft, Speed: 185 kts, Hdg: 82 deg, Dist: 9.7 NM
[ADS-B] Flight 10: IBE06HA (Iberia, Airbus A320neo) Alt: 33900 ft, Speed: 426 kts, Hdg: 35 deg, Dist: 32.9 NM
[ADS-B] Flight 11: EJU45EZ (easyJet, Airbus A320) Alt: 8025 ft, Speed: 294 kts, Hdg: 264 deg, Dist: 4.4 NM
[ADS-B] Flight 12: TVF43RL (Transavia, Boeing 737-800) Alt: 7550 ft, Speed: 208 kts, Hdg: 12 deg, Dist: 29.6 NM
[ADS-B] Successfully loaded 12 real live aircraft.
```

- **ADS-B Live Ingestion**: Successfully queried and stream-filtered 12 live aircraft within 35 NM of the Eiffel Tower in Paris.
- **Flight Kinematics**: Parsed real altitude, ground speed, Mach number, vertical speed, heading, and distance from Paris center.
- **Interactive Carousel**: Auto-cycles every 6 seconds or manually cycles via `< PREV`, `NEXT >`, or card taps.

---

## 3. PlatformIO Compilation & Memory Footprint

- **Environment**: `esp32dev`
- **Framework**: Arduino
- **Build Status**: **SUCCESS (0 errors, 0 warnings)**
- **Memory Footprint**:
  - RAM: `49,720 bytes` (15.2% of 320 KB)
  - Flash: `991,549 bytes` (75.6% of 1.31 MB)
- **Flashing Speed**: 921600 baud via `esptool.py` on `COM5`
