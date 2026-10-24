# 🎛️ Project 04 — CYD 2.8" Touch RGB Sliders & Color Mixer

> Modernized touchscreen RGB color mixer for the 2.8" Cheap Yellow Display (ESP32-2432S028R). Features smooth graphical sliders that mix colors in real-time on screen and dynamically drive the onboard RGB LED via PWM hardware channels.

---

## 📋 Overview

- **Difficulty**: ⭐ Beginner (Level 1/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% uses onboard RGB LED & screen)
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Ergonomic Desk Stand**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **3 Independent Touch Sliders**: Red, Green, and Blue channels (0–255 range).
- **Live 16-bit Color Swatch**: Shows the blended color preview box with active HEX color readout.
- **Hardware PWM Control**: Directly drives GPIO 4 (Red), GPIO 16 (Green), and GPIO 17 (Blue) using ESP32 LEDC hardware timers.
- **Active-LOW Inversion Handled**: Automatically computes inverse PWM duties for the CYD's common-anode onboard LED.

---

## 🚀 How to Flash & Run

### PlatformIO:
```bash
cd projects/beginner/04-rgb-sliders
pio run -t upload
```

### Arduino IDE:
1. Ensure `TFT_eSPI` is configured for your 2.8" CYD.
2. Select **ESP32 Dev Module**.
3. Upload `04-rgb-sliders.ino`.

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

