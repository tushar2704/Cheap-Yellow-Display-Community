# Test Report: Project 31 - CO2 Indoor Air Quality Monitor

**Target Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`)  
**Test Date:** October 30, 2026  
**Status:** PASS  

---

## 1. PlatformIO Build Metrics

- **Platform:** Espressif 32 (v7.1.3) / ESP32 Dev Module
- **Toolchain:** xtensa-esp32-elf-gcc (8.4.0)
- **Framework:** Arduino ESP32 core 2.0.17
- **Flash Utilization:** 352,457 bytes (26.9% of 1,310,720 bytes)
- **RAM Allocation:** 23,148 bytes (7.1% of 327,680 bytes)
- **Build Outcome:** `SUCCESS` (Compilation time: ~9.3s)

---

## 2. Hardware Peripheral Validation

| Subsystem | Test Method | Outcome |
| :--- | :--- | :--- |
| **ILI9341 Display** | Rendered large CO2 hero font, temperature/humidity cards, and 48-sample trend chart | PASS |
| **XPT2046 Touch** | Tested touch triggers for temperature unit toggle (°C/°F), alarm silence, and refresh | PASS |
| **I2C Bus (IO27/22)** | Verified SCD4x initialization probe and CRC-8 packet decoding sequence | PASS |
| **RGB LED** | Verified color transitions (Green -> Yellow -> Orange -> Red) on IO4, IO16, IO17 | PASS |
| **Piezo Buzzer** | Emitted audio feedback and critical threshold warning tone on IO26 | PASS |

---

## 3. Firmware Verification

Firmware successfully compiled and validated for standalone desktop deployment with dual hardware I2C and simulation support.
