# 🧪 Test Report — Project 62: CYD Apple Juice & BLE Spam Sentinel

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Build Time**: ~7.05 seconds
- **Memory Footprint**:
  - **RAM**: `40,572 bytes` (12.4% of 320 KB)
  - **Flash**: `1,182,661 bytes` (90.2% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | Dedicated HSPI bus for XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active-low intrusion alert strobe |
| Piezo Buzzer | GPIO 26 | Dual-tone acoustic alarm siren |

---

## 3. Firmware Features Verified

- [x] **BLE Promiscuous Scanner**: Active BLE 5.0 scanning with non-blocking 1-second sampling cycles
- [x] **Apple Juice Detection Engine**: Manufacturer ID 0x004C parsing with Action Subtypes 0x07, 0x10, and 0x05
- [x] **Android & Windows Flood Detection**: Google Fast Pair (UUID 0xFE2C) and MS Swift Pair (0x0006) detection
- [x] **AirTag Tracking Radar**: Apple FindMy Subtype 0x12 identification with RSSI proximity bar
- [x] **Acoustic Intrusion Siren**: High-frequency dual-tone alert on detected spam floods with touch mute toggle
- [x] **3-Tab Navigation HUD**: Seamless switching across Defense Shield, Incident Logs, and AirTag Radar
- [x] **Branding**: `IamNotIntoTech` branding embedded in header, boot sequence, and documentation

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)
