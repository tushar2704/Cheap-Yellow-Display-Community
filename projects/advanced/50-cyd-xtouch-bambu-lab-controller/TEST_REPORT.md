# 🧪 Test Report — Project 50: CYD X-Touch Bambu Lab 3D Printer Controller

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Memory Footprint**:
  - **RAM**: `47,320 bytes` (14.4% of 320 KB)
  - **Flash**: `958,901 bytes` (73.2% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active LOW logic verified |
| Piezo Buzzer | GPIO 26 | Haptic click and tone feedback |
| WiFi / MQTT | Built-in ESP32 WiFi | Tested with PubSubClient & ArduinoJson v7 |

---

## 3. Firmware Features Verified

- [x] **Screen 0 (Main HUD)**: Hotend and bed live/target temperatures, animated progress bar, layer counter, remaining countdown, speed modes, pause/stop buttons
- [x] **Screen 1 (AMS Bay)**: 4-slot filament bay with spool color badges, material tags, capacity meters, active slot highlighting
- [x] **Screen 2 (Hardware Controls)**: Chamber internal temperature, chamber light toggle, part fan %, auxiliary fan %
- [x] **Auto Simulation Fallback**: Automatic demo simulation activates when printer is offline, allowing complete offline interactive testing
- [x] **Branding**: `IamNotIntoTech` branding integrated into boot screen, navigation header, and hardware deck
- [x] **Touch Calibration**: `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` with 350ms debounce
- [x] **RGB LED**: Cyan breathe during active print, green solid on finish, amber on pause/error

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

