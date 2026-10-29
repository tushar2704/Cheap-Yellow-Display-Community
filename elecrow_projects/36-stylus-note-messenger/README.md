# Project 36: Stylus Note Messenger

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Stylus Note Messenger** turns the Elecrow 2.8" CYD into a high-precision digital sketchpad and desk note communicator. Leveraging the sub-millimeter precision of the XPT2046 resistive touch digitizer paired with a stylus pen, it captures smooth, continuous handwritten notes, diagrams, and quick memos.

With a single tap of the on-screen "SEND" button, vector stroke payloads are dispatched via Wi-Fi HTTP webhooks or MQTT to a companion Discord channel, Home Assistant dashboard, or remote desktop terminal.

---

## Features

- **High-Precision Vector Stroke Interpolation**:
  - Continuous point-to-point stroke interpolation prevents dotted lines during rapid handwriting or sketch motions.
  - Generates crisp digital ink across a generous 272x212 pixel drawing canvas.
- **Multi-Color Ink Palette & Eraser**:
  - Five vibrant neon ink pigments: Neon Cyan, Amber Yellow, Vibrant Green, Hot Pink, and Crisp White.
  - One-touch Eraser toggle for swift corrections.
- **Adjustable Pen Tip Thickness**:
  - Stepper toggle for 3 line weights: 1px Fine Needle, 3px Medium Ballpoint, and 5px Chisel Marker.
- **One-Touch Webhook Transmission**:
  - Live vector stroke counter tracking drawing complexity.
  - "SEND" button packages handwritten memos and dispatches via network webhook.
  - Visual animated transmission dialog with progress indicators.
- **RGB & Acoustic Feedback**:
  - Flashing blue LED during payload transmission; solid emerald green LED upon successful message acknowledgment.
  - Dual-tone happy completion chimes on `IO26`.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **Status RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW transmission feedback indicator |
| **Audio Buzzer** | `GPIO 26` | Acoustic feedback for pen selection and delivery |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/36-stylus-note-messenger
   pio run --target upload
   ```
3. Enter your local Wi-Fi SSID and target Webhook URL (e.g. Discord or Home Assistant webhook) in `36-stylus-note-messenger.ino` to transmit notes over the network.
