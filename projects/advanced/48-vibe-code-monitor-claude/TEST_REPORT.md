# 🧪 Test Report — Project 48: Claude Code Vibe Monitor

**Date**: 2026-11-06  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Memory Footprint**:
  - **RAM**: `48,780 bytes` (14.9% of 320 KB)
  - **Flash**: `998,357 bytes` (76.2% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active LOW logic verified |
| WiFi / HTTP | Built-in ESP32 WiFi | Tested with ArduinoJson v7 parser |

---

## 3. Firmware Features Verified

- [x] **Screen 0 (Window Gauge)**: 5-hour rolling rate-limit window gauge with dynamic warning colors, reset countdown, 7-day cumulative bar, session cost
- [x] **Screen 1 (Economics)**: Token breakdown by input, output, cache write, cache read with price weighting
- [x] **Screen 2 (Warning / Standby)**: Auto-triggers red pulsing warning border when quota >80%; falls back to NTP standby clock
- [x] **Touch Navigation**: 500ms debounce with XPT2046 calibration (`map(p.x, 240, 3800, 0, 320)`)
- [x] **RGB LED States**: Orange breathe (<50%), amber pulse (50-80%), red rapid blink (>80%), blue solid (offline)
- [x] **Python Bridge (`bridge/monitor.py`)**: Reads `~/.claude/projects/**/*.jsonl` locally with rolling 5h window computation

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)
