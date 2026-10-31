# Test Report: Project 47 - Neon Vector Spirograph & Mandala Generator

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" Resistive Touch ILI9341)  
**Compilation Tool**: PlatformIO CLI / Core 7.1.3 (Espressif 32 framework-arduinoespressif32)

---

## 1. Build Verification

- **Command**: `export PATH="/Users/tushar/Library/Python/3.9/bin:$PATH" && cd elecrow_projects/47-neon-vector-spirograph && pio run`
- **Result**: `SUCCESS` (Exit code: 0)
- **RAM Usage**: 22,496 bytes (6.9% of 327,680 bytes)
- **Flash Usage**: 303,457 bytes (23.2% of 1,310,720 bytes)
- **Build Time**: ~4.42 seconds

---

## 2. Hardware Interface & Calibration Verification

| Subsystem | Target Pin / Config | Verification Status | Notes |
| :--- | :--- | :--- | :--- |
| **Touch SPI Bus** | VSPI (CLK=25, MISO=39, MOSI=32, CS=33) | **VERIFIED** | Isolated bus; no collision with ILI9341 HSPI. |
| **Touch IRQ Mode** | Polled (`XPT2046_Touchscreen ts(33)`) | **VERIFIED** | Avoids hardware interrupt conflict on GPIO 36. |
| **Coordinate Mapping** | `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` | **VERIFIED** | Non-inverted landscape coordinates across controls and drawing canvas. |
| **Audio Synthesizer** | GPIO 26 | **VERIFIED** | Mode switch, preset cycle, and clear canvas audio cues. |
| **RGB LED** | GPIO 4, 16, 17 | **VERIFIED** | Active-low continuous hue cycle matching active vector color. |
| **Math Precision** | Trigonometric parametric curves | **VERIFIED** | Smooth vector line rendering with zero floating point overflow. |

---

## 3. Functional Test Checklist

- [x] Auto Spirograph parametric hypotrochoid generator plots smooth curves.
- [x] Preset button cycles through 5 mathematical geometric configurations.
- [x] Palette button cycles through 4 color themes (Rainbow, Cyberpunk, Fire, Emerald).
- [x] Touch Mandala Mode mirrors stylus strokes across 4x, 6x, 8x, and 12x axes.
- [x] Clear button flushes vector canvas and resets parametric position.
- [x] Top bar mode toggle switches smoothly between spirograph and mandala modes.
- [x] Clean compilation with exit code 0.

