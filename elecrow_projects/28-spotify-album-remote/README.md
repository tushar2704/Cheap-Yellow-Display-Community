# Project 28: Spotify Album Art & Media Remote

A dedicated desktop music playback terminal and ambient vinyl art player built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This desktop controller provides intuitive touch control for streaming audio platforms like Spotify Connect. Featuring a high-contrast dark theme, it showcases full-color album artwork stylized with an animated vinyl groove disc, tracks elapsed playback time with a precision timeline scrubber, and syncs ambient RGB backlighting to the dominant hue of the active album jacket.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for tactile media click chirps

## Features
- **Album Artwork & Vinyl Styling**: Renders a dedicated 100x100 album jacket border with concentric vinyl grooves and album-matched center spindle colors.
- **Dynamic Chromatic Mood Lighting**: Active-low RGB LED projects matching lighting backdrops synchronized with current song palettes (Magenta, Red, Gold, Cyan).
- **Progress Timeline Scrubber**: Microsecond-accurate playback timeline updater redrawing solely elapsed progress elements to prevent screen flicker.
- **Dedicated Media Touch Deck**:
  - `|<<` Skip Previous track
  - `PLAY / PAUSE` Toggle playback state with status color changes
  - `>>|` Skip Next track
  - `-` / `+` Volume attenuation and gain control

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
