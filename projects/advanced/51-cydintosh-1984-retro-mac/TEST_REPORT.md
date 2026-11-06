# 🧪 Test Report — Project 51: CYD Cydintosh 1984 Retro Mac Emulator

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Memory Footprint**:
  - **RAM**: `22,644 bytes` (6.9% of 320 KB)
  - **Flash**: `342,105 bytes` (26.1% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Configured for warm vintage power LED |
| Piezo Buzzer | GPIO 26 | Ascending boot chime & key clicks |

---

## 3. Firmware Features Verified

- [x] **1984 Boot Sequence**: Animated Happy Mac icon centered on CRT gray background with vintage boot chime
- [x] **System 6 Desktop**: Authentic 50% checkerboard dither pattern, striped window frames, drop shadows, and desktop icons
- [x] **Interactive MacPaint**: 1-bit touch canvas with Pencil, Brush, Eraser, and Clear controls
- [x] **Working Calculator**: Live four-function accumulator calculator with button sounds
- [x] **About This Mac Dialog**: Displays System 6.0.8, Finder 6.1, memory stats, and `IamNotIntoTech` branding
- [x] **Touch Calibration**: `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` with active stroke interpolation

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)
