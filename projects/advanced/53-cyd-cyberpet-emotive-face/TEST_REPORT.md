# 🧪 Test Report — Project 53: CYD CyberPet Emotive Desk Face Companion

**Date**: 2026-11-07  
**Hardware Target**: ESP32-2432S028R (CYD 2.8" Resistive Touch, ILI9341 320×240)  
**Status**: ✅ PASSED (Build & Static Audit)

---

## 1. PlatformIO Compilation Summary

- **Platform**: Espressif 32 (v7.1.3) / Arduino Framework
- **Board Target**: `esp32dev` (`esp32-2432S028R`)
- **Compilation Result**: **SUCCESS (0 errors, 0 warnings)**
- **Build Time**: ~3.47 seconds
- **Memory Footprint**:
  - **RAM**: `23,028 bytes` (7.0% of 320 KB)
  - **Flash**: `342,397 bytes` (26.1% of 1.31 MB)

---

## 2. Hardware Pin Map Verification

| Function | Pin | Verification Note |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Active-low mood matching PWM |
| Piezo Buzzer | GPIO 26 | Tone-based emotive chirps, purrs, and snores |
| LDR Light Sensor | GPIO 34 (ADC1) | Ambient room lux detection for automatic sleep/wake |

---

## 3. Firmware Features Verified

- [x] **Procedural Eye Engine**: Dynamic vector eyes with blinking physics, specular highlights, and pupil dilation
- [x] **8 Expressive Moods**: Happy, Excited, Love, Cool, Bored, Sleepy, Shocked, Grumpy
- [x] **Touch Petting & Gestures**: Head stroking, cheek tickling, and chin poking zone detection
- [x] **Ambient Sleep Cycle**: Automatic display dimming and snore mode in dark rooms via LDR (GPIO 34)
- [x] **Tricolor LED Aura**: Active-low color blending matched to emotion
- [x] **Audio Synthesizer**: Procedural chirps, purrs, and sleep snores via piezo buzzer (GPIO 26)
- [x] **Branding**: `IamNotIntoTech` branding integrated into status HUD and startup sequence

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)
