# 🟢 Project 07 — CYD 2.8" Matrix Digital Rain & Cyberpunk Desk Standby

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a futuristic desk display with falling Matrix digital glyph streams, touch-reactive ripple waves, cybernetic sound feedback, and automatic brightness adjustment powered by the onboard LDR ambient light sensor.

---

## 📋 Overview

- **Difficulty**: ⭐ Beginner (Level 1/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% uses onboard display, LDR sensor, speaker, and RGB LEDs)
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Cyberdeck Wedge Stand**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Falling Matrix Digital Rain**: 32 independent stream columns with randomized speeds, glyphs, and trailing alpha fade.
- **LDR Ambient Light Sensor Auto-Dimming**: Reads GPIO 34 in real time; dynamically adjusts backlight PWM duty cycle so the display stays gentle at night and vibrant during daylight.
- **Interactive Touch Screen Ripples**: Tap anywhere on the glass to trigger electric green cascading rain showers and audio feedback.
- **Onboard RGB & Audio Chimes**: Cybernetic tone chirps on touch and neon green status LED pulses.

---

## 🚀 How to Flash & Run

### PlatformIO (Recommended):
```bash
cd projects/beginner/07-retro-matrix
pio run -t upload
```

### Arduino IDE:
1. Ensure `TFT_eSPI` is configured for your 2.8" CYD.
2. Select **ESP32 Dev Module**.
3. Upload `07-retro-matrix.ino`.

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)
