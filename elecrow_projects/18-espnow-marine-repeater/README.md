# Project 18: ESP-NOW Marine Instrument Repeater

A zero-router peer-to-peer marine navigation instrument repeater built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This wireless marine cockpit deck connects to masthead and hull transducers via low-latency 2.4GHz ESP-NOW packets without needing an onboard Wi-Fi router. It renders an analog 360° apparent wind polar rose, live wind velocity, ultrasonic water depth with shallow depth alarm thresholds, atmospheric barometric pressure, and water temperature.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for shallow water audio alerts and touch clicks

## Features
- **360° Polar Wind Rose**: Visual apparent wind angle dial with color-coded port (red) and starboard (green) needles and centralized yacht icon.
- **Transducer Telemetry Cards**:
  - Ultrasonic Water Depth (feet / meters) with critical shallow water threshold detection.
  - Atmospheric Barometer (hPa) for squall and cold-front alerting.
  - Water Temperature (°F / °C).
- **Night Vision Preservation**: High-contrast, all-red nocturnal palette toggle preventing night blindness during ocean navigation.
- **Unit Conversion**: Instant toggle between Nautical (Knots/Feet/°F) and Metric (m/s/Meters/°C) modes.
- **Hardware Alarm Indicator**:
  - Solid Red: Critical shallow water depth (< 2.5m)
  - Yellow: Gale warning (> 22 knots)
  - Green: Safe cruising conditions

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
