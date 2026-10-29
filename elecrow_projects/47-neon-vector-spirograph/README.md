# Project 47: Neon Vector Spirograph & Mandala Generator

**Target Device:** Elecrow CrowPanel 2.8" ESP32 Display (`ESP32-2432S028R`)  
**Channel Branding:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## Overview

**Neon Vector Spirograph & Mandala Generator** is an interactive mathematical visualizer and digital kaleidoscopic canvas designed for the Elecrow 2.8" CYD touch display. It combines a parametric hypotrochoid / epitrochoid curve generation engine with a real-time N-fold symmetry (4x, 6x, 8x, 12x) touch-drawing mandala canvas. Users can switch between hypnotic automated spirographs with cycling neon color palettes and freeform stylus sketching where every stroke is mirrored across geometric rotational axes.

---

## Key Hardware Pinout

| Function | ESP32 Pin | Interface / Bus | Description |
| :--- | :--- | :--- | :--- |
| **TFT MOSI** | `IO13` | SPI | ILI9341 Serial Data |
| **TFT MISO** | `IO12` | SPI | ILI9341 Data In |
| **TFT SCLK** | `IO14` | SPI | ILI9341 Clock |
| **TFT CS** | `IO15` | GPIO / SPI | Display Chip Select |
| **TFT DC** | `IO2` | GPIO | Display Command / Data |
| **TFT Backlight** | `IO21` | GPIO | Active HIGH Backlight control |
| **Touch CS** | `IO33` | VSPI | XPT2046 Touch Controller Chip Select |
| **Touch IRQ** | `IO36` | GPIO | XPT2046 Touch Interrupt |
| **RGB LED (Red)** | `IO4` | GPIO | Active LOW Hue Cycling Indicator |
| **RGB LED (Green)** | `IO16` | GPIO | Active LOW Hue Cycling Indicator |
| **RGB LED (Blue)** | `IO17` | GPIO | Active LOW Hue Cycling Indicator |
| **Piezo Speaker** | `IO26` | DAC / PWM | Touch and harmonic frequency generator |
| **LDR Photocell** | `IO34` | ADC1_CH6 | Ambient room brightness sensor |

---

## Features

1. **Parametric Spirograph Generator (Hypotrochoid)**:
   - Evaluates continuous trigonometric curves:
     $$x(\theta) = (R - r)\cos(\theta) + d\cos\left(\frac{R - r}{r}\theta\right)$$
     $$y(\theta) = (R - r)\sin(\theta) - d\sin\left(\frac{R - r}{r}\theta\right)$$
   - Pre-loaded harmonic mathematical presets (8-Petal, Complex Star 13, Harmonic Rosetta, Double Bloom, Trifoil).
2. **Interactive Touch Mandala Canvas**:
   - Real-time rotational symmetry drawing with 4x, 6x, 8x, or 12x mirror planes.
   - Dual-pixel stroke interpolation for anti-aliased appearance.
3. **Dynamic Neon Palette Engine**:
   - 4 built-in color themes: Neon Rainbow, Cyberpunk (Cyan/Hot-Pink), Sunset Fire, and Emerald Acid.
   - Smooth HSV-to-RGB565 mathematical interpolation along continuous vector strokes.
4. **Hardware Multi-Sensory Feedback**:
   - Active RGB LED cycles through vibrant color spectra synchronized with the drawing beam.
   - Piezo tone generator on IO26 outputs acoustic feedback during parameter shifts.

---

## Quick Start & Build Instructions

### PlatformIO
1. Open this directory in VS Code or Terminal:
   ```bash
   cd elecrow_projects/47-neon-vector-spirograph
   ```
2. Build firmware:
   ```bash
   pio run
   ```
3. Upload to CrowPanel 2.8" CYD via USB-C:
   ```bash
   pio run --target upload
   ```

---

## Usage Guide

1. **Auto Spirograph Mode**:
   - The engine automatically computes and traces harmonic curves in real time.
   - Tap **PRESET** to cycle through 5 distinct geometric configurations.
   - Tap **PALETTE** to switch color schemes.
   - Tap **CLEAR** to wipe the canvas and restart the current curve.
2. **Touch Mandala Mode**:
   - Tap **TOUCH MANDALA** in the top-right corner.
   - Drag a stylus or finger across the black center viewport. Your strokes will instantly replicate across rotational symmetry axes.
   - Tap **SYM: Nx** to cycle symmetry folding between 4x, 6x, 8x, and 12x.
