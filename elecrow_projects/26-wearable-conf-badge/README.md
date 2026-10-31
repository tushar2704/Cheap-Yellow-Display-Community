# Project 26: Wearable Tech Conference Badge & vCard Deck

An interactive wearable electronic name badge, vCard contact exchanger, and crowd telemetry monitor built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This wearable smart badge replaces passive paper conference tags with an interactive full-color display for tech summits and hackathons. It features a VIP Speaker credentials badge, an automated contact exchange QR code encoding a full standard vCard 3.0 specification for rapid smartphone address book imports, LiPo battery fuel gauging, and dynamic ambient sensor monitoring.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **Ambient Light Sensor**: LDR on GPIO 34 (analog read)
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for touch chirps

## Features
- **VIP Speaker Badge**: High-contrast dark badge layout displaying speaker name, channel handle (`@IamNotIntoTech`), talk title, and stage room scheduling.
- **Instant vCard QR Exchanger**: Renders a dense Version 4 QR code directly on the TFT; iOS Camera and Google Lens automatically parse and import full contact cards with zero cellular connectivity required.
- **Ambient Sensor & Crowd Deck**: Real-time ambient illumination lux sensor readouts alongside 2.4GHz beacon density metrics.
- **Active-Low RGB Mood Lighting**: Cycles through a 6-phase neon ambient glow (Red, Yellow, Green, Cyan, Blue, Purple) to match event atmospheres.
- **Touch Navigation**: Fast switching between Badge view, QR code, and Telemetry cards with anti-jitter debouncing.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

