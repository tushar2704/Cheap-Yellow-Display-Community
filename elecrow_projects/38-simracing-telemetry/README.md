# Project 38: Sim-Racing Telemetry Dash

A digital sim-racing dashboard and steering wheel telemetry terminal engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Bring GT3 and Formula-style telemetry directly to your racing simulator rig. This dedicated desktop instrument display features a 16-segment RPM shift light bar, an oversized central gear indicator with dynamic redline flash and acoustic shift chimes, live digital speedometer with unit toggling (KM/H / MPH), real-time millisecond lap delta tracking, and a 4-wheel tyre thermal matrix.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Shift Flash LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) for peripheral redline shift alerts
- **Buzzer**: Piezo buzzer on GPIO 26 for acoustic shift chirps and pit limiter audio

## Features
- **16-Segment Tachometer Shift Light Bar**:
  - Green (4,500 - 6,000 RPM) -> Amber (6,000 - 7,500 RPM) -> Red (7,500 - 8,000 RPM) -> Flashing Redline (> 8,000 RPM).
- **Giant Central Gear Indicator**:
  - Center stage gear readout (`R`, `N`, `1`..`7`) with flashing red background during shift points.
- **Speedometer & Lap Delta Engine**:
  - Dual unit support: `KM/H` and `MPH` with instant touch switching.
  - Live session delta indicator with color coding: Green for positive time gain, Red for time loss.
- **4-Wheel Tyre Thermal Matrix**:
  - Real-time temperature monitoring across Front-Left, Front-Right, Rear-Left, and Rear-Right tyres.
  - Heat zone color coding: Cold Blue (<75°C), Optimal Green (75-95°C), Warm Amber (95-105°C), Overheating Red (>105°C).
- **Interactive Sim Touch Deck**:
  - `DISCORD MUTE`: Visual mic status and audio toggle macro.
  - `UNIT: MPH / KM/H`: Immediate speed measurement conversion.
  - `PIT LIMITER`: Speed clamp toggle with warning colors.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
