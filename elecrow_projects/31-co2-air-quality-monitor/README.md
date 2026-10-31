# Project 31: CO2 Indoor Air Quality Monitor

A standalone desk air quality and CO2 safety console engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Elevated CO2 concentrations directly degrade cognitive function, focus, and sleep quality. This dedicated environmental station connects to Sensirion photoacoustic NDIR sensors (SCD40/SCD41) over the CYD external HY2.0 I2C bus, delivering continuous non-dispersive infrared carbon dioxide, temperature, and relative humidity telemetry with an integrated historical trend engine and multi-level acoustic/chromatic safety alerts.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **I2C Port**: External HY2.0 connector (`SDA=27`, `SCL=22`) for Sensirion SCD4x
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for hazardous threshold alarms and touch feedback

## Features
- **Photoacoustic NDIR Telemetry**: High-precision carbon dioxide sensing with hardware CRC-8 verification.
- **Simulation Fallback Engine**: Seamlessly transitions to realistic atmospheric fluctuation simulation when external sensor hardware is not connected.
- **Air Quality Classification**:
  - `< 800 ppm`: Excellent Air (Vibrant Green)
  - `800 - 1200 ppm`: Moderate Air (Warm Yellow)
  - `1200 - 1600 ppm`: Poor Ventilation (Warning Orange)
  - `> 1600 ppm`: Hazard Alert (Blinking Red + Acoustic Siren)
- **48-Point Historical Trend Chart**: Live scrolling trend line with horizontal threshold indicators showing indoor carbon dioxide accumulation over time.
- **Interactive Touch Control Deck**:
  - `UNIT: °C / °F`: Instant toggle between Metric and Imperial temperature display
  - `ALARM: ON / MUTED`: Silences acoustic alarms while maintaining optical alerts
  - `REFRESH`: Forces immediate sensor interrogation and display update

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
