# Project 30: Catppuccin Weather Station

A modern desktop micro-climate terminal designed with the pastel Catppuccin Mocha aesthetic palette for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This compact ambient workstation appliance brings modern minimalist interface design to physical desk hardware. Featuring the iconic Catppuccin Mocha color theme, the station provides instantaneous readings, 24-hour weather trends with continuous temperature curves and precipitation bar charts, and a 7-day extended outlook. Integrated hardware photoresistor monitoring automatically adjusts display backlight intensity to ambient lighting conditions.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **Ambient Light Sensor**: Onboard LDR on ADC GPIO 34 driving IO21 backlight PWM
- **RGB Mood LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) reflecting atmospheric conditions
- **Buzzer**: Piezo buzzer on GPIO 26 for tactile UI feedback chirps

## Features
- **Catppuccin Mocha Styling**: Custom 16-bit 565 palette matching authentic Catppuccin accents (Subtext, Lavender, Peach, Sapphire, Mauve).
- **Multi-Tab Touch Architecture**:
  - `NOW`: Primary dashboard card presenting temperature, feels-like, UV index, humidity, wind velocity, atmospheric pressure, and auto-dim duty cycle.
  - `24H TREND`: High-resolution analytical graph displaying temperature slope splines alongside precipitation probability volume columns.
  - `7-DAY OUTLOOK`: Weekly forecast card with daily condition summaries, precipitation likelihood, and normalized min/max temperature gradient bars.
- **Dynamic LDR Ambient Auto-Dimming**: Real-time ADC light sensing dynamically adjusts PWM backlight between bright room and darkroom levels.
- **Atmospheric Chromatic Feedback**: RGB status LED changes hue in real-time according to sky conditions (Clear sunny amber, Cloudy sky cyan, Rain blue, Thunderstorm violet).

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

