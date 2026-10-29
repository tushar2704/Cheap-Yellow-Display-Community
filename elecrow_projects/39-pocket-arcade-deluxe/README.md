# Project 39: 5-in-1 Pocket Arcade Deluxe

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **5-in-1 Pocket Arcade Deluxe** converts the Elecrow 2.8" CYD into a retro handheld gaming machine packed with five classic arcade games. Engineered with an ultra-responsive zero-flicker dirty-rectangle rendering pipeline running at 40 FPS, it delivers smooth touchscreen gaming directly on the ILI9341 display.

Complete with an arcade marquee game selector menu, persistent high-score leaderboards, 8-bit chiptune sound effects, and powerup RGB lighting, it provides timeless arcade entertainment in a pocket form factor.

---

## Included Games

1. **Cyber Breakout Deluxe**:
   - Touch paddle control with continuous physics ball rebound angles and multi-tiered destructible color brick rows.
2. **Neon Snake**:
   - 60 FPS directional touch-steered cyber snake with procedural food spawning, growing tail segments, and collision detection.
3. **Flappy CYD**:
   - Tap-to-jump gravity physics simulation with random pipe gap obstacles and pass-through score counters.
4. **Space Defender**:
   - Touch-guided starship with laser firing mechanics targeting descending alien invaders.
5. **Memory Matrix**:
   - Simon-style 4-quadrant musical audio-visual pattern memorization game.

---

## Features

- **Zero-Flicker Dirty-Rectangle Graphics**:
  - Direct localized sprite erasing and redrawing eliminating screen flash and tearing.
- **Arcade Audio Synthesizer**:
  - Emits authentic 8-bit retro gaming sound effects on `GPIO 26` (paddle bounces, flap chirps, food bites, and crash jingles).
- **RGB Powerup Lighting**:
  - Tri-color RGB LED (`IO4`, `IO16`, `IO17`) flashes green on scoring, blue on powerups, and red on game over.
- **Arcade Cabinet Menu**:
  - Graphical touch menu launcher with high score records and single-tap game switching.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **Arcade RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW score/powerup/gameover indicator |
| **8-Bit Chiptune Buzzer** | `GPIO 26` | Piezo sound effects synthesizer |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/39-pocket-arcade-deluxe
   pio run --target upload
   ```
3. Use a finger or stylus to select a game from the main arcade marquee and play immediately.
