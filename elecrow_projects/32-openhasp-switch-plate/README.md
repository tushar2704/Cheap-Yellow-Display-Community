# Project 32: Smart Home Touch Switch Plate

A responsive smart home terminal and touch wall controller engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Control your connected home devices from a dedicated desktop or wall-mounted touch console. Designed for seamless Home Assistant and MQTT architectures, this terminal provides instantaneous control over multi-zone lighting with analog dimming sliders, dual-zone HVAC thermostat temperature and mode tuning, and one-touch scene triggers with synchronized chromatic mood feedback.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Scene LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for tactile UI feedback chirps

## Features
- **Multi-Zone Lighting Deck**:
  - Independent toggle switches for Main Ceiling, Desk Lamp, and Ambient Strip.
  - Smooth interactive touch-draggable 0-100% brightness slider with real-time value readouts.
- **HVAC Climate Thermostat Deck**:
  - Dual current/target temperature displays with +/- 0.5°C fine adjustment buttons.
  - 4-way HVAC mode selector: `HEAT`, `COOL`, `AUTO`, `OFF`.
- **Instant Scene Launcher**:
  - 4 high-contrast macro cards: `Focus Work`, `Movie Night`, `Relax / Chill`, `All Off`.
  - Real-time hardware RGB LED color sync matching active environmental scenes (Blue for Focus, Magenta for Movie, Green for Relax).
- **Persistent Bottom Navigation**:
  - High-visibility tabs (`LIGHTS`, `CLIMATE`, `SCENES`) for single-tap switching between control views.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

