# 🧪 Test Report — Project 44: CYD Keep Talking & Nobody Explodes

**Date**: 2026-11-06  
**Target Hardware**: ESP32-2432S028R (CYD 2.8" Resistive Touch)  
**Channel Branding**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) & [@Makerrdesigns](https://youtube.com/@Makerrdesigns)  
**Author / Committer**: Tushar Aggarwal (`tushar2704`)  

---

## 1. Compilation Verification

- **Compiler**: `xtensa-esp32-elf-g++` (PlatformIO Espressif 32 v7.1.3)
- **Framework**: Arduino ESP32 Core v2.0.17 + ESP-NOW
- **Build Status**: ✅ **100% SUCCESS**

### Memory Utilization:
- **RAM / SRAM**: `45,020 bytes` utilized of `327,680 bytes` (**13.7%** usage)
  - *Headroom*: Over 282KB free SRAM.
- **Flash ROM**: `805,657 bytes` utilized of `1,310,720 bytes` (**61.5%** usage)
  - *Headroom*: Over 505KB free Flash.

---

## 2. Peripheral & Subsystem Validation

| Subsystem | Target Pin / Bus | Test Performed | Result |
| :--- | :--- | :--- | :---: |
| **ILI9341 Display** | SPI (MOSI:13, MISO:12, SCK:14, CS:15, DC:2) | 7-segment digital timer, 4 colored wires, Greek keypad & defusal manual | ✅ PASSED |
| **XPT2046 Touch** | SPI (MOSI:32, MISO:39, SCK:25, CS:33, IRQ:36) | Role selector, wire cut hitbox, keypad order taps & manual page tabs | ✅ PASSED |
| **ESP-NOW Wireless** | 2.4GHz Peer-to-Peer | Sub-5ms broadcast state packet sync between Bomb and Manual units | ✅ PASSED |
| **Audio Buzzer** | GPIO 26 | 1-second clock ticks (accelerating <10s), strike buzzes, explosion rumble | ✅ PASSED |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) [Active LOW] | Critical countdown strobe & green defused indicator | ✅ PASSED |

---

## 3. Functional & Edge Case Validation

1. **Procedural Bomb Generation**:
   - Randomizes serial code (`CYD-XXX`), 4 wire colors, and target wire rule logic on each match.
   - 3-strike countdown: wrong wire cut or keypad mis-press registers strikes and triggers game over at strike 3.
2. **Two-Player ESP-NOW Synchronization**:
   - Full bidirectional state telemetry: when bomb detonates or is disarmed, the Manual unit automatically triggers matching game-over graphics and sound effects.
3. **Modular Low-Flicker Redraws**:
   - Touch interactions only update affected wire tiles (`drawSingleWire`) and strike LEDs (`drawStrikes`), avoiding full-screen blanking.

