# Project 47: Neon Vector Spirograph & Mandala Generator

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

The **Neon Vector Spirograph & Mandala Generator** transforms the 2.8" Cheap Yellow Display (CYD) into an interactive vector graphics synthesizer and rotational symmetry drawing studio. Featuring high-precision hypotrochoid parametric calculations, multi-palette neon cycling, and a real-time touch mandala mirror engine supporting 4x, 6x, 8x, and 12x radial symmetry, this project blends mathematics with embedded creative art.

---

## Key Hardware Pinout

| Peripheral | Hardware Pin | Purpose |
| :--- | :--- | :--- |
| **TFT Driver** | HSPI (Pins 12, 13, 14, 15, 2) | 320x240 ILI9341 Color Display |
| **TFT Backlight** | GPIO 21 | Backlight digital control |
| **Touch MOSI** | GPIO 32 | Dedicated VSPI Data In for XPT2046 |
| **Touch MISO** | GPIO 39 | Dedicated VSPI Data Out |
| **Touch CLK** | GPIO 25 | Dedicated VSPI Bus Clock |
| **Touch CS** | GPIO 33 | Chip Select for Touch Controller |
| **Buzzer** | GPIO 26 | Harmonic frequency feedback tones |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Dynamic color hue synchronization (Active LOW) |

---

## Operating Modes & Features

1. **Auto Spirograph Mode**:
   - Continuous vector generation plotting parametric hypotrochoid curves:
     $$x(\theta) = (R - r)\cos(\theta) + d\cos\left(\frac{R - r}{r}\theta\right)$$
     $$y(\theta) = (R - r)\sin(\theta) - d\sin\left(\frac{R - r}{r}\theta\right)$$
   - Built-in geometric presets:
     - Hypotrochoid 8-Petal
     - Complex Star 13
     - Harmonic Rosetta
     - Double Bloom
     - Classic Trifoil

2. **Touch Mandala Mode**:
   - Stylus-driven radial drawing canvas.
   - Selectable symmetry fold count: **4x**, **6x**, **8x**, and **12x** rotational mirrors.
   - Dual-line brush thickness for vivid vector strokes.

3. **Dynamic Color Engines**:
   - **Neon Rainbow**: 360-degree HSV smooth cyclic gradient.
   - **Cyberpunk**: Alternating high-contrast electric cyan and hot pink.
   - **Fire**: Heat spectrum across deep red, ember orange, and bright yellow.
   - **Emerald**: Neon lime and forest emerald spectrum.

4. **Hardware SPI Bus Isolation**:
   - VSPI bus on pins 32, 39, 25, 33 ensures zero interference with display refresh cycles.
   - Calibrated non-inverted landscape coordinates deliver accurate stylus response across all buttons and canvas regions.

---

## Building and Flashing

Build and upload using PlatformIO:

```bash
cd elecrow_projects/47-neon-vector-spirograph
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```

