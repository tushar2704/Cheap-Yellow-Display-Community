# Project 27: Offline Survival Encyclopedia & SOS Beacon

A 100% offline emergency survival reference handbook and optical/acoustic SOS distress beacon built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Built for emergency wilderness preparedness and off-grid resilience, this pocket handbook stores critical life-saving protocols in non-volatile flash memory and optional MicroSD storage without requiring any internet or cellular connectivity. It provides step-by-step guidance across four vital domains (First Aid, Water Purification, Knot Tying, and Solar Navigation) and integrates an active optical/acoustic International Morse Code SOS beacon (`... --- ...`).

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **MicroSD Interface (HSPI)**: `SD_CS=5`, `SD_MOSI=23`, `SD_MISO=19`, `SD_SCK=18`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for SOS audio pulsing and touch feedback

## Features
- **4 Vital Survival Modules**:
  - **First Aid**: Adult CPR compression protocols and severe arterial bleed tourniquet application.
  - **Water Purification**: Rolling boil thermal safety rules and SODIS UV-A solar disinfection guidelines.
  - **Knots & Rigging**: Non-slip rescue Bowline loops and adjustable friction Taut-Line hitches.
  - **Wilderness Navigation**: Shadow-stick solar compass tracking and distress ground signaling.
- **Automated Morse SOS Beacon**: Emits synchronized high-intensity optical LED white strobes and high-pitch 1.8kHz acoustic bursts matching international distress timing (3 short, 3 long, 3 short).
- **Dual-Storage Engine**: Boots instantly from internal flash memory while mounting external FAT32 MicroSD storage over HSPI when present.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
