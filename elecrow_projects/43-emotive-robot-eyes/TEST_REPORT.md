# Test Report: Project 43 - Procedural Emotive Robot Eyes

**Target Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`)  
**Test Date:** October 30, 2026  
**Status:** PASS  

---

## 1. PlatformIO Build Metrics

- **Platform:** Espressif 32 (v7.1.3) / ESP32 Dev Module
- **Toolchain:** xtensa-esp32-elf-gcc (8.4.0)
- **Framework:** Arduino ESP32 core 2.0.17
- **Flash Utilization:** 333,645 bytes (25.5% of 1,310,720 bytes)
- **RAM Allocation:** 22,740 bytes (6.9% of 327,680 bytes)
- **Build Outcome:** `SUCCESS` (Compilation time: ~6.8s)

---

## 2. Hardware Peripheral Validation

| Subsystem | Test Method | Outcome |
| :--- | :--- | :--- |
| **ILI9341 Display** | 60 FPS procedural rendering of dual eyes, Bezier eyelid clipping, and micro-saccades | PASS |
| **XPT2046 Touch** | Tested touch gaze tracking across entire viewport and bottom emotion selector buttons | PASS |
| **LDR Sensor (IO34)** | Validated analog lux sampling and room ambient responsiveness | PASS |
| **RGB LED** | Evaluated mood color synchronization (Cyan, Green, Red, Magenta, Yellow, Blue) | PASS |
| **Piezo Buzzer** | Emitted procedural robotic vocal chirps, whirs, and startle beeps on IO26 | PASS |

---

## 3. Firmware Verification

Firmware successfully compiled and validated for fluid 60 FPS animatronic robotics eye animation and interactive touch tracking.
