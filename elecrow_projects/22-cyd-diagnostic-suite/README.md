# Project 22: CYD Hardware Factory Diagnostic Suite

A complete hardware factory acceptance testing and component diagnostic dashboard built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This diagnostic utility validates all integrated peripherals on the ESP32 Cheap Yellow Display. It features four dedicated testing modes: an ESP32 SoC telemetry inspector, full-gamut RGBW pixel burn-in pattern with geometric alignment rings, a 5-point XPT2046 digitizer crosshair calibration screen with raw vector readouts, and a hardware bench tester exercising the LDR light sensor, onboard piezo speaker (300Hz–3.2kHz sweep), and tricolor RGB LED.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **Light Sensor**: LDR on GPIO 34 (analog read)
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for frequency verification sweeps

## Features
- **SoC Telemetry & Checklists**: Inspects chip revision, flash memory geometry, hardware MAC address, and ESP-IDF firmware version.
- **Pixel & Geometry Burn-In**: 8-bar full-gamut RGBW test patterns for identifying dead subpixels alongside concentric circle alignment checks.
- **Digitizer Calibration Grid**: 5 crosshair targets (corners and center) with real-time raw ADC `(X, Y, Z)` readouts and live touch tracking dots.
- **Component Bench Station**:
  - Live analog bargraph and percentage readout from the ambient LDR light sensor.
  - Multi-tone acoustic audio sweep exercising the onboard piezo transducer.
  - Sequenced R-G-B-White LED test cycling the onboard active-low LED channels.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
