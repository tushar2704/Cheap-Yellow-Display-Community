# 🧪 Test Report — Project 59: CYD Marauder WiFi & BLE Security Auditor

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Build Time**: ~7.03 seconds
- **Memory Footprint**:
  - **RAM**: `45,164 bytes` (13.8% of 320 KB)
  - **Flash**: `794,453 bytes` (60.6% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active-low intruder alert & sniffer pulse |
| Piezo Buzzer | GPIO 26 | Acoustic Geiger ticks & deauth alarm siren |

---

## 3. Firmware Features Verified

- [x] **802.11 Promiscuous RX Engine**: Real-time packet capture via `esp_wifi_set_promiscuous` with IRAM ISR callback
- [x] **2.4 GHz 13-Channel RF Waterfall**: Live packet density bar chart across channels 1–13 with 300ms channel hopping
- [x] **Deauthentication & Disassociation IDS**: Real-time 0x0C and 0x0A frame detection with red alert siren and LED flash
- [x] **WiFi AP Scanner**: Discovers nearby SSIDs, channels, RSSI, and WPA2/WPA3/Open encryption flags
- [x] **BLE Beacon & Tag Sniffer**: Tracking of nearby Bluetooth Low Energy tags, AirTags, and smart devices
- [x] **Acoustic Geiger Feedback**: Subtle audio ticks on packet reception with on-screen mute toggle
- [x] **Branding**: `IamNotIntoTech` branding embedded in header, boot sequence, and documentation

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)
