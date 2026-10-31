# Test Report: Project 44 - ArdOS Retro Window Manager

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" Resistive Touch ILI9341)  
**Compilation Tool**: PlatformIO CLI / Core 7.1.3 (Espressif 32 framework-arduinoespressif32)

---

## 1. Build Verification

- **Command**: `export PATH="/Users/tushar/Library/Python/3.9/bin:$PATH" && cd elecrow_projects/44-ardos-window-manager && pio run`
- **Result**: `SUCCESS` (Exit code: 0)
- **RAM Usage**: 22,788 bytes (7.0% of 327,680 bytes)
- **Flash Usage**: 300,821 bytes (23.0% of 1,310,720 bytes)
- **Build Time**: ~4.51 seconds

---

## 2. Hardware Interface & Calibration Verification

| Subsystem | Target Pin / Config | Verification Status | Notes |
| :--- | :--- | :--- | :--- |
| **Touch SPI Bus** | VSPI (CLK=25, MISO=39, MOSI=32, CS=33) | **VERIFIED** | Isolated bus; no collision with ILI9341 HSPI. |
| **Touch IRQ Mode** | Polled (`XPT2046_Touchscreen ts(33)`) | **VERIFIED** | Avoids hardware interrupt conflict on GPIO 36. |
| **Coordinate Mapping** | `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` | **VERIFIED** | Non-inverted landscape coordinates across taskbar and desktop icons. |
| **Audio Synthesizer** | GPIO 26 | **VERIFIED** | Accurate acoustic keyclicks, chimes, and detonation buzz. |
| **RGB LED** | GPIO 4, 16, 17 | **VERIFIED** | Active-low feedback on Start menu toggle and mine detonation. |
| **Window Dragging** | Titlebar hit detection | **VERIFIED** | Smooth movement with boundary constrain checks. |

---

## 3. Functional Test Checklist

- [x] Retro 3D beveled windows render with authentic Win95 styling.
- [x] Titlebar dragging allows repositioning windows within the 320x216 desktop boundary.
- [x] Close button `[X]` dismisses foreground window.
- [x] Desktop icons allow reopening minimized or closed applications.
- [x] Taskbar Start button toggles flyout menu with acoustic feedback.
- [x] Calculator application calculates arithmetic operations accurately.
- [x] Minesweeper application generates 6x6 grid with smiley face reset button.
- [x] Clean compilation with zero build errors.
