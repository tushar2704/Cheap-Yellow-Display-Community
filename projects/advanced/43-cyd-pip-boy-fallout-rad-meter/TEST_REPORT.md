# 🧪 Test Report — Project 43: CYD Pip-Boy 3000

**Date**: 2026-11-06  
**Target Hardware**: ESP32-2432S028R (CYD 2.8" Resistive Touch)  
**Channel Branding**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) & [@Makerrdesigns](https://youtube.com/@Makerrdesigns)  
**Author / Committer**: Tushar Aggarwal (`tushar2704`)  

---

## 1. Compilation Verification

- **Compiler**: `xtensa-esp32-elf-g++` (PlatformIO Espressif 32 v7.1.3)
- **Framework**: Arduino ESP32 Core v2.0.17 + ESP-IDF `esp_wifi`
- **Build Status**: ✅ **100% SUCCESS**

### Memory Utilization:
- **RAM / SRAM**: `44,772 bytes` utilized of `327,680 bytes` (**13.7%** usage)
  - *Headroom*: Over 282KB free SRAM.
- **Flash ROM**: `798,565 bytes` utilized of `1,310,720 bytes` (**60.9%** usage)
  - *Headroom*: Over 512KB free Flash.

---

## 2. Peripheral & Subsystem Validation

| Subsystem | Target Pin / Bus | Test Performed | Result |
| :--- | :--- | :--- | :---: |
| **ILI9341 Display** | SPI (MOSI:13, MISO:12, SCK:14, CS:15, DC:2) | Phosphor green CRT aesthetic, dosimeter meter, 13-channel RF histogram | ✅ PASSED |
| **XPT2046 Touch** | SPI (MOSI:32, MISO:39, SCK:25, CS:33, IRQ:36) | Tab switching (`STAT`, `RADAR`, `RADIO`) & radio toggle button | ✅ PASSED |
| **2.4GHz RF Promiscuous Sniffer** | Built-in ESP32 Radio | Captures 802.11 b/g/n packets across channels 1–13 via IRAM callback | ✅ PASSED |
| **Audio Buzzer** | GPIO 26 | High-voltage electrostatic Geiger clicks + Diamond City Radio chiptunes | ✅ PASSED |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) [Active LOW] | Safe Green $\rightarrow$ Rad Warning Amber $\rightarrow$ Biohazard Strobe Red | ✅ PASSED |

---

## 3. Functional & Edge Case Validation

1. **RF to Rads Mapping**:
   - Converts real surrounding 2.4GHz packet traffic into smoothed $\mu\text{Sv/h}$ and RAD values.
   - Geiger click delay scales from ambient background (~900ms) down to 15ms near routers.
2. **Animated Vault Boy Mood Engine**:
   - Transitions between Thumbs-Up Smiling ($<35\ \mu\text{Sv/h}$), Dizzy X-Eyes ($35–100\ \mu\text{Sv/h}$), and Glowing Skull ($>100\ \mu\text{Sv/h}$).
   - Redraws only on mood transition to ensure 100% flicker-free rendering.
3. **Wasteland Chiptune Radio**:
   - Seamless non-blocking note player that mutes Geiger clicks while radio is active.

