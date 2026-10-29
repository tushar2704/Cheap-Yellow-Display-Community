# Project 38: Sim-Racing Telemetry Dash

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Sim-Racing Telemetry Dash** transforms the Elecrow 2.8" CYD into a competition-grade digital dashboard display (DDU) and RPM shift light indicator for sim-racing simulators (such as Assetto Corsa, iRacing, F1 23/24, Project CARS, and BeamNG). Powered by high-frequency telemetry updates, it delivers real-time race telemetry directly to the driver's eye line.

Featuring an ultra-bright 16-segment RPM shift light bar, giant center gear indicator, dynamic lap delta tracker, 4-wheel tyre temperature/pressure matrix, and a physical/touch Discord voice mute macro button, it provides complete sim cockpit awareness.

---

## Features

- **16-Segment RPM Shift Light Bar**:
  - Multicolor LED sequence: Green (50-70% RPM) -> Amber (70-88% RPM) -> Red (88-96% RPM) -> Flashing Magenta (>96% Redline).
  - Synchronized peripheral eye-level flashing via on-board RGB LED (`IO4`, `IO16`, `IO17`).
- **Giant Digital Gear Indicator**:
  - High-visibility center gear display (`N`, `1`..`7`, `R`) with red flash alert upon reaching optimal upshift RPM.
- **Speedometer & Lap Delta Engine**:
  - Digital speed readout with instant KM/H and MPH unit switching.
  - Live lap delta calculation displaying green for negative time gain and red for time loss.
- **4-Wheel Tyre Health Matrix**:
  - Independent surface temperatures (°C) and tyre pressures (psi) for Front-Left, Front-Right, Rear-Left, and Rear-Right wheels.
  - Heat gradient color coding: Cold Blue (<75°C), Optimal Green (75-98°C), Overheating Red (>98°C).
- **Interactive Touch Cockpit Macros**:
  - **DISCORD MUTE**: One-touch voice mute toggle that changes status and button color instantly.
  - **SPEED UNIT**: Toggles between KM/H and MPH on the fly.
  - **PIT LIMITER**: Engages pit lane speed restriction mode with visual amber alert.
- **Acoustic Shift Tone**:
  - Emits high-frequency 3.4kHz shift cue chirps on `IO26` at the exact optimal upshift point.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **Shift Indicator RGB LED**| `GPIO 4` (R), `16` (G), `17` (B)| Active LOW peripheral shift indicator |
| **Audio Shift Buzzer** | `GPIO 26` | High-frequency acoustic shift cue |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/38-simracing-telemetry
   pio run --target upload
   ```
3. Set your local Wi-Fi SSID and UDP telemetry port in `38-simracing-telemetry.ino` to connect with your PC simulator, or run directly in high-speed simulation mode immediately.
