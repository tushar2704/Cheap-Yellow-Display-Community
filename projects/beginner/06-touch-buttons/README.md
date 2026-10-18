# 🔘 Project 06 — CYD 2.8" Touch Button Matrix & Toggle Dashboard

> A tactile touch control interface for the 2.8" Cheap Yellow Display (ESP32-2432S028R). Demonstrates latching toggle switches, state tracking, RGB LED feedback, and an interactive counter card.

---

## 📋 Overview

- **Difficulty**: ⭐ Beginner (Level 1/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% onboard touchscreen & LEDs)
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Wall Mount Home Controller**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Latching Toggle Buttons**: Three independent on/off switches ("LIGHT", "FAN", "ALARM") with active glow states.
- **Audio & Haptic Tone Feedback**: Distinct frequencies trigger on each button press via onboard speaker pin 26.
- **RGB LED Hardware State**: The onboard RGB LED synchronizes with button states (Yellow for Light, Blue for Fan, Red for Alarm).
- **Interactive Counter Card**: Tap-to-increment card with big bold digital digits.

---

## 🚀 How to Flash & Run

### PlatformIO:
```bash
cd projects/beginner/06-touch-buttons
pio run -t upload
```

### Arduino IDE:
1. Ensure `TFT_eSPI` is configured for your 2.8" CYD.
2. Select **ESP32 Dev Module**.
3. Upload `06-touch-buttons.ino`.

---

## 📜 License
Dual License: AGPL-3.0 for code & CC BY-NC-ND 4.0 for assets | Author: [@tushar2704](https://github.com/tushar2704)
