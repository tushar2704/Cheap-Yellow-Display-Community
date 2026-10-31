# Project 19: Wi-Fi Channel & Spectrum Auditor

A handheld 2.4GHz Wi-Fi spectrum analyzer and channel congestion optimizer built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This standalone network appliance scans the full 2.4GHz Wi-Fi spectrum (Channels 1 through 13/14) using the ESP32 radio hardware. It renders real-time overlapping parabolic transmission curves, signal strength waterfalls (-100 to -30 dBm), an AP count density overlay, and calculates the optimal least-interfered non-overlapping channel (1, 6, or 11).

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for scan sweep tones and touch confirmation

## Features
- **RF Spectrum Parabola Visualizer**: Renders proportional 20MHz bandwidth curves centered over each channel frequency with peak RSSI heights.
- **Interference Recommendation Engine**: Automatically analyzes channel overlap, adjacent channel bleeding, and peak transmitter RSSI to recommend the cleanest channel.
- **Detailed AP Table**: Toggles into a structured table view reporting SSID, operating Channel, signal strength (dBm), and security encryption mode (WPA2, WPA3, OPEN).
- **Traffic Congestion LED**:
  - Green: Clean airspace (< 8 nearby APs)
  - Yellow: Moderate traffic (8–18 APs)
  - Red: Heavy saturation (> 18 APs)
- **Acoustic Scan Sweep**: Dual-tone frequency sweep upon scan completion.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
