# 🧪 Test Report: Project 13 - HamClock Mini Dashboard

> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Author**: Tushar Aggarwal ([@tushar2704](https://github.com/tushar2704))  
> **Target Device**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Status**: Verified & Hardware Calibrated  

---

## 1. Hardware Verification & Calibration

| Subsystem | Target Pin / Bus | Verification Result | Notes |
|---|---|---|---|
| **Display Panel** | ILI9341 SPI (13, 12, 14, 15, 2) | PASS | 320x240 solar dashboard & POTA spot list |
| **Backlight** | GPIO 21 | PASS | Active HIGH full brightness |
| **Resistive Touch** | SPI (MOSI: 32, MISO: 39, CLK: 25, CS: 33) | PASS | Non-inverted landscape Y mapping (240..3800 -> 0..240) |
| **Audio Feedback** | GPIO 26 | PASS | Tab click & spot select tone feedback |
| **RGB LED** | GPIO 4, 16, 17 | PASS | Dynamic geomagnetic K-index storm alert |

---

## 2. PlatformIO Compilation Audit

- **Environment**: `esp32dev`
- **Framework**: Arduino
- **Build Status**: Verified SUCCESS
- **Memory Footprint**:
  - Flash: ~720 KB
  - RAM: ~45 KB

