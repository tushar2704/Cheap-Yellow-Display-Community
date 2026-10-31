# Project 29: Formula 1 Pit Wall Clock & Telemetry

A dedicated desktop motorsport telemetry dashboard and Grand Prix race countdown terminal engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Transform your desk into an authentic Formula 1 pit wall command console. This terminal displays live circuit telemetry, high-contrast vector circuit maps, World Championship driver standings, track environmental conditions (track temp, air temp, rain probability), and an interactive 5-red-lights race start audio/visual simulator.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) for FIA race flags
- **Buzzer**: Piezo buzzer on GPIO 26 for race start countdown and tactile touch clicks

## Features
- **Grand Prix Weekend Countdown**: Microsecond-accurate countdown timer ticking down days, hours, minutes, and seconds until Sunday race lights out.
- **Vector Circuit Map Renderer**: Displays 2D track layouts (Monaco, Silverstone, Spa-Francorchamps, Suzuka) with DRS activation zones and circuit statistics.
- **World Drivers' Championship Standings**: Live championship leaderboard styled with authentic team livery accent bars (Red Bull Navy, McLaren Papaya Orange, Ferrari Rosso Corsa).
- **Pit Wall Environmental Telemetry**: Real-time track surface temperature, ambient air temperature, and rain probability radar metrics.
- **Interactive Lights-Out Start Sequence**: Simulates the authentic FIA 5-red-lights race start protocol with synchronized piezo tones and RGB flag signaling.
- **Touch Navigation Deck**:
  - `CIRCUIT`: Cycle through active Grand Prix circuits and race schedules
  - `STANDINGS`: Switch to current Championship driver leaderboard
  - `STATUS`: Display pit wall track telemetry and trigger race start sequence

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

