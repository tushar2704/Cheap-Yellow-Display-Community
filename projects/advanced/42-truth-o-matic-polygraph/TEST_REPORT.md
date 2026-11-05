# 🧪 Test Report — Project 42: CYD Truth-O-Matic 9000

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
- **RAM / SRAM**: `23,132 bytes` utilized of `327,680 bytes` (**7.1%** usage)
  - *Headroom*: Over 304KB free SRAM.
- **Flash ROM**: `346,989 bytes` utilized of `1,310,720 bytes` (**26.5%** usage)
  - *Headroom*: Over 963KB free Flash.

---

## 2. Peripheral & Subsystem Validation

| Subsystem | Target Pin / Bus | Test Performed | Result |
| :--- | :--- | :--- | :---: |
| **ILI9341 Display** | SPI (MOSI:13, MISO:12, SCK:14, CS:15, DC:2) | 170-pixel scrolling strip chart, dynamic stress meter & verdict stamps | ✅ PASSED |
| **XPT2046 Touch** | SPI (MOSI:32, MISO:39, SCK:25, CS:33, IRQ:36) | Thumbprint pad contact detection + secret corner coordinates | ✅ PASSED |
| **Audio Buzzer** | GPIO 26 | Dual-tone heartbeat (`lub-dub`), calibration chirp, victory fanfare & alarm siren | ✅ PASSED |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) [Active LOW] | Safe Blue $\rightarrow$ Calibrating Amber $\rightarrow$ Verdict Green / Strobe Red | ✅ PASSED |
| **Optical LDR** | GPIO 34 | Ambient finger shadow detection & pressure correlation | ✅ PASSED |

---

## 3. Functional & Edge Case Validation

1. **State Machine Flow**:
   - `STATE_IDLE` $\rightarrow$ `STATE_CALIBRATING` (1.5s) $\rightarrow$ `STATE_SCANNING` (4.5s) $\rightarrow$ `STATE_VERDICT`.
   - Contact withdrawal test: Lifting thumb during scan aborts test with acoustic warning and resets safely.
2. **Secret Prank Overrides**:
   - Debounced corner triggers: Tapping Top-Left forces TRUTH; Top-Right forces LIE; Bottom-Left resets to AUTO.
   - Indicator pixel renders subtly on top bezel without revealing bias to the victim.
3. **Anti-Flicker Optimization**:
   - Status messages are stage-gated (`scanMessageStage`) to prevent repetitive redraws during 60Hz scanning loop.
