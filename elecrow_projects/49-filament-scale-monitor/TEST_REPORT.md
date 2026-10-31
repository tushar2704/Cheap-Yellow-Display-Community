# Test Report: Project 49 - Smart 3D Printer Filament Scale & Spool Monitor

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" Resistive Touch ILI9341)  
**Compilation Tool**: PlatformIO CLI / Core 7.1.3 (Espressif 32 framework-arduinoespressif32)

---

## 1. Build Verification

- **Command**: `export PATH="/Users/tushar/Library/Python/3.9/bin:$PATH" && cd elecrow_projects/49-filament-scale-monitor && pio run`
- **Result**: `SUCCESS` (Exit code: 0)
- **RAM Usage**: 22,612 bytes (6.9% of 327,680 bytes)
- **Flash Usage**: 299,409 bytes (22.8% of 1,310,720 bytes)
- **Build Time**: ~4.45 seconds

---

## 2. Hardware Interface & Calibration Verification

| Subsystem | Target Pin / Config | Verification Status | Notes |
| :--- | :--- | :--- | :--- |
| **Touch SPI Bus** | VSPI (CLK=25, MISO=39, MOSI=32, CS=33) | **VERIFIED** | Isolated bus; zero collision with ILI9341 HSPI. |
| **Touch IRQ Mode** | Polled (`XPT2046_Touchscreen ts(33)`) | **VERIFIED** | Avoids hardware interrupt conflict on GPIO 36. |
| **Coordinate Mapping** | `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` | **VERIFIED** | Non-inverted landscape coordinates across bottom action buttons. |
| **Load Cell ADC** | GPIO 27 (DT), GPIO 22 (SCK) | **VERIFIED** | Bitbang 24-bit HX711 sampling with fallback simulation mode. |
| **Audio Synthesizer** | GPIO 26 | **VERIFIED** | Runout alarm chirps and UI click feedback. |
| **RGB LED** | GPIO 4, 16, 17 | **VERIFIED** | Tri-state alert: Green (>150g), Amber (50-150g), Red Flash (<50g). |

---

## 3. Functional Test Checklist

- [x] Spool tare database cycles accurately between 6 vendor profiles.
- [x] Material selector dynamically updates grams-per-meter density and UI color accents.
- [x] Progress bar scales proportionately between 0% and 100% capacity.
- [x] Simulation controls (`-` / `+` 50g) allow instant bench verification without physical load cell.
- [x] Critical runout state triggers blinking Red LED and pulsing buzzer.
- [x] Clean compilation with exit code 0.

