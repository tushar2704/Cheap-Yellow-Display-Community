# 🟡 Project 01 — Hello CYD (Cheap Yellow Display Hardware Test)

> Complete all-in-one hardware self-test suite for the ESP32 Cheap Yellow Display. Tests TFT display, resistive touchscreen, onboard RGB LED, light sensor, speaker, and MicroSD card with **zero external wiring**.

---

## 📋 Overview

- **Difficulty**: ⭐ Beginner (Level 1/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% uses onboard hardware!)
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `SPI`, `SD`

---

## 📺 Video Tutorials

- 🎬 **Watch the Complete Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Perfect Desk Stand Enclosure**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## 🚀 Features Tested

| Feature | Pin(s) | What it does |
|---|---|---|
| **Color TFT Display** | GPIO 2, 12, 13, 14, 15, 21 | Renders 320x240 landscape UI, rounded cards, custom color palette |
| **Resistive Touch** | GPIO 33 (CS), 36 (IRQ) | Real-time crosshair drawing, pressure (Z-axis) & coordinate tracking |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Boot status indicator and interactive color cycling |
| **Ambient Light Sensor** | GPIO 34 (ADC) | Live bar graph reacting to light changes (cover sensor to test) |
| **Audio Speaker** | GPIO 26 (DAC) | Audible touch feedback beep whenever the screen is touched |
| **MicroSD Card Slot** | GPIO 5, 18, 19, 23 | Automatic SPI bus detection and mount check |

---

## ⚡ How to Flash & Run

### Method 1: Using PlatformIO (Recommended)
1. Open this folder in VS Code / Cursor with PlatformIO installed.
2. Connect your CYD via USB cable.
3. Click **PlatformIO: Upload** (or run `pio run -t upload`).
4. That's it! All library dependencies and pin configurations are bundled in `platformio.ini`.

### Method 2: Using Arduino IDE
1. Install the `TFT_eSPI` library via Library Manager.
2. Install the `XPT2046_Touchscreen` library by Paul Stoffregen.
3. Copy [`hardware/tft_espi/User_Setup_CYD_28R.h`](../../../hardware/tft_espi/User_Setup_CYD_28R.h) into your Arduino library `TFT_eSPI/User_Setup.h`.
4. Select Board: **ESP32 Dev Module**.
5. Upload Speed: `921600`.
6. Click **Upload**.

---

## 🎯 What to Expect
1. Screen turns dark blue with a vibrant yellow community banner.
2. The onboard RGB LED blinks green when ready.
3. Tap anywhere inside the canvas: a cyan dot appears, a soft audio beep plays, and exact X/Y coordinates update instantly.
4. Wave your hand over the small photodiode (LDR) on the front: the light level bar graph moves in real time!

---

## 📜 License
MIT License — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)
