# 📱 Project 05 — CYD 2.8" LVGL v8 Interactive Widget Showcase

> Experience smartphone-smooth touch animations, multi-tab navigation, sliders, switches, and circular gauge meters on your 2.8" Cheap Yellow Display (ESP32-2432S028R) using the industry-standard **LVGL v8** embedded graphics engine.

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐ Intermediate (Level 3/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% onboard display, touch, and LEDs)
- **Libraries Used**: `lvgl`, `TFT_eSPI`, `XPT2046_Touchscreen`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Cyberdeck Enclosure**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Multi-Tab Interface**: Swipe or tap tabs across "Controls", "Gauges", and "About" screens.
- **Hardware-Linked Widgets**:
  - Tapping the "Toggle LED" button triggers an audio chirp and toggles the onboard green LED.
  - Dragging the "Brightness Slider" modulates the onboard red LED duty cycle via hardware PWM timers.
- **Arc Circular Gauge**: Renders animated circular progress meters with smooth anti-aliased geometry.
- **Optimized Double-Buffering**: Zero flicker with DMA-friendly partial line buffers.

---

## 🚀 How to Flash & Run

### PlatformIO:
```bash
cd projects/intermediate/05-lvgl-widgets-demo
pio run -t upload
```

### Arduino IDE:
1. Install `lvgl` (version 8.3.x) and `TFT_eSPI`.
2. Copy [`hardware/lvgl/lv_conf.h`](../../../hardware/lvgl/lv_conf.h) next to your Arduino `lvgl` library folder.
3. Select **ESP32 Dev Module** and upload.

---

## 📜 License
Dual License: AGPL-3.0 for code & CC BY-NC-ND 4.0 for assets | Author: [@tushar2704](https://github.com/tushar2704)
