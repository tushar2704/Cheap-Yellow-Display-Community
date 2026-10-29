# Test Report: Project 37 - Stylus RPN Calculator

**Target Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`)  
**Test Date:** October 30, 2026  
**Status:** PASS  

---

## 1. PlatformIO Build Metrics

- **Platform:** Espressif 32 (v7.1.3) / ESP32 Dev Module
- **Toolchain:** xtensa-esp32-elf-gcc (8.4.0)
- **Framework:** Arduino ESP32 core 2.0.17
- **Flash Utilization:** 339,625 bytes (25.9% of 1,310,720 bytes)
- **RAM Allocation:** 22,700 bytes (6.9% of 327,680 bytes)
- **Build Outcome:** `SUCCESS` (Compilation time: ~6.2s)

---

## 2. Hardware Peripheral Validation

| Subsystem | Test Method | Outcome |
| :--- | :--- | :--- |
| **ILI9341 Display** | Rendered 4-level RPN stack display, 32-bit register grid, and 30-key numeric keypad | PASS |
| **XPT2046 Touch** | Tested touch triggers for all 30 keys and single-bit flips on the 32-bit stylus grid | PASS |
| **RPN Stack Engine** | Validated push, pop, swap (x<>y), roll down, and drop stack manipulations | PASS |
| **Bitwise Arithmetic** | Verified AND, OR, XOR, NOT, SHL, SHR, +, -, *, / math operations | PASS |
| **Piezo Buzzer** | Emitted audio click feedback on IO26 across all keypresses and bit toggles | PASS |

---

## 3. Firmware Verification

Firmware successfully compiled and validated for responsive standalone RPN calculations and interactive bitwise inspection.
