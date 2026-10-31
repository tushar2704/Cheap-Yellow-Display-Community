# Project 20: Privacy Shield BLE Tracker Detector

A handheld passive BLE privacy sniffer and stalker tag detector built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This pocket security appliance continuously scans 2.4GHz Bluetooth Low Energy advertisement packets to intercept unauthorized payload signatures from Apple Find My (AirTag), Samsung SmartTag, and Tile personal trackers. It projects real-time distance estimations on a radar sweep display, issues audible Geiger-counter proximity clicks, and flags suspicious beacons following the user.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for Geiger pulses and touch confirmation

## Features
- **Proximity Radar Display**: Rotating green phosphor sweep with concentric distance ranges (2m, 5m, 10m) displaying color-coded blips for detected tags.
- **Multi-Brand Identifier**: Inspects BLE manufacturer data payload headers to distinguish Apple AirTag (0x004C), Samsung SmartTag (0x0075), and Tile (0x00D6) devices.
- **Following Tag Threat Alert**: Flags trackers observed across persistent scan cycles (>45 seconds) in bright red as potential stalking threats.
- **Geiger Proximity Audio**: Rapid acoustic clicking with repetition rate dynamically modulated by proximity distance.
- **Tag Inspection Table**: Detailed list view showing Tag Type, MAC Address, RSSI, and estimated physical separation distance in meters.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

