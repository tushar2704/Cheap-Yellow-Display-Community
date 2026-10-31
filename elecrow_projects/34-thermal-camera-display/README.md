# Project 34: Local Thermal Imaging Camera

A standalone 32x24 thermal imaging and non-contact surface temperature inspection console engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Transform your CYD screen into a portable infrared thermal camera. Supporting external Melexis MLX90640 far-infrared sensor arrays via the external HY2.0 I2C bus, this diagnostic terminal provides real-time heat mapping with 768 thermal pixels, automated center crosshair target temperature calculation, maximum hotspot coordinate tracking, and user-switchable pseudocolor palettes.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **I2C Port**: External HY2.0 interface (`SDA=27`, `SCL=22`) running at 400 kHz for MLX90640
- **RGB Mood LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for overheat alerts and touch feedback

## Features
- **768-Point Thermal Heatmap**: Renders full 32x24 thermal frames with 7x7 pixel block scaling on the ILI9341 display.
- **Organic Thermal Simulation Fallback**: Automatically activates a dual-source dynamic thermal plume simulation when physical sensor hardware is not detected.
- **3 Color Palettes**:
  - `IRONBOW`: Classic FLIR-style ironbow palette (Black, Purple, Red, Yellow, White).
  - `RAINBOW`: High-dynamic range Turbo spectrum with crisp thermal separation.
  - `GRAYSCALE`: High-contrast intensity map ideal for electrical component inspection.
- **Target Reticles & Measurement Readouts**:
  - Center Crosshair: Real-time temperature readout of central object.
  - Hotspot Tracking: Automated bounding box and crosshair locking onto the hottest pixel in the frame.
  - Min / Max / Center temperature sidebar with vertical color gradient reference bar.
- **Touch Action Toolbar**:
  - `HOLD FRAME`: Freezes active frame for close inspection.
  - `PALETTE`: Cycles between Ironbow, Rainbow, and Grayscale shaders.
  - `HOTSPOT`: Toggles tracking reticle on the maximum temperature point.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

