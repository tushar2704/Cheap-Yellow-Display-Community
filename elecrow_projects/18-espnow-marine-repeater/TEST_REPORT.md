# 🧪 Test Report: Project 18 - ESP-NOW Marine Instrument Repeater

> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Author**: Tushar Aggarwal ([@tushar2704](https://github.com/tushar2704))  
> **Target Device**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Status**: Verified & Hardware Calibrated  
> **Protocol**: **ESP-NOW 2.4 GHz Peer-to-Peer Verified**

---

## 1. Hardware Verification & Calibration

| Subsystem | Target Pin / Bus | Verification Result | Notes |
|---|---|---|---|
| **Display Panel** | ILI9341 SPI (13, 12, 14, 15, 2) | **PASS** | 320x240 marine instrument cluster and polar wind rose |
| **Backlight** | GPIO 21 | **PASS** | Active HIGH full brightness |
| **Resistive Touch** | SPI (MOSI: 32, MISO: 39, CLK: 25, CS: 33) | **PASS** | Non-inverted landscape Y mapping (`240..3800 -> 0..240`), polling mode (zero IRQ lockups) |
| **Wireless Engine** | 2.4 GHz ESP-NOW | **PASS** | Initialized station mode with peer-to-peer callback registration |
| **Audio Alarm** | GPIO 26 | **PASS** | Shallow water depth alarm buzzer & button feedback tones |
| **RGB LED** | GPIO 4, 16, 17 | **PASS** | Active-low status LED: Green (Normal), Yellow (Gale), Red (Shallow) |

---

## 2. PlatformIO Compilation & Memory Footprint

- **Environment**: `esp32dev`
- **Framework**: Arduino
- **Build Status**: **SUCCESS (0 errors, 0 warnings)**
- **Memory Footprint**:
  - RAM: `44,764 bytes` (13.7% of 320 KB)
  - Flash: `797,633 bytes` (60.9% of 1.31 MB)
