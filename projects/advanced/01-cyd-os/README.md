# 🖥️ Flagship Project — CYD OS (Touchscreen Desktop Launcher)

> A lightweight, smartphone-like desktop operating system and app launcher designed for the 2.8" Cheap Yellow Display (ESP32-2432S028R). Features an app grid, live status bar, and modular app windows.

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% onboard ESP32, TFT, touch, LDR, LEDs, speaker)
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the Complete Code Walkthrough & OS Architecture**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the CYD Handheld Cyberdeck Terminal Housing**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features & Built-in Apps

- **Top Status Bar**: Live system uptime timer, WiFi connection status, and power indicator.
- **6-App Launcher Grid**:
  1. 🌦️ **Weather App**: Simulated local conditions, temperature, humidity, and weekly outlook.
  2. 🎵 **Music Player**: Compact audio control card with play/pause state.
  3. 🎨 **Paint App**: Fullscreen sketchpad with immediate stylus drawing.
  4. 📊 **SysInfo**: Live real-time hardware telemetry: free SRAM heap memory, CPU frequency, flash size, ESP-IDF core version, and LDR light sensor ADC reading.
  5. 💡 **RGB Flashlight**: Turns display to solid white and turns on all onboard RGB LEDs at 100% power.
  6. ⚙️ **Settings**: Display backlight PWM slider, device model specs, and licensing credits.
- **Window Management**: Dedicated top-right "X" close button in every app window to smoothly exit back to the desktop.
- **Audio Feedback**: Tactile click tones for every button and icon interaction.

---

## 🚀 How to Flash & Run

### PlatformIO:
```bash
cd projects/advanced/01-cyd-os
pio run -t upload
```

### Arduino IDE:
1. Ensure `TFT_eSPI` is configured for your 2.8" CYD.
2. Select **ESP32 Dev Module**.
3. Upload `01-cyd-os.ino`.

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

