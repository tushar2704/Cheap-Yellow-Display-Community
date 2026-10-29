# Project 32: Smart Home Touch Switch Plate

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Smart Home Touch Switch Plate** turns the Elecrow 2.8" CYD into a modern in-wall smart home control terminal. Designed for Home Assistant ecosystems via standard MQTT protocols, it provides zero-latency touch controls for multi-zone lighting, HVAC target thermostat regulation, and instant room scene triggers.

The user interface features three dedicated pages with smooth continuous touch slider tracking for master brightness, increment buttons for climate control, and RGB feedback synchronized with active scene states.

---

## Features

- **Multi-Zone Lighting Panel (Page 1)**:
  - Three independent zone toggles ("Main Ceiling", "Desk Lamp", "Ambient Strip") with real-time state glow badges.
  - Continuous drag-and-drop master brightness slider (0% to 100%) with numerical percentage readout.
- **HVAC & Climate Thermostat (Page 2)**:
  - Large ambient room temperature readout alongside target thermostat setting.
  - Direct touch `[-0.5°]` and `[+0.5°]` increment adjustments.
  - Four-mode HVAC selector: **HEAT**, **COOL**, **AUTO**, and **OFF**.
- **One-Touch Master Scenes (Page 3)**:
  - Instant scene presets: "Focus Work" (100% cool white), "Movie Night" (15% warm dim), "Relax / Chill" (50% ambient), and "All Off" (blackout).
- **RGB Mood Synchronization**:
  - The on-board RGB LED (`IO4`, `IO16`, `IO17`) lights up with the active room mood (Blue for Focus, Magenta for Movie, Green for Relax).
- **Audible Touch Feedback**:
  - Emits crisp, satisfying frequency chirps on `IO26` upon button presses and slider updates.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **RGB Scene LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW scene mood indicator |
| **Piezo Buzzer** | `GPIO 26` | Touch interaction audio confirmation |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/32-openhasp-switch-plate
   pio run --target upload
   ```
3. Set your Wi-Fi SSID and MQTT broker address in `32-openhasp-switch-plate.ino` to connect with your Home Assistant broker, or use the built-in standalone simulation mode for instant desk testing.
