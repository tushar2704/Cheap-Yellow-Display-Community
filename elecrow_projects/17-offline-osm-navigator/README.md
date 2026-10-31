# Project 17: Offline OSM Trail Navigator & Topo Deck

An offline handheld topographic navigation unit and trail waypointer built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Designed for zero-connectivity hiking and expedition navigation, this handheld system renders procedural and SD-backed topographic maps with vector elevation contours, water boundaries, and winding hiking trails. Large, high-target glove controls on the right bezel provide intuitive zoom and pan commands, alongside single-tap waypoint drop flags.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **MicroSD Interface (HSPI)**: `SD_CS=5`, `SD_MOSI=23`, `SD_MISO=19`, `SD_SCK=18`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for waypoint and pan/zoom tactile chimes

## Features
- **Offline Vector Topo Engine**: Procedural contour line generator simulating topographic elevation curves, dynamic river channels, and dashed trail pathways.
- **MicroSD Card Auto-Detection**: Probes the onboard TF slot over HSPI for tile sets; falls back seamlessly to internal memory vector mode when no card is present.
- **Glove-Optimized Controls**: High-contrast, large-target touch controls on the right margin for Zoom In (+), Pan Up (▲), Pan Down (▼), Zoom Out (-), and Waypoint Pin (WP).
- **Interactive Waypoint Engine**: Tap anywhere on the active map canvas to center pan, or press WP to mark points of interest (campsite, summits).
- **Trip Telemetry**: Live readout of Latitude, Longitude, Altitude MSL, Trip Distance (km), and compass heading.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
