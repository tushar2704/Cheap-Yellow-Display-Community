# Project 46: Boids Artificial Life & 4D Tesseract Physics Sandbox

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

**Boids Artificial Life & 4D Tesseract Physics Sandbox** demonstrates emergent swarm robotics and multi-dimensional geometric physics simulation on the Cheap Yellow Display (CYD). The application features two interactive real-time simulation engines:
1. **Flocking Boids Swarm Simulation**: 42 independent agents executing Reynolds' artificial life flocking algorithms (Separation, Alignment, Cohesion) with interactive touch beacon attraction (food) or repulsion (predator hawk).
2. **4D Hypercube (Tesseract) Projection Engine**: 16 vertices and 32 edges rotating across multi-dimensional planes (XW, YZ, XY) projected down to 3D and 2D perspective display with depth tinting and variable rotation physics.

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
| **Buzzer** | GPIO 26 | Beacon spawn and UI interaction tones |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | State indicator (Active LOW) |

---

## Features & Controls

1. **Top Navigation Bar**:
   - Displays `@IamNotIntoTech` channel branding.
   - Mode Toggle button (`BOIDS FLOCK` / `4D TESSERACT`) instantly switches between physical simulation domains.

2. **Boids Simulation Controls**:
   - **ATTRACTOR / PREDATOR**: Toggles beacon mode between food attractant (green) and predator repulsor (red).
   - **SCATTER**: Re-seeds boid velocity and position vectors across the canvas.
   - **Canvas Touch**: Drops a timed beacon that commands swarm agents to flock towards or scatter away from the touch coordinates.

3. **4D Tesseract Controls**:
   - **SPEED - / SPEED +**: Dynamically alters angular velocity across 4D rotation matrices.
   - **ROT AXIS**: Applies a 90-degree orthogonal jump in 4D space.
   - **Interactive Drag**: Dragging stylus across canvas applies direct torque to multi-dimensional rotation angles.
   - **Live FPS Gauge**: Real-time render loop counter displayed on the bottom status bar.

4. **Hardware SPI Isolation & Non-Inverted Y Calibration**:
   - Touch controller operates on isolated VSPI pins (32, 39, 25, 33) preventing bus collisions with the display.
   - Calibrated landscape coordinates deliver accurate stylus response across all buttons and canvas regions.

---

## Building and Flashing

Build and upload using PlatformIO:

```bash
cd elecrow_projects/46-boids-physics-toy
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```
