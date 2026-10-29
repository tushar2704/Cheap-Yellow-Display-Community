# Project 41: Minimax AI Gomoku & Tactical Board

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Minimax AI Gomoku & Tactical Board** turns the Elecrow 2.8" CYD into a standalone strategic artificial intelligence gaming appliance. Running an embedded Minimax search engine with Alpha-Beta pruning on the ESP32's dual-core processor, it challenges players in the ancient game of Gomoku (Five-in-a-Row) without needing internet or cloud computation.

The appliance features an authentic bamboo wood board theme, stylus-precision stone placement, real-time AI heuristics telemetry (search depth, leaf nodes evaluated, and live advantage meter), tactile stone audio snaps, and RGB calculation indicators.

---

## Features

- **10x10 Tactical Gomoku Grid**:
  - Warm wood grain styling with star points (hoshi) and anti-aliased black/white stone graphics with specular highlights.
  - Red indicator ring on the most recent move for easy board tracking.
- **Embedded Minimax Search with Alpha-Beta Pruning**:
  - Two selectable difficulty tiers: **NOVICE** (Depth 2) and **MASTER** (Depth 3 with offensive/defensive pattern recognition).
  - Evaluates thousands of game tree permutations in milliseconds.
- **Live AI Telemetry Dashboard**:
  - Search Depth indicator.
  - Nodes Searched real-time counter.
  - Dynamic Advantage Meter bar visualizing AI vs Human positional superiority.
- **Tactile Audio & RGB Mood Feedback**:
  - Emits crisp stone snap acoustic clicks on `GPIO 26`.
  - Blue RGB LED (`IO17`) pulses while AI evaluates branch heuristics; green flashes on human victory, red on AI victory.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **AI Status RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW AI thinking / victory indicator |
| **Tactile Buzzer** | `GPIO 26` | Piezo stone placement snap synthesizer |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/41-minimax-ai-gomoku
   pio run --target upload
   ```
3. Tap on any grid intersection with a stylus or finger to place your stone and play against the on-device AI.
