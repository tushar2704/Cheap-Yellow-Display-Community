# Project 33: Streaming Internet Lo-Fi Radio

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Streaming Internet Lo-Fi Radio** turns the Elecrow 2.8" CYD into an ambient desktop audio streamer and high-refresh spectrum analyzer. Featuring an animated 16-band real-time audio FFT spectrum visualizer, it pairs chilled lo-fi study beats, retrowave, and jazz audio with eye-catching neon visualizations and bass-reactive RGB lighting.

With full touchscreen media controls, users can skip channels, pause playback, toggle visualizer rendering modes, and regulate volume directly on the display.

---

## Features

- **16-Band Real-Time Audio FFT Visualizer**:
  - 16 discrete vertical frequency bands with physical gravity peak hold lines.
  - Three switchable graphic visualization modes:
    - **BARS**: Neon cyan-to-magenta segmented equalizer bars.
    - **DOTS**: Minimalist retro dot matrix spectrum.
    - **FIRE**: Warm analog audio wave flame gradient.
- **Station Carousel & Stream Metadata**:
  - Live station metadata display with stream bitrate and genre tags.
  - Preloaded channels: *Lofi Girl Beats*, *Synthwave Cyberwave*, *Tokyo Coffeehouse Jazz*, and *Deep Drone Cosmos*.
- **Touch Media Controls**:
  - Previous (`|<<`) and Next (`>>|`) station skip buttons.
  - Interactive `PLAY` / `PAUSE` state toggle.
  - Five-step Master Volume cyclic stepper (`20%` to `100%`).
  - One-tap visualizer mode cycler.
- **Bass-Reactive Mood Lighting**:
  - Hardware RGB LED (`IO17` / `IO16`) dynamically pulses cyan and teal in rhythm with low-frequency bass kicks.
- **Audio Output**:
  - Generates audio tones and streams to the on-board speaker on `GPIO 26`.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **Bass Sync RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW rhythmic bass indicator |
| **Audio DAC / Speaker** | `GPIO 26` | Audio tone and stream output pin |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/33-internet-lofi-radio
   pio run --target upload
   ```
3. Set your Wi-Fi SSID and credentials in `33-internet-lofi-radio.ino` to stream live audio, or enjoy the built-in standalone audio synthesizer and spectrum visualizer immediately.
