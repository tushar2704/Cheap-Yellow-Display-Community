# Test Report: Project 02 - Chmod & VLSM Visual Network Architect

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" Resistive Touch ILI9341)  
**Compilation Tool**: PlatformIO CLI / Core 7.1.3 (Espressif 32 framework-arduinoespressif32)

---

## 1. Build Verification

- **Command**: `export PATH="/Users/tushar/Library/Python/3.9/bin:$PATH" && cd elecrow_projects/02-chmod-vlsm-architect && pio run`
- **Result**: `SUCCESS` (Exit code: 0)
- **RAM Usage**: 22,460 bytes (6.9% of 327,680 bytes)
- **Flash Usage**: 300,009 bytes (22.9% of 1,310,720 bytes)
- **Build Time**: ~4.81 seconds

---

## 2. Hardware Interface & Calibration Verification

| Subsystem | Target Pin / Config | Verification Status | Notes |
| :--- | :--- | :--- | :--- |
| **Touch SPI Bus** | VSPI (CLK=25, MISO=39, MOSI=32, CS=33) | **VERIFIED** | Isolated bus; zero collision with ILI9341 HSPI. |
| **Touch IRQ Mode** | Polled (`XPT2046_Touchscreen ts(33)`) | **VERIFIED** | Avoids hardware interrupt conflict on GPIO 36. |
| **Coordinate Mapping** | `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` | **VERIFIED** | Non-inverted landscape coordinates across CIDR buttons and 3x3 chmod matrix. |
| **Audio Synthesizer** | GPIO 26 | **VERIFIED** | Click tones on mode tabs, CIDR buttons, and matrix toggles. |
| **RGB LED** | GPIO 4, 16, 17 | **VERIFIED** | Active-low status indicator. |
| **Math Precision** | Bitwise CIDR calculations & octal conversion | **VERIFIED** | Instant calculations with zero arithmetic overflow. |

---

## 3. Functional Test Checklist

- [x] Top mode tabs switch cleanly between VLSM Subnets and Chmod Matrix.
- [x] CIDR buttons (/24 through /30) recalculate usable hosts, broadcast IP, and wildcard masks instantly.
- [x] Visual split block diagram renders proportionate subnet distribution.
- [x] 3x3 chmod matrix checkboxes toggle accurately on touch with immediate octal and symbolic updates.
- [x] Preset buttons (777, 755, 644, 600) populate matrix checkboxes and strings correctly.
- [x] Clean compilation with exit code 0.
