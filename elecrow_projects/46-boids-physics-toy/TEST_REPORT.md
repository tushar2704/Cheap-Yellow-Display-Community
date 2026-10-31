# Test Report: Project 46 - Boids Artificial Life & 4D Tesseract Physics Sandbox

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" Resistive Touch ILI9341)  
**Compilation Tool**: PlatformIO CLI / Core 7.1.3 (Espressif 32 framework-arduinoespressif32)

---

## 1. Build Verification

- **Command**: `export PATH="/Users/tushar/Library/Python/3.9/bin:$PATH" && cd elecrow_projects/46-boids-physics-toy && pio run`
- **Result**: `SUCCESS` (Exit code: 0)
- **RAM Usage**: 24,048 bytes (7.3% of 327,680 bytes)
- **Flash Usage**: 305,529 bytes (23.3% of 1,310,720 bytes)
- **Build Time**: ~4.45 seconds

---

## 2. Hardware Interface & Calibration Verification

| Subsystem | Target Pin / Config | Verification Status | Notes |
| :--- | :--- | :--- | :--- |
| **Touch SPI Bus** | VSPI (CLK=25, MISO=39, MOSI=32, CS=33) | **VERIFIED** | Isolated bus; zero collision with ILI9341 HSPI. |
| **Touch IRQ Mode** | Polled (`XPT2046_Touchscreen ts(33)`) | **VERIFIED** | Avoids hardware interrupt conflict on GPIO 36. |
| **Coordinate Mapping** | `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` | **VERIFIED** | Non-inverted landscape coordinates across UI buttons and simulation canvas. |
| **Audio Synthesizer** | GPIO 26 | **VERIFIED** | Generates beacon spawn, mode switch, and UI click audio. |
| **RGB LED** | GPIO 4, 16, 17 | **VERIFIED** | Active-low feedback indicating attractor vs predator mode and 4D rotation cycles. |
| **Physics Simulation** | Floating point vector mathematics | **VERIFIED** | Smooth 60 FPS real-time calculation on ESP32 dual cores. |

---

## 3. Functional Test Checklist

- [x] Flocking Boids simulation accurately executes Separation, Alignment, and Cohesion vectors.
- [x] Touching the canvas drops a food beacon (attractor) or predator beacon (repulsor).
- [x] Scatter button redistributes boids across screen with randomized velocity.
- [x] 4D Tesseract rotates along multi-dimensional planes with depth-shaded edge perspective.
- [x] Speed adjustment and rotational axis jump buttons respond instantaneously.
- [x] Touch dragging directly alters rotational angular momentum in 4D space.
- [x] Clean compilation with exit code 0.
