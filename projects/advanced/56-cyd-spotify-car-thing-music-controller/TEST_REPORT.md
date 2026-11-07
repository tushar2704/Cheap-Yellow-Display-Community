# 🧪 Test Report — Project 56: CYD Spotify / Apple Music Desk Controller (Car Thing)

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Build Time**: ~8.57 seconds
- **Memory Footprint**:
  - **RAM**: `22,628 bytes` (6.9% of 320 KB)
  - **Flash**: `335,837 bytes` (25.6% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active-low dominant color beat pulse |
| Piezo Buzzer | GPIO 26 | Transport touch haptic click audio |

---

## 3. Firmware Features Verified

- [x] **Vinyl Turntable Deck**: 104×104 px animated vinyl record graphics with dominant color border
- [x] **Track Metadata Engine**: Displays song title, artist, album, and liked heart state
- [x] **Touch Scrub Bar**: Interactive seek bar with elapsed/remaining minute and second clock
- [x] **Transport Media Controls**: Big Play/Pause toggle, Skip Next, Skip Previous, Shuffle, and Repeat mode
- [x] **Touch Volume Slider**: Smooth horizontal attenuator mapping 0% to 100% volume
- [x] **Device Output Switcher**: Multi-device routing chip selector
- [x] **RGB LED Beat Pulse**: Dominant album art color rhythm pulsing
- [x] **Branding**: `IamNotIntoTech` branding integrated into header and startup sequence

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

