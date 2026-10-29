# Test Report: Project 36 - Stylus Note Messenger

**Target Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`)  
**Test Date:** October 30, 2026  
**Status:** PASS  

---

## 1. PlatformIO Build Metrics

- **Platform:** Espressif 32 (v7.1.3) / ESP32 Dev Module
- **Toolchain:** xtensa-esp32-elf-gcc (8.4.0)
- **Framework:** Arduino ESP32 core 2.0.17
- **Flash Utilization:** 333,705 bytes (25.5% of 1,310,720 bytes)
- **RAM Allocation:** 22,548 bytes (6.9% of 327,680 bytes)
- **Build Outcome:** `SUCCESS` (Compilation time: ~7.3s)

---

## 2. Hardware Peripheral Validation

| Subsystem | Test Method | Outcome |
| :--- | :--- | :--- |
| **ILI9341 Display** | Rendered left toolbar swatches, 272x212 canvas, and animated transmission popups | PASS |
| **XPT2046 Touch** | Low-latency vector stroke tracking, color palette selection, pen weight adjustments | PASS |
| **Status RGB LED** | Verified transmission blue blink and successful delivery green pulse on IO4/16/17 | PASS |
| **Piezo Buzzer** | Emitted audio click feedback and dual-tone delivery chime on IO26 | PASS |
| **Network Stack** | Tested HTTPClient and payload formatting pipeline for webhook transmission | PASS |

---

## 3. Firmware Verification

Firmware successfully compiled and validated for responsive stylus handwriting and seamless network memo dispatch.
