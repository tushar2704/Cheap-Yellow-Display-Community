# Project 16: Live Coastal Tide Chart & Lunar Tracker

An interactive marine tide wave plotter and astronomical lunar tracker built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This project simulates real-time semi-diurnal harmonic tidal curves across a 24-hour diurnal cycle for major global harbors (SF Bay, Dover, Sydney, and Mumbai). It dynamically renders mean sea level datums, high and low water crests, lunar illumination percentages (spring vs. neap tides), and solar transitions with smooth resistive touch station navigation.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for touch chirp confirmation

## Features
- **Harmonic Sine Tide Curves**: Smooth double-line tidal wave plotting with mathematical semi-diurnal period modeling (12.42h).
- **Time Cursor & Water Level Gauge**: Live vertical time cursor with real-time level readout and rising/falling trend indicator.
- **Lunar & Solar Ephemeris**: Real-time lunar phase badge (Waxing Gibbous / Spring tide indicator) and sunrise/sunset markers.
- **Harbor Selector Tabs**: 4 quick-switch harbor tabs along the bottom bezel with non-inverted touch coordinates.
- **State LED Integration**:
  - Cyan: Rising / Flood tide
  - Blue: Falling / Ebb tide
  - Yellow: Slack water / Mid-tide transition

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

