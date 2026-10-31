# Project 39: 5-in-1 Pocket Arcade Deluxe

A multi-game retro arcade entertainment console engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
A handheld pocket gaming appliance packing 5 classic retro titles into a single unified ESP32 firmware. Powered by the CYD's 2.8-inch touchscreen, this system combines smooth 40 FPS real-time rendering, analog touch drag physics, multi-directional touch quadrant navigation, persistent high score retention, and authentic 8-bit sound effects.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Mood LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) for game over alerts and scoring powerups
- **Buzzer**: Piezo buzzer on GPIO 26 for 8-bit chiptune audio synthesizers

## Features
- **5 Built-In Retro Arcade Games**:
  - `Cyber Breakout`: Touch-drag paddle with dynamic ball angle rebounds and 32 destructible color-tiered bricks.
  - `Neon Snake`: Touch steering logic with continuous growing tail segments and randomized food generation.
  - `Flappy CYD`: Tap-to-flap gravity physics with scrolling pipe obstacles and point counters.
  - `Space Defender`: Galactic ship maneuverability and alien invader waves.
  - `Memory Matrix`: Simon-style multi-quadrant chromatic and musical pattern memory tester.
- **Arcade Selection Marquee**:
  - Main menu presenting game titles, descriptions, and high-score records with instant launch.
- **Global Exit Protocol**:
  - Tap `EXIT [X]` in the top corner during any game to return safely to the selection marquee.
- **8-Bit Audio Effects**:
  - Distinctive chiptune tones for paddle bounces, wall hits, food bites, wing flaps, and crash alarms.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
