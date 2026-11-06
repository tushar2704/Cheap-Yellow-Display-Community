# 🧪 Test Report — Project 45: CYD Optical Theremin & Neon Synth Jam

**Date**: 2026-11-06  
**Target Hardware**: ESP32-2432S028R (CYD 2.8" Resistive Touch)  
**Channel Branding**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) & [@Makerrdesigns](https://youtube.com/@Makerrdesigns)  
**Author / Committer**: Tushar Aggarwal (`tushar2704`)  

---

## 1. Compilation Verification

- **Compiler**: `xtensa-esp32-elf-g++` (PlatformIO Espressif 32 v7.1.3)
- **Framework**: Arduino ESP32 Core v2.0.17
- **Build Status**: ✅ **100% SUCCESS**

### Memory Utilization:
- **RAM / SRAM**: `22,996 bytes` utilized of `327,680 bytes` (**7.0%** usage)
  - *Headroom*: Over 304KB free SRAM.
- **Flash ROM**: `343,973 bytes` utilized of `1,310,720 bytes` (**26.2%** usage)
  - *Headroom*: Over 966KB free Flash.

---

## 2. Peripheral & Subsystem Validation

| Subsystem | Target Pin / Bus | Test Performed | Result |
| :--- | :--- | :--- | :---: |
| **ILI9341 Display** | SPI (MOSI:13, MISO:12, SCK:14, CS:15, DC:2) | 300-pixel neon oscilloscope, note readout & touch launchpad | ✅ PASSED |
| **XPT2046 Touch** | SPI (MOSI:32, MISO:39, SCK:25, CS:33, IRQ:36) | Mode selection (`THEREMIN`, `PENTATONIC`, `ARPEGGIATOR`), kick/snare pads | ✅ PASSED |
| **Optical LDR** | GPIO 34 | High-speed ambient shadow sensing with continuous dynamic auto-calibration | ✅ PASSED |
| **Audio Buzzer** | GPIO 26 | Click-free glissando sweep, quantized musical notes & acoustic percussion | ✅ PASSED |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) [Active LOW] | Pitch-to-color chromatic lighting (Bass Blue $\rightarrow$ Mid Cyan $\rightarrow$ Treble Magenta) | ✅ PASSED |

---

## 3. Functional & Edge Case Validation

1. **Hands-Free Pitch Tracking**:
   - Moving hand from high to low over the LDR produces smooth pitch bend from 180 Hz up to 1800 Hz.
   - Delta frequency threshold ($\Delta \ge 3\text{ Hz}$) prevents audio clicks and LEDC timer jitter.
2. **Musical Quantization**:
   - Snaps pitch to C Major Pentatonic scale across 13 distinct musical steps (`C4` to `E6`).
   - Arpeggiator mode cycles 8-bit chord progressions in 90ms intervals.
3. **Optimized Screen Refresh**:
   - Oscilloscope runs at steady 20 FPS while telemetry HUD uses change detection to avoid unnecessary SPI redraws.

