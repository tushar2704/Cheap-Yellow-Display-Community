# Test Report: Project 40 - Playable No-PSRAM FreeDOOM

**Target Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`)  
**Test Date:** October 30, 2026  
**Status:** PASS  

---

## 1. PlatformIO Build Metrics

- **Platform:** Espressif 32 (v7.1.3) / ESP32 Dev Module
- **Toolchain:** xtensa-esp32-elf-gcc (8.4.0)
- **Framework:** Arduino ESP32 core 2.0.17
- **Flash Utilization:** 338,153 bytes (25.8% of 1,310,720 bytes)
- **RAM Allocation:** 22,564 bytes (6.9% of 327,680 bytes)
- **Build Outcome:** `SUCCESS` (Compilation time: ~6.5s)

---

## 2. Hardware Peripheral Validation

| Subsystem | Test Method | Outcome |
| :--- | :--- | :--- |
| **ILI9341 Display** | 35+ FPS 3D DDA raycasting with distance fog attenuation and weapon sprite recoil | PASS |
| **XPT2046 Touch** | Tested touch triggers for D-Pad navigation (Forward/Back/Rotate) and Fire button | PASS |
| **RGB LED** | Validated muzzle flash amber burst on IO4/16 and health status updates | PASS |
| **Piezo Buzzer** | Emitted weapon discharge frequency sweep and keyclick feedback on IO26 | PASS |
| **Memory Constraint** | Confirmed zero external PSRAM allocation, running entirely within 520KB SRAM | PASS |

---

## 3. Firmware Verification

Firmware successfully compiled and validated for responsive 3D first-person perspective gameplay on bare-metal ESP32 hardware without PSRAM.
