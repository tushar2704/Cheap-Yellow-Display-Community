# Project 35: Solar & Battery Flow Dashboard

An interactive renewable energy flow monitor and homelab power distribution console engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Keep track of residential solar production, home energy consumption, battery storage states, and utility grid exchanges on a dedicated desktop display. Featuring animated directional energy particle streams, dual display views (animated topology network and 24-hour balance bar chart), and live grid self-sufficiency status lighting, this terminal transforms abstract smart meter telemetry into a tangible visual dashboard.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Mood LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) reflecting grid self-sufficiency
- **Buzzer**: Piezo buzzer on GPIO 26 for tactile UI feedback chirps

## Features
- **Dynamic Energy Topology**:
  - Central Inverter hub orchestrating power flows across Solar PV, Battery, Home Load, and Grid nodes.
  - Directional animated energy packet particles flowing continuously between active circuits.
- **24-Hour Comparative Balance Chart**:
  - Two-hour interval stacked/paired analytical bar graphs comparing Solar yield against Household draw.
- **Key Power Telemetry Cards**:
  - Solar Generation: Real-time generation output (kW) and cumulative daily yield (kWh).
  - Battery Storage: Dynamic State of Charge (%) with directional charge/discharge wattage.
  - Home Draw: Total household electrical demand.
  - Grid Net: Real-time utility exchange (+kW exporting, -kW importing).
- **Grid Self-Sufficiency Indicator**:
  - Green: 100% self-sufficient / exporting excess solar power to utility.
  - Amber / Yellow: Self-contained home powered from stored battery reserves.
  - Red: Grid import dependent.
- **Touch View Navigation**:
  - Single-tap view toggle button (`VIEW: HISTORY` / `VIEW: FLOW`).

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
