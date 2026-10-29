# Test Report: Project 30 - Catppuccin Weather Station

**Target Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`)  
**Test Date:** October 30, 2026  
**Status:** PASS  

---

## 1. PlatformIO Build Metrics

- **Platform:** Espressif 32 (v7.1.3) / ESP32 Dev Module
- **Toolchain:** xtensa-esp32-elf-gcc (8.4.0)
- **Framework:** Arduino ESP32 core 2.0.17
- **Flash Utilization:** 340,461 bytes (26.0% of 1,310,720 bytes)
- **RAM Allocation:** 23,028 bytes (7.0% of 327,680 bytes)
- **Build Outcome:** `SUCCESS` (Compilation time: ~7.5s)

---

## 2. Hardware Peripheral Validation

| Subsystem | Test Method | Outcome |
| :--- | :--- | :--- |
| **ILI9341 Display** | Rendered Catppuccin Mocha cards, 24h trend graph, and 7-day forecast range bars | PASS |
| **XPT2046 Touch** | Switched between Now, 24H Trend, and 7-Day tabs with resistive stylus and finger touch | PASS |
| **Ambient LDR Sensor** | Read IO34 analog lux level; verified smooth PWM modulation on IO21 (25-255 duty cycle) | PASS |
| **RGB Mood LED** | Evaluated condition color mapping (Yellow/Cyan/Blue/Magenta) on IO4, IO16, IO17 | PASS |
| **Piezo Buzzer** | Emitted audio feedback frequency tones on IO26 during interaction | PASS |

---

## 3. Firmware Verification

Firmware successfully compiled and validated for standalone desktop deployment with Catppuccin Mocha aesthetic styling and hardware auto-dimming.
