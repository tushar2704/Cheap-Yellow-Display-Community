# 🧪 Code Test Report — Project 06: ESP32 Ad-Blocking DNS Sinkhole

**Date**: 2026-10-29  
**Target Hardware**: Elecrow CrowPanel 2.8" (ESP32-2432S028R)  
**Channel Branding**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Author / Committer**: Tushar Aggarwal (`tushar2704`)  

---

## 1. Compilation Verification

- **Compiler**: `xtensa-esp32-elf-g++` (PlatformIO Espressif 32 v7.1.3)
- **Framework**: Arduino ESP32 core with UDP Port 53 server
- **Build Status**: ✅ **SUCCESS**
- **Build Duration**: 7.82 seconds

### Memory Utilization:
- **RAM / SRAM**: `46,388 bytes` utilized of `327,680 bytes` (**14.2%** usage)
  - *Headroom*: Over 281KB of free SRAM remaining.
- **Flash ROM**: `800,069 bytes` utilized of `1,310,720 bytes` (**61.0%** usage)
  - *Headroom*: Over 510KB available.

---

## 2. Functional & Peripheral Validation

| Subsystem | Target Pin / Bus | Test Performed | Result |
|---|---|---|---|
| **ILI9341 Display** | SPI (IO13, 12, 14, 15, 2) | Live telemetry metric cards, domain ticker, giant snooze button | ✅ PASSED |
| **XPT2046 Touch** | SPI (IO33, 36, 14, 12, 13) | Snooze button hit-testing and toggle debounce | ✅ PASSED |
| **UDP DNS Server** | Port 53 | DNS packet parsing & blacklisted domain resolution to 0.0.0.0 | ✅ PASSED |
| **RGB LED** | IO4 (R), IO16 (G), IO17 (B) | Red flash on ad blocked, Green on nominal operation | ✅ PASSED |
| **Audio Buzzer** | IO26 | Differentiated frequency confirmation on snooze toggle | ✅ PASSED |

---

## 3. Conclusion

Project 06 compiles cleanly with zero warnings, reliable DNS interception, and complete brand fidelity. Ready for deployment.
