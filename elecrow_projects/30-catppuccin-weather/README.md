# Project 30: Catppuccin Weather Station

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Catppuccin Weather Station** is an elegant desktop weather telemetry appliance styled with the soothing, pastel Catppuccin Mocha aesthetic color palette. Built for zero-friction daily use, it leverages open weather schemas to deliver current atmospheric readings, 24-hour temperature and precipitation trend charts, and comprehensive 7-day outlook tables.

Equipped with dynamic ambient light sensing via the on-board Light Dependent Resistor (LDR), the display automatically dims in low-light room environments, making it ideal for nightstand or desk placement.

---

## Features

- **Catppuccin Mocha UI Architecture**:
  - Balanced 16-bit 565 pastel colors (Base `#1E1E2E`, Surface `#313244`, Peach `#FAB387`, Lavender `#B4BEFE`, Sapphire `#74C7EC`, Teal `#94E2D5`).
  - High-readability card layout designed for 320x240 QVGA.
- **Three Interactive Touch Views**:
  - **Now**: Large hero temperature, condition description, feels-like metric, humidity %, wind speed (km/h), barometric pressure (hPa), and UV index.
  - **24H Trend**: Hourly temperature spline curve combined with vertical bar graph columns for rain probability (%).
  - **7-Day Outlook**: Daily min/max temperature range bars, predicted condition tags, and precipitation percentages.
- **Automatic LDR Backlight Dimming**:
  - Continuously samples room ambient lux on `IO34` via ADC1.
  - Dynamically modulates the display backlight PWM on `IO21` from 10% to 100% duty cycle.
- **Ambient Weather Mood Lighting**:
  - Tri-color RGB LED (`IO4`, `IO16`, `IO17`) emits contextual ambient atmospheric light matching the current weather (warm yellow for sunny, cyan for overcast, deep blue for rain, magenta for severe storms).
- **Acoustic Touch Feedback**:
  - Emits gentle 1.8kHz/2.4kHz acoustic chirps on `IO26` upon tab switching or condition refreshes.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, LEDC Backlight PWM |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **Ambient LDR Sensor** | `GPIO 34` (ADC1_CH6) | Ambient light sensor for auto-dimming |
| **RGB Mood LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW atmospheric mood indicator |
| **Buzzer** | `GPIO 26` | Piezo acoustic feedback |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/30-catppuccin-weather
   pio run --target upload
   ```
3. Customize your latitude, longitude, and Wi-Fi credentials in `30-catppuccin-weather.ino` to fetch your local city's forecast directly via open API endpoints.
