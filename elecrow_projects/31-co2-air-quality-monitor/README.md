# Project 31: CO2 Indoor Air Quality Monitor

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **CO2 Indoor Air Quality Monitor** is a dedicated environmental safety monitor that measures real-time Carbon Dioxide concentration in parts-per-million (ppm), ambient temperature, and relative humidity. Utilizing the HY2.0 4-pin expansion port for I2C communication with photoacoustic NDIR sensors (such as the Sensirion SCD40 / SCD41), it acts as an intelligent room ventilation sentinel.

When CO2 levels climb into hazardous ranges (> 1400 ppm), the device automatically flashes an intense visual red warning and sounds an acoustic alarm to remind you to open windows and refresh indoor air.

---

## Features

- **Sensirion SCD4x NDIR I2C Interface**:
  - Connects to hardware I2C on the HY2.0 expansion header (`IO27` SDA, `IO22` SCL).
  - Implements native Sensirion CRC-8 checksum verification for robust communication.
  - Features seamless automatic fallback to High-Fidelity Simulation Mode if no physical sensor is attached.
- **Color-Coded Air Quality Index**:
  - **< 800 ppm (Green)**: Fresh outdoor air / optimal cognitive performance.
  - **800 - 1200 ppm (Yellow)**: Moderate ventilation / typical occupied office.
  - **1200 - 1600 ppm (Orange)**: Poor air circulation / potential drowsiness.
  - **> 1600 ppm (Red)**: Hazardous stale air / "VENTILATE ROOM NOW" warning.
- **24-Hour Trend Chart**:
  - 48-point rolling historical trend graph displaying historical room ventilation patterns with 800 ppm and 1200 ppm threshold reference lines.
- **Acoustic & RGB Safety Alarm**:
  - Flashes red on-board RGB LED (`IO4`) and sounds recurring 2.6kHz tone pulses on `IO26` when threshold is exceeded.
- **Touch Controls**:
  - **UNIT**: Instant toggle between Celsius (°C) and Fahrenheit (°F).
  - **ALARM**: One-touch mute switch to silence acoustic chirps while preserving visual LED warnings.
  - **REFRESH**: Forces an immediate I2C poll cycle.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **HY2.0 I2C Port** | `GPIO 27` (SDA) / `22` (SCL) | Hardware I2C bus for SCD4x sensor |
| **RGB Status LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW air safety indicator |
| **Alarm Buzzer** | `GPIO 26` | Piezo acoustic alarm for stale air warnings |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. (Optional) Connect your SCD40/SCD41 sensor to the HY2.0 4-pin port (`IO27` SDA, `IO22` SCL, `3V3`, `GND`).
3. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/31-co2-air-quality-monitor
   pio run --target upload
   ```
4. If a sensor is connected, the display displays the `SCD4x NDIR` badge; otherwise, it seamlessly runs in demo simulation mode for testing.
