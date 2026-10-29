# Project 46: Boids Artificial Life & 4D Tesseract Physics Sandbox

**Target Device:** Elecrow CrowPanel 2.8" ESP32 Display (`ESP32-2432S028R`)  
**Channel Branding:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## Overview

**Boids Artificial Life & 4D Tesseract Sandbox** is a high-performance procedural physics and generative mathematics engine for the Elecrow 2.8" CYD touch display. It combines a real-time 42-agent Craig Reynolds flocking algorithm (Separation, Alignment, Cohesion) with a perspective-projected 4-dimensional hypercube (Tesseract) wireframe renderer operating at up to 60 FPS. Users can interactively introduce predators or food beacons via resistive touch, alter 4D rotation hyperplanes, and adjust simulation physics directly on screen.

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
| **RGB LED (Red)** | `IO4` | GPIO | Active LOW Predator / Alert Indicator |
| **RGB LED (Green)** | `IO16` | GPIO | Active LOW Attractor / Cohesion Indicator |
| **RGB LED (Blue)** | `IO17` | GPIO | Active LOW 4D Rotation Phase Indicator |
| **Piezo Speaker** | `IO26` | DAC / PWM | Touch feedback tone generator |
| **LDR Photocell** | `IO34` | ADC1_CH6 | Ambient room brightness sensor |

---

## Features

1. **Dual Simulation Engines**:
   - **Mode 1: Craig Reynolds Boids Flocking**:
     - 42 autonomous boids computing vector forces in real time.
     - Separation rule: distance repulsion below 12px prevents collisions.
     - Alignment rule: heading matching with flock neighbors within 36px.
     - Cohesion rule: attraction to flock local center of mass.
     - Soft-margin boundary restitution keeping boids inside the display area.
   - **Mode 2: 4D Wireframe Tesseract Engine**:
     - Full 4D rotation across XW, YZ, and XY hyperplanes.
     - Perspective transformation from 4D to 3D and 3D to 2D screen coordinates.
     - 16 vertices and 32 connected edges with depth-cue color mapping.
2. **Interactive Touch Controls**:
   - Touch anywhere on the flocking screen to deploy an **Attractor (Food)** or **Predator (Hawk)** beacon.
   - Scatter button to randomize boid trajectories.
   - 4D drag gesture to manually accelerate hypercube rotation across hyperplanes.
   - Dedicated buttons to increase/decrease rotation speeds and rotate 4D axes by 90°.
3. **Optimized Frame Pacing**:
   - Zero-flicker vector line and circle clearing.
   - Built-in real-time FPS counter maintaining smooth animations.
   - RGB LED state synchronized with simulation physics.

---

## Quick Start & Build Instructions

### PlatformIO
1. Open this directory in VS Code or Terminal:
   ```bash
   cd elecrow_projects/46-boids-physics-toy
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

1. **Switch Views**: Tap the **VIEW: TESSERACT** or **VIEW: BOIDS** button in the top-right corner to toggle between the 2D flocking simulator and the 4D hypercube.
2. **Boids Mode**:
   - Tap **ATTRACTOR / PREDATOR** at the bottom-left to select beacon behavior.
   - Tap any point in the flock to drop a glowing beacon. Watch the boids swarm towards the food or flee in panic from the predator.
   - Tap **SCATTER** to disperse the flock into random headings.
3. **Tesseract Mode**:
   - Tap **SPEED +** or **SPEED -** to adjust rotational velocity.
   - Tap **ROT AXIS** to execute a 90° hyper-rotation step.
   - Drag a stylus across the screen to manually spin the 4D wireframe.
