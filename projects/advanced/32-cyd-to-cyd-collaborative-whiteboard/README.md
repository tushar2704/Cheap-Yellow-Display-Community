# 🎨 32 - CYD-to-CYD Synchronous Collaborative Touch Whiteboard

> Real-time shared digital sketchpad for two or more Cheap Yellow Displays. Draw, annotate, and brainstorm simultaneously across devices with sub-5ms vector stroke streaming over ESP-NOW!

[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Ready%20to%20Flash-brightgreen.svg)](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../LICENSE)

---

## 📺 Community Channels
- 🎬 **Tech, Coding & Build Videos**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printing, Enclosures & CAD**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🎯 Features

- **Real-Time Vector Synchronization**: Drawing with a stylus or finger on one CYD instantly streams vector line segments $[(x_0, y_0) \to (x_1, y_1), \text{color}, \text{width}]$ over ESP-NOW; appears on the partner display with sub-5ms latency!
- **6-Color Palette**: Pure White, Red, Green, Blue, CYD Yellow, and Cyan.
- **Eraser Mode**: Tap `[ERASE]` to scrub lines with a thickened 6px eraser brush.
- **Stroke Width Selector**: Easily toggle between fine 1px detail pen and bold 3px marker.
- **Synchronized Canvas Wipe**: Tapping `[CLEAR]` wipes both connected CYD screens simultaneously with an acoustic confirmation chime.

---

## 🛠️ Hardware Requirements

- **2x ESP32-2432S028R** (Resistive) or **ESP32-2432S028C** (Capacitive)
- **Zero setup**: Automatically peers across 2.4GHz broadcast frames on boot.

---

## 🚀 Quick Start (Web Flasher)

1. Open the [CYD Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/) in Chrome or Edge.
2. Select **32 — CYD-to-CYD Collaborative Whiteboard**.
3. Flash both devices in 1-click!

---

## 💻 Building with PlatformIO

```bash
git clone https://github.com/tushar2704/Cheap-Yellow-Display-Community.git
cd Cheap-Yellow-Display-Community/projects/advanced/32-cyd-to-cyd-collaborative-whiteboard
pio run --target upload
```

---

## 📜 License
Licensed under GNU AGPLv3. See [LICENSE](../../LICENSE) for details.
