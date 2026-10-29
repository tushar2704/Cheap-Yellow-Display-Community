# Project 34: Local Thermal Imaging Camera

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Local Thermal Imaging Camera** turns the Elecrow 2.8" CYD into a handheld thermal infrared diagnostics scanner. Utilizing the HY2.0 expansion header for hardware I2C communication with 32x24 Far-Infrared sensor arrays (such as the MLX90640), it captures and visualizes surface heat maps in real time.

The firmware features dynamic auto-ranging scale calibration, real-time center crosshair temperature sampling, automatic peak hot-spot tracking, three false-color rendering palettes, and an active overheat warning system.

---

## Features

- **32x24 IR Matrix Rendering**:
  - Interpolates 768 thermal pixels into a clean 224x168 thermal viewfinder with 7x7 display cells per sensor node.
  - Features high-fidelity thermal simulation fallback when no physical I2C sensor is attached.
- **Three False-Color Palettes**:
  - **IRONBOW**: Classic thermal imaging gradient (dark violet -> purple -> orange -> yellow -> white).
  - **RAINBOW (TURBO)**: High-contrast spectral map (blue -> cyan -> green -> yellow -> red).
  - **GRAYSCALE**: Monochrome linear radiance map for industrial inspections.
- **Dynamic Hot-Spot & Center Reticles**:
  - Center reticle with live center spot temperature (`CTR: 24.5°C`).
  - Active hot-spot target reticle that automatically tracks and follows the maximum temperature peak in the frame (`MAX: 38.5°C`).
- **Touch Viewfinder Controls**:
  - **FREEZE**: Instantly freezes/holds the thermal frame for close inspection.
  - **PALETTE**: Cycles across Ironbow, Rainbow, and Grayscale modes.
  - **HOT-SPOT**: Toggles on-screen maximum temperature target reticle.
- **Overheat Safety System**:
  - Flashes red on-board RGB LED (`IO4`) and triggers acoustic alarm chirps on `IO26` if surface temperature exceeds 50.0°C (122°F).

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **HY2.0 I2C Bus** | `GPIO 27` (SDA) / `22` (SCL) | Hardware I2C bus for MLX90640 |
| **RGB Overheat LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW thermal safety indicator |
| **Buzzer** | `GPIO 26` | Acoustic feedback and overheat alarm |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. (Optional) Connect an MLX90640 sensor module to the HY2.0 port (`IO27` SDA, `IO22` SCL, `3V3`, `GND`).
3. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/34-thermal-camera-display
   pio run --target upload
   ```
4. If a sensor is detected at address `0x33`, live I2C data is rendered; otherwise, the device runs in demo simulation mode for testing.
