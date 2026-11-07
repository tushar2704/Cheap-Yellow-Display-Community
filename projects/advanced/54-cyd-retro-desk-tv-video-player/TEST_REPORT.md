# 🧪 Test Report — Project 54: CYD Retro Desk TV / Video & Animation Player

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Build Time**: ~5.99 seconds
- **Memory Footprint**:
  - **RAM**: `22,980 bytes` (7.0% of 320 KB)
  - **Flash**: `348,193 bytes` (26.6% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active-low standby / power glow |
| Piezo Buzzer | GPIO 26 | Tone-based test tones, static snow noise, and click haptics |

---

## 3. Firmware Features Verified

- [x] **Procedural TV Channels**:
  - CH 01: Full SMPTE color bars + 1000 Hz calibration tone + test card
  - CH 02: Realistic analog UHF snow static with acoustic noise generator
  - CH 03: 80s Outrun Synthwave sunset with 3D perspective rolling grid
  - CH 04: Bouncing DVD video screensaver with border physics & corner-hit tracker
  - CH 05: Matrix green phosphor digital rain cascade
  - CH 06: Audio oscilloscope waveform visualizer with stereo L/R VU meters
- [x] **Analog Bezel Controls**: Channel up/down buttons with static transition effect, volume toggle, CRT scanlines toggle
- [x] **Authentic CRT Power Off**: Two-stage phosphor line collapse and center dot fadeout
- [x] **On-Screen Display (OSD)**: Retro green VCR channel & mode overlay banner
- [x] **Branding**: `IamNotIntoTech` branding embedded in TV chassis badge, test card, and boot sequence

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

