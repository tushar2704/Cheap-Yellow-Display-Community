# Capacitive 15 - Smart Thermostat Circular Dial

**Target Board:** `ESP32-2432S028C` (2.8" Capacitive Touch CYD)  
**Touch Controller:** `CST816S` / `CST820` (I²C: SDA=33, SCL=32, RST=25, INT=21)  
**Author:** `tushar2704 <tushar.inseec@gmail.com>`  
**License:** Dual Licensed (GNU AGPLv3 for Source Code / CC BY-NC-ND 4.0 for Hardware & Assets)  
**Credits:** Inspired by `@IamNotIntoTech` & `@Makerrdesigns`

---

## 📖 Overview

Smart HVAC thermostat: radial dial dragging, 16-30°C range, Heat/Cool modes & flame status. Engineered specifically to demonstrate capacitive touch responsiveness, hardware-accelerated gestures (Swipe Left/Right/Up/Down), zero-pressure touch tracking, and low-latency audio/visual feedback.

---

## ⚡ Features

- **Capacitive Touch:** CST816S I²C glass touch interface with gesture recognition.
- **Audio Feedback:** Synthesizes clicks, alert beeps, and audio response on GPIO 26.
- **Chroma Sync:** RGB LED indicates active mode, toggles, and state changes.
- **Hardware Telemetry:** Real-time LDR ambient light sensing on GPIO 34.

---

## 🛠️ Hardware Pinout

| Peripheral | Board Pin | Description |
| :--- | :--- | :--- |
| **Display SPI** | GPIO 13 (MOSI), 12 (MISO), 14 (SCK), 15 (CS), 2 (DC) | ILI9341 320x240 LCD |
| **Capacitive Touch** | GPIO 33 (SDA), GPIO 32 (SCL), GPIO 25 (RST), GPIO 21 (INT) | CST816S I²C Controller |
| **RGB LED** | GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) | Status Tri-Color LED |
| **Audio Amp** | GPIO 26 | Tone / Audio Output |
| **LDR Sensor** | GPIO 34 | Ambient Light Lux Meter |
