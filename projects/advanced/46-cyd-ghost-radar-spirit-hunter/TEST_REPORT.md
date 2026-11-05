# 🧪 Test Report — Project 46: CYD Ghost Radar & EMF Tracker

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
- **RAM / SRAM**: `23,056 bytes` utilized of `327,680 bytes` (**7.0%** usage)
  - *Headroom*: Over 304KB free SRAM.
- **Flash ROM**: `344,637 bytes` utilized of `1,310,720 bytes` (**26.3%** usage)
  - *Headroom*: Over 966KB free Flash.

---

## 2. Peripheral & Subsystem Validation

| Subsystem | Target Pin / Bus | Test Performed | Result |
| :--- | :--- | :--- | :---: |
| **ILI9341 Display** | SPI (MOSI:13, MISO:12, SCK:14, CS:15, DC:2) | 360° rotating sonar radar scope, 5-stage EMF LED gauge & EVP terminal | ✅ PASSED |
| **XPT2046 Touch** | SPI (MOSI:32, MISO:39, SCK:25, CS:33, IRQ:36) | Radar scope touch-to-spawn anomaly trigger & manual EVP radio sweep | ✅ PASSED |
| **Optical LDR** | GPIO 34 | Shadow passing detection mapped to simulated freezing temperature drops | ✅ PASSED |
| **Audio Buzzer** | GPIO 26 | Sonar revolution pings, multi-frequency EVP static bursts & EMF 5 alarm | ✅ PASSED |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) [Active LOW] | Calm Green $\rightarrow$ Activity Amber $\rightarrow$ EMF 5 Crimson Siren Strobe | ✅ PASSED |

---

## 3. Functional & Edge Case Validation

1. **Radar Scope & Blip Mathematics**:
   - Continuous vector beam rotation ($0$ to $2\pi$ radians) with persistent blip decay and scope ring preservation.
   - Sweep beam collision detection triggers audible proximity alerts when passing active entities.
2. **Spirit Box EVP Voice Decoder**:
   - Periodically synthesizes radio static and decodes creepy keywords (*"BEHIND YOU"*, *"RUN"*, *"COLD"*).
3. **Dynamic Temperature Response**:
   - Updates HUD every 400ms; casting shadow over CYD immediately drops temperature into freezing negative digits.
