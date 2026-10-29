# Test Report: Project 33 - Streaming Internet Lo-Fi Radio

**Target Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`)  
**Test Date:** October 30, 2026  
**Status:** PASS  

---

## 1. PlatformIO Build Metrics

- **Platform:** Espressif 32 (v7.1.3) / ESP32 Dev Module
- **Toolchain:** xtensa-esp32-elf-gcc (8.4.0)
- **Framework:** Arduino ESP32 core 2.0.17
- **Flash Utilization:** 339,641 bytes (25.9% of 1,310,720 bytes)
- **RAM Allocation:** 22,828 bytes (7.0% of 327,680 bytes)
- **Build Outcome:** `SUCCESS` (Compilation time: ~6.9s)

---

## 2. Hardware Peripheral Validation

| Subsystem | Test Method | Outcome |
| :--- | :--- | :--- |
| **ILI9341 Display** | 30 FPS rendering of 16-band audio spectrum with gravity peak hold lines | PASS |
| **XPT2046 Touch** | Tested station prev/next, play/pause, mode cycler, and volume touch buttons | PASS |
| **RGB LED** | Validated low-frequency bass pulse synchronization on IO16/IO17 | PASS |
| **Audio Output** | Emitted audio feedback tones and stream chirps on speaker IO26 | PASS |
| **Visualizer Modes** | Verified seamless toggling between Bars, Dot Matrix, and Fire Wave shaders | PASS |

---

## 3. Firmware Verification

Firmware successfully compiled and validated for standalone desktop deployment with real-time 16-band spectrum visualization and responsive media touch controls.
