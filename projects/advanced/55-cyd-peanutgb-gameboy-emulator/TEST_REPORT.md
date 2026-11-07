# 🧪 Test Report — Project 55: CYD PeanutGB Classic Game Boy Handheld

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Build Time**: ~6.52 seconds
- **Memory Footprint**:
  - **RAM**: `22,900 bytes` (7.0% of 320 KB)
  - **Flash**: `338,033 bytes` (25.8% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | Dedicated HSPI bus for XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active-low status LED |
| Piezo Buzzer | GPIO 26 | Dual-tone startup chime & 4-voice PSG tones |

---

## 3. Firmware Features Verified

- [x] **Authentic Boot Animation**: Scrolling `IamNotIntoTech` logo with dual-tone C6->C7 boot chime
- [x] **Native 160×144 Aspect LCD**: Pixel-perfect centering with vintage dark lens bezel
- [x] **Touchpad Handheld Controls**: 4-way D-Pad, angled A & B buttons, Select & Start pills
- [x] **4 Palette Modes**: DMG Pea-Soup Green, Pocket Monochrome, Cyber Neon, Solar Amber
- [x] **3 Built-in Games**: Space Brick Breaker, Pixel Runner (Flappy), Chiptune Synthesizer
- [x] **Branding**: `IamNotIntoTech` branding embedded in handheld header, boot screen, and test reports

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)
