# Project 33: Streaming Internet Lo-Fi Radio

An ambient desktop streaming audio terminal and real-time audio visualizer engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
A dedicated desktop radio and audio player appliance delivering chill study beats, retrowave tracks, and ambient drone streams. It features an integrated 16-band audio spectrum analyzer with gravity peak hold, three switchable visualization shaders, stream metadata telemetry, rhythmic bass pulse chromatic LED feedback, and full touch playback controls.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Mood LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) pulsing to sub-bass energy
- **Buzzer**: Piezo buzzer on GPIO 26 for tactile UI feedback chirps

## Features
- **16-Band Real-Time Audio Visualizer**: 
  - Dynamic fast-decay spectrum analyzer with simulated FFT organic response.
  - Authentic gravity peak hold dropping lines for physical spectrum realism.
- **3 Visualizer Display Engines**:
  - `BARS`: Quad-color segmented neon equalizer bars (Cyan, Green, Yellow, Magenta).
  - `DOTS`: High-contrast cybernetic dot-matrix frequency constellation.
  - `FIRE`: Solid color-blended thermal wave visualizer with glowing peaks.
- **Station Carousel & Telemetry**:
  - Pre-configured lo-fi and study streams (ChilledCow Lofi Beats, Synthwave Radio 80s, Deep Focus Ambient, Cyberpunk Electro Hub).
  - Live bitrate, streaming format, and title metadata readouts.
- **Rhythmic Bass Chromatic Feedback**:
  - Active-low RGB LED evaluates instantaneous sub-bass energy and emits synchronized pulses.
- **Touch Control Deck**:
  - `|<<` / `>>|`: Instant station switching
  - `PLAY / PAUSE`: Audio stream engine toggle
  - `MODE`: Cycles between Bars, Dots, and Fire visualizer modes
  - `VOL`: Step through volume gain levels (20% to 100%)

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

