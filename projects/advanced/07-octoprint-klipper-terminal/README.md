# 🖨️ Project 07 — CYD 2.8" OctoPrint & Klipper 3D Printer Touch Terminal

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a dedicated touchscreen terminal and telemetry monitor for Klipper / Moonraker (Mainsail, Fluidd) and OctoPrint 3D printer setups. Displays live hotend nozzle and heated bed temperatures, active print progress %, remaining print duration, and instant interactive touch controls (Pause, Resume, Emergency Cancel, Chamber Light). Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Supported Printer Backends**: Klipper / Moonraker, OctoPrint, Repetier Server
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `ArduinoJson`, `HTTPClient`, `WiFi`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the 3D Printer Terminal Integration Guide**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Desktop Enclosure & Mount**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Dual Thermal Monitor**: Real-time hotend extruder and heated print bed actual vs. setpoint temperatures with color-coded safety badges.
- **Active Job Progress Bar**: Visual percent completion bar, active G-code filename, elapsed print time, and remaining time calculation.
- **Interactive Touch Controls**:
  - ⏸️ **PAUSE / RESUME**: Safely halt or resume G-code execution on the printer.
  - ⏹️ **CANCEL**: Immediate abort and emergency heater shutoff command.
  - 💡 **LIGHT / FAN**: Toggle enclosure LED chamber lighting or auxiliary exhaust fan.
  - 🔄 **TEMP TAP**: Tap thermal panel to re-sample sensor thermistors immediately.
- **RGB State Lighting**:
  - 🔵 **Cyan Pulse**: Actively printing.
  - 🟡 **Yellow**: Print paused / awaiting operator attention.
  - 🟢 **Green**: Print complete & ready.
  - 🔴 **Red**: Thermal runaway alert or emergency cancel triggered.
- **Acoustic Feedback**: Clean piezo chimes (GPIO 26) on button clicks and print finish.

---

## 🚀 How to Flash & Configure

### 1. 1-Click Flash in Your Browser (No Coding Required)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install OctoPrint & Klipper Terminal**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/07-octoprint-klipper-terminal
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)
