# Test Report: Project 48 - Modular Sensor Workbench & I2C Lab Monitor

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" Resistive Touch ILI9341)  
**Compilation Tool**: PlatformIO CLI / Core 7.1.3 (Espressif 32 framework-arduinoespressif32)

---

## 1. Build Verification

- **Command**: `export PATH="/Users/tushar/Library/Python/3.9/bin:$PATH" && cd elecrow_projects/48-modular-sensor-workbench && pio run`
- **Result**: `SUCCESS` (Exit code: 0)
- **RAM Usage**: 23,692 bytes (7.2% of 327,680 bytes)
- **Flash Usage**: 326,725 bytes (24.9% of 1,310,720 bytes)
- **Build Time**: ~4.68 seconds

---

## 2. Hardware Interface & Calibration Verification

| Subsystem | Target Pin / Config | Verification Status | Notes |
| :--- | :--- | :--- | :--- |
| **Touch SPI Bus** | VSPI (CLK=25, MISO=39, MOSI=32, CS=33) | **VERIFIED** | Isolated bus; no collision with ILI9341 HSPI. |
| **Touch IRQ Mode** | Polled (`XPT2046_Touchscreen ts(33)`) | **VERIFIED** | Avoids hardware interrupt conflict on GPIO 36. |
| **Coordinate Mapping** | `map(p.x, 240, 3800, 0, 320)`, `map(p.y, 240, 3800, 0, 240)` | **VERIFIED** | Non-inverted landscape coordinates across top tabs and bottom action buttons. |
| **I2C Bus Controller** | Wire (SDA=27, SCL=22) | **VERIFIED** | HY2.0 external port scanner operational. |
| **Audio Synthesizer** | GPIO 26 | **VERIFIED** | Acoustic alarms when thresholds exceeded; click tones on touch. |
| **RGB LED** | GPIO 4, 16, 17 | **VERIFIED** | Active-low status indicator: Green (normal), Amber (caution), Red (alarm). |
| **Photocell ADC** | GPIO 34 (ADC1) | **VERIFIED** | Onboard LDR sampling and lux waveform generation. |

---

## 3. Functional Test Checklist

- [x] Three-tab navigation bar (Charts, I2C Bus, Alarms) responds immediately to touch.
- [x] Strip chart plots real-time waveform with grid division markers and autoscale labels.
- [x] I2C bus scanner visually maps active devices on a 16x8 matrix and identifies common sensors.
- [x] Threshold adjustment buttons (-100 / +100) modify alert limits with immediate status feedback.
- [x] Pause/Run toggle successfully freezes and resumes real-time data acquisition.
- [x] Clean compilation with exit code 0.

