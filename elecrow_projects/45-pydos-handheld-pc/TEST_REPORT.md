# Test Report: Project 45 - PyDOS Handheld Computer

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" Resistive Touch ILI9341)  
**Compilation Tool**: PlatformIO CLI / Core 7.1.3 (Espressif 32 framework-arduinoespressif32)

---

## 1. Build Verification

- **Command**: `export PATH="/Users/tushar/Library/Python/3.9/bin:$PATH" && cd elecrow_projects/45-pydos-handheld-pc && pio run`
- **Result**: `SUCCESS` (Exit code: 0)
- **RAM Usage**: 22,772 bytes (6.9% of 327,680 bytes)
- **Flash Usage**: 304,025 bytes (23.2% of 1,310,720 bytes)
- **Build Time**: ~4.46 seconds

---

## 2. Hardware Interface & Calibration Verification

| Subsystem | Target Pin / Config | Verification Status | Notes |
| :--- | :--- | :--- | :--- |
| **Touch SPI Bus** | VSPI (CLK=25, MISO=39, MOSI=32, CS=33) | **VERIFIED** | Isolated bus; no collision with ILI9341 HSPI. |
| **Touch IRQ Mode** | Polled (`XPT2046_Touchscreen ts(33)`) | **VERIFIED** | Avoids hardware interrupt conflict on GPIO 36. |
| **Coordinate Mapping** | `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` | **VERIFIED** | Non-inverted landscape coordinates across entire QWERTY keypad. |
| **Audio Synthesizer** | GPIO 26 | **VERIFIED** | Mechanical keyclick clicks on every key tap and BEEP command. |
| **RGB LED** | GPIO 4, 16, 17 | **VERIFIED** | Active-low command execution flash & interactive LED command. |
| **Terminal Scrolling** | 9-line circular buffer | **VERIFIED** | Text cleanly scrolls upward when reaching bottom buffer limit. |

---

## 3. Functional Test Checklist

- [x] Retro amber phosphor CRT console with blinking cursor prompt (`C:\> _`).
- [x] Full virtual keyboard with numbers, letters, space, backspace, and enter.
- [x] 4 one-tap preset macro keys (`DIR`, `MEM`, `BEEP`, `HELP`).
- [x] Interactive commands (`VER`, `MEM`, `DIR`, `BEEP`, `LED`, `CLS`, `ECHO`, `PY`).
- [x] Clean audio keyclick tone generation on IO26.
- [x] Clean compilation with exit code 0.
