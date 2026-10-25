# Project 01: CardFlow Gesture Launcher (2.8" Capacitive CYD)

**Target Board:** `ESP32-2432S028C` (2.8" Capacitive Touch CYD)  
**Touch Controller:** `CST816S` / `CST820` (I²C)  
**Author:** `tushar2704 <tushar.inseec@gmail.com>`  
**License:** Dual Licensed (GNU AGPLv3 for Source Code / CC BY-NC-ND 4.0 for Hardware & Assets)  
**Credits:** Inspired by `@IamNotIntoTech` & `@Makerrdesigns`

---

## 📖 Overview

The **CardFlow Gesture Launcher** demonstrates the silky-smooth, low-latency gesture navigation made possible by the capacitive glass touch panel on the **ESP32-2432S028C**. It replaces clunky resistive stylus tapping with intuitive horizontal card swipes, hardware-accelerated gesture parsing, visual pagination indicators, and synchronized RGB ambient lighting.

---

## ⚡ Features

- **Fluid Swiping**: Swipe left or right to seamlessly cycle through 5 modular telemetry cards.
- **Hardware Gesture Engine**: Leverages the on-chip CST816S gesture detector for Swipe Left, Swipe Right, Single Tap, and Long Press.
- **Dynamic Chroma Sync**: Backlight RGB LEDs automatically cross-fade to match the theme color of the active card.
- **Acoustic Audio Feedback**: Synthesizes slide clicks and interaction chimes via the onboard speaker amplifier (GPIO 26).
- **Top Status Bar**: Live uptime clock, board profile badge, and battery icon.

---

## 🛠️ Hardware Wiring & Pinout

| Peripheral | Board Pin | Description |
| :--- | :--- | :--- |
| **Display SPI** | GPIO 13 (MOSI), 12 (MISO), 14 (SCK), 15 (CS), 2 (DC) | ILI9341 320x240 LCD |
| **Capacitive Touch** | GPIO 33 (SDA), GPIO 32 (SCL), GPIO 25 (RST), GPIO 21 (INT) | CST816S I²C Controller |
| **RGB LED** | GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) | Common-Anode Status LED |
| **Audio Amp** | GPIO 26 | Tone / Chime Speaker |
| **LDR Sensor** | GPIO 34 | Ambient Light Lux Meter |
