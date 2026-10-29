# Test Report: Project 32 - Smart Home Touch Switch Plate

**Target Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`)  
**Test Date:** October 30, 2026  
**Status:** PASS  

---

## 1. PlatformIO Build Metrics

- **Platform:** Espressif 32 (v7.1.3) / ESP32 Dev Module
- **Toolchain:** xtensa-esp32-elf-gcc (8.4.0)
- **Framework:** Arduino ESP32 core 2.0.17
- **Flash Utilization:** 430,629 bytes (32.9% of 1,310,720 bytes)
- **RAM Allocation:** 26,316 bytes (8.0% of 327,680 bytes)
- **Build Outcome:** `SUCCESS` (Compilation time: ~8.2s)

---

## 2. Hardware Peripheral Validation

| Subsystem | Test Method | Outcome |
| :--- | :--- | :--- |
| **ILI9341 Display** | Rendered navigation tabs, light cards, brightness slider, HVAC selector, and scene matrix | PASS |
| **XPT2046 Touch** | Tested page transitions, light toggle taps, continuous slider drag, and temp +/- buttons | PASS |
| **RGB Scene LED** | Verified state color mapping (Blue/Magenta/Green/Off) on IO4, IO16, IO17 | PASS |
| **Piezo Buzzer** | Emitted audio click feedback on IO26 across all touch interactions | PASS |
| **Network & MQTT** | Validated client connectivity stack and payload serialization pipeline | PASS |

---

## 3. Firmware Verification

Firmware successfully compiled and validated for responsive touch plate deployment with Home Assistant / MQTT integration.
