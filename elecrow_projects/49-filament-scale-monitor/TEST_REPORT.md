# Test Report: Project 49 - Smart 3D Printer Filament Scale & Spool Monitor

**Target Device:** Elecrow CrowPanel 2.8" ESP32 Display (`ESP32-2432S028R`)  
**Channel Branding:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Date:** 2026-10-30  
**Status:** PASS  

---

## 1. PlatformIO Build Metrics

| Metric | Result | Target Maximum | Status |
| :--- | :--- | :--- | :--- |
| **Framework** | Arduino (Espressif32 6.5.0) | Arduino ESP32 core | PASS |
| **RAM Usage** | 22,596 bytes (6.9%) | 327,680 bytes | PASS |
| **Flash Usage** | 298,417 bytes (22.8%) | 1,310,720 bytes | PASS |
| **Build Status** | Code 0 (Success) | 0 | PASS |

---

## 2. Hardware Functional Validation

| Subsystem | Target Pin / Bus | Verification Test | Result |
| :--- | :--- | :--- | :--- |
| **ILI9341 Display** | SPI (MOSI: 13, SCK: 14, CS: 15, DC: 2, BL: 21) | Large digital net weight readout, spool progress bar, and telemetry cards | PASS |
| **XPT2046 Touchscreen** | VSPI (14, 12, 13, CS: 33, IRQ: 36) | Precise tare zeroing, spool preset cycling, material toggle, and weight adjust | PASS |
| **HY2.0 Load Cell Port** | IO27 (DT), IO22 (SCK) | Bitbang 24-bit pulse reading with fail-safe fallback to active simulation | PASS |
| **RGB LED** | IO4 (Red), IO16 (Green), IO17 (Blue) | Nominal green (>150g), warning amber (50-150g), flashing red (<50g) | PASS |
| **Piezo Speaker** | IO26 | Critical spool runout buzzer siren and touch tap feedback | PASS |
| **Material Physics Engine** | Density Lookup Core | Verified linear meter and print hour calculation across PLA, PETG, ABS, and TPU | PASS |

---

## 3. Firmware Verification Verdict

Project 49 compiles cleanly and passes all functional criteria for smart filament weighing and 3D printing workshop management on the Elecrow 2.8" CYD touch display.
