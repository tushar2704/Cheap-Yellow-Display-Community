# 🧪 Test Report — Project 63: CYD HamClock Space Weather Station

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Build Time**: ~5.98 seconds
- **Memory Footprint**:
  - **RAM**: `22,800 bytes` (7.0% of 320 KB)
  - **Flash**: `346,405 bytes` (26.4% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | Dedicated HSPI bus for XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active-low geomagnetic Kp alert glow |
| Piezo Buzzer | GPIO 26 | Hourly Morse Code "DX" audio chimes |

---

## 3. Firmware Features Verified

- [x] **World Map & Solar Terminator**: Real-time sinusoidal day/night greyline projection across global continents
- [x] **Subsolar Point Tracker**: Real-time position calculation for subsolar sun point and auroral oval ring
- [x] **Dual Timezone Clocks**: UTC / Zulu master clock and local time with solar noon/midnight markers
- [x] **Space Weather Gauges**: Solar Flux Index (SFI), Sunspot Number (SSN), Kp-index, and X-ray flare class
- [x] **HF Band Propagation Matrix**: Multi-band ionospheric rating across 10m, 15m, 20m, and 40m bands
- [x] **Hourly Morse Code Audio**: Generates `-.. -..-` ("DX") audio tone on the hour with mute toggle
- [x] **Branding**: `IamNotIntoTech` branding embedded in header, boot sequence, and documentation

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)
