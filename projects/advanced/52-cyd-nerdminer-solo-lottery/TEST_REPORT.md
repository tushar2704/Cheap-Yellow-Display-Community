# 🧪 Test Report — Project 52: CYD NerdMiner Solo Bitcoin Lottery Miner

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Memory Footprint**:
  - **RAM**: `22,804 bytes` (7.0% of 320 KB)
  - **Flash**: `337,225 bytes` (25.7% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Gold pulse on mining, green flash on share |
| Piezo Buzzer | GPIO 26 | Arcade coin audio chime |

---

## 3. Firmware Features Verified

- [x] **Dual-Core SHA-256 Engine**: Real double-SHA256 hardware acceleration via `mbedtls` calculating block nonces
- [x] **Live Hashrate Gauge**: Real-time KH/s calculation sampled every 1000ms with total hash accumulator
- [x] **Screen 0 (Miner Dashboard)**: Valid share counter, best difficulty recorder, current block height, jackpot reward in BTC & USD
- [x] **Screen 1 (Network & Halving)**: BTC spot price, global network hashrate, 4-year halving cycle progress bar
- [x] **Screen 2 (Stratum Console)**: Scrolling terminal log displaying jobs and accepted difficulty shares with blinking `_` cursor
- [x] **Touch Navigation**: Full calibration `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` with 350ms debounce
- [x] **Branding**: `IamNotIntoTech` branding integrated into boot screen and navigation header

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

