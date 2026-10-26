# 🏓 30 - CYD-to-CYD Cross-Screen Ball Physics & Air Pong

> Seamless dual-display wireless gaming experience for two Cheap Yellow Displays. Pass an animated physics ball back and forth across physical screen boundaries using sub-5ms ESP-NOW wireless packets!

[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Ready%20to%20Flash-brightgreen.svg)](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../LICENSE)

---

## 📺 Community Channels
- 🎬 **Tech, Coding & Build Videos**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printing, Enclosures & CAD**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🎯 How It Works

Place two CYDs side-by-side on your desk:
1. Tap the top-left button on one display to set **`ROLE: P1 [L]`** (Player 1 Left screen).
2. Tap the top-left button on the second display to set **`ROLE: P2 [R]`** (Player 2 Right screen).
3. Drag your paddle vertically anywhere on screen.
4. When the ball leaves the right boundary of Screen 1 ($X \ge 315$), it vanishes from Screen 1 and **instantly emerges on the left edge ($X = 8$) of Screen 2 with identical velocity and angle** via ESP-NOW.
5. If a player fails to return the ball and it crosses their goal line, an acoustic alarm sounds, RGB LEDs flash, and the scoreboard updates synchronously across both units!

---

## 🛠️ Hardware Requirements

- **2x ESP32-2432S028R** (or ESP32-2432S028C)
- **Zero Wi-Fi router or internet required** — runs on raw 2.4GHz MAC-to-MAC peer frames with zero configuration.

---

## 🚀 Quick Start (Web Flasher)

1. Open the [CYD Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/) in Google Chrome or Edge.
2. Select **30 — CYD-to-CYD Cross-Screen Pong**.
3. Flash both CYD units in 1-click!

---

## 💻 Building with PlatformIO

```bash
git clone https://github.com/tushar2704/Cheap-Yellow-Display-Community.git
cd Cheap-Yellow-Display-Community/projects/advanced/30-cyd-to-cyd-cross-screen-pong
pio run --target upload
```

---

## 📜 License
Licensed under GNU AGPLv3. See [LICENSE](../../LICENSE) for details.
