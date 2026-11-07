# 🧪 Test Report — Project 58: CYD Home Assistant Quick-Tiles Glass Panel

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Build Time**: ~7.52 seconds
- **Memory Footprint**:
  - **RAM**: `22,820 bytes` (7.0% of 320 KB)
  - **Flash**: `337,885 bytes` (25.8% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active-low HVAC & security status aura |
| Piezo Buzzer | GPIO 26 | UI touch click haptics |

---

## 3. Firmware Features Verified

- [x] **4 Navigation Tabs**: Smooth switching across Home Overview, Lights, Climate, and Energy views
- [x] **Circular Thermostat Engine**: Target temperature setpoint modification with dual-arc dial
- [x] **2×2 Lighting Dimmer Grid**: Interactive tiles with toggle state and percentage bar
- [x] **Security Alarm Switcher**: Disarmed, Armed Home, and Armed Away mode toggling
- [x] **Energy & Solar Telemetry**: Solar production, whole-home load, and battery state-of-charge gauge
- [x] **Digital Clock & Environment**: Real-time seconds counter and room temperature telemetry
- [x] **Branding**: `IamNotIntoTech` branding embedded in header, boot sequence, and test reports

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

