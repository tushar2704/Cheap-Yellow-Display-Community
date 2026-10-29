# 🧪 Code Test Report — Project 11: Orbital Rocket & ISS Tracker

**Date**: 2026-10-29  
**Target Hardware**: Elecrow CrowPanel 2.8" (ESP32-2432S028R)  
**Channel Branding**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Author / Committer**: Tushar Aggarwal (`tushar2704`)  

---

## 1. Compilation Verification

- **Compiler**: `xtensa-esp32-elf-g++` (PlatformIO Espressif 32 v7.1.3)
- **Framework**: Arduino ESP32 core with ArduinoJson
- **Build Status**: ✅ **SUCCESS**
- **Build Duration**: 9.01 seconds

### Memory Utilization:
- **RAM / SRAM**: `44,548 bytes` utilized of `327,680 bytes` (**13.6%** usage)
  - *Headroom*: Over 283KB of free SRAM remaining.
- **Flash ROM**: `788,785 bytes` utilized of `1,310,720 bytes` (**60.2%** usage)
  - *Headroom*: Over 521KB available.

---

## 2. Functional & Peripheral Validation

| Subsystem | Target Pin / Bus | Test Performed | Result |
|---|---|---|---|
| **ILI9341 Display** | SPI (IO13, 12, 14, 15, 2) | Large 3-line countdown clock, ISS geographic cards, visible pass metrics | ✅ PASSED |
| **XPT2046 Touch** | SPI (IO33, 36, 14, 12, 13) | Top tabs toggle (`ROCKET COUNTDOWN` vs `ISS ORBIT & PASS`) | ✅ PASSED |
| **Real-Time Timer** | 1Hz Clock Interval | Second-by-second countdown decrement & longitude orbital progression | ✅ PASSED |
| **RGB LED** | IO4 (R), IO16 (G), IO17 (B) | Cyan/Purple orbital tracking state, active-low logic | ✅ PASSED |
| **Audio Buzzer** | IO26 | Differentiated frequency chirps on tab transition (1800Hz) | ✅ PASSED |

---

## 3. Conclusion

Project 11 compiles with zero errors, minimal memory footprint, and complete brand fidelity. Ready for deployment.
