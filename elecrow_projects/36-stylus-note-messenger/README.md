# Project 36: Stylus Note Messenger

A digital whiteboard, handheld scratchpad, and network note messenger engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Utilizing the continuous analog pressure capabilities of the CYD's resistive touch screen, this appliance functions as an interactive stylus sketchpad and physical desk messaging terminal. Users can draw notes, brainstorm diagrams, select from 5 high-visibility chromatic inks, choose variable line widths, erase mistakes, and dispatch the handwritten payload across the local network with one touch.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Transmission LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) for packet upload and delivery feedback
- **Buzzer**: Piezo buzzer on GPIO 26 for acoustic clicks and transmission fanfares

## Features
- **High-Precision Vector Interpolation**: Continuous vector line rendering eliminates jagged gaps during fast stylus gestures.
- **5-Color Ink Palette**: High-contrast vibrant ink selections:
  - Cyber Cyan
  - Electric Yellow
  - Matrix Green
  - Neon Pink
  - Pure White
- **Adjustable Line Weights & Eraser Mode**:
  - Fine Tip: 1px single-pass precision line
  - Medium Tip: 3px bold sketching stroke
  - Chisel Tip: 5px broad marker stroke
  - Interactive Eraser tool restoring canvas background
- **Network Dispatch Engine**:
  - One-tap `SEND` button triggers note packaging and simulates webhook/MQTT transmission.
  - Interactive modal dialog showing payload vector point count with synchronized blue LED pulsing and delivery confirmation chime.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

