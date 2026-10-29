# 🧪 Code Test Report — Project 21: Retro CRT Phosphor Bench Oscilloscope

**Date**: 2026-10-30  
**Target Hardware**: Elecrow CrowPanel 2.8" (ESP32-2432S028R)  
**Channel Branding**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Author / Committer**: Tushar Aggarwal (`tushar2704`)  

---

## 1. Compilation Verification

- **Compiler**: `xtensa-esp32-elf-g++` (PlatformIO Espressif 32 v7.1.3)
- **Framework**: Arduino ESP32 core with LEDC hardware PWM generator
- **Build Status**: ✅ **SUCCESS**
- **Build Duration**: 6.61 seconds

### Memory Utilization:
- **RAM / SRAM**: `23,756 bytes` utilized of `327,680 bytes` (**7.2%** usage)
  - *Headroom*: Over 303KB of free SRAM remaining.
- **Flash ROM**: `339,981 bytes` utilized of `1,310,720 bytes` (**25.9%** usage)
  - *Headroom*: Over 970KB available.

---

## 2. Functional & Peripheral Validation

| Subsystem | Target Pin / Bus | Test Performed | Result |
|---|---|---|---|
| **ILI9341 Display** | SPI (IO13, 12, 14, 15, 2) | 8x10 CRT graticule, green phosphor persistence fade, right telemetry panel | ✅ PASSED |
| **XPT2046 Touch** | SPI (IO33, 36, 14, 12, 13) | Timebase step, Volts/div step, RUN/HOLD toggle, and PWM calibration toggle | ✅ PASSED |
| **ADC Sampling** | GPIO 34 (ADC1_CH6) | Continuous analog waveform sampling with rising-edge trigger acquisition | ✅ PASSED |
| **PWM Generator** | GPIO 27 (LEDC Ch0) | 1 kHz square wave reference output at 50% duty cycle | ✅ PASSED |
| **RGB LED** | IO4 (R), IO16 (G), IO17 (B) | Dynamic state coloring: Green (Trigger locked), Yellow (Free run), Blue (Hold/Frozen) | ✅ PASSED |
| **Audio Buzzer** | IO26 | Tactile button click chirps on all parameter adjustments | ✅ PASSED |

---

## 3. Conclusion

Project 21 compiles cleanly with ultra-low memory consumption (7.2% RAM) and provides responsive real-time oscilloscope instrumentation with self-contained calibration signals.
