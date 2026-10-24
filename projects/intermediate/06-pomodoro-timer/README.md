# ⏳ Project 06 — CYD 2.8" Pomodoro Productivity Timer & Desk Companion

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a dedicated tactile desk Pomodoro timer. Boost productivity with 25-minute focus intervals, 5-minute short breaks, 15-minute long breaks, big digital countdown digits, live progress bars, session counter, and audible end-of-session chimes.

---

## 📋 Overview

- **Difficulty**: ⭐⭐ Intermediate (Level 2/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% onboard touchscreen, LEDs, and speaker)
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Ergonomic Desk Stand**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **3 Interval Presets**: Instant touch tab switching between **Focus (25m)**, **Short Break (5m)**, and **Long Break (15m)**.
- **Bold Digital Display**: High-contrast countdown digits with live horizontal progress bar.
- **Synchronized RGB LED States**:
  - 🔴 **Red**: Deep Focus / Work session running.
  - 🟢 **Green**: Break session active.
  - 🔵 **Blue**: Timer paused / ready.
- **Audible End Chimes**: Automatic multi-tone alarm on speaker pin 26 when time expires.
- **Session Tracker**: Tracks total completed focus blocks across your workday.

---

## 🚀 How to Flash & Run

### PlatformIO:
```bash
cd projects/intermediate/06-pomodoro-timer
pio run -t upload
```

### Arduino IDE:
1. Ensure `TFT_eSPI` is configured for your 2.8" CYD.
2. Select **ESP32 Dev Module**.
3. Upload `06-pomodoro-timer.ino`.

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)
