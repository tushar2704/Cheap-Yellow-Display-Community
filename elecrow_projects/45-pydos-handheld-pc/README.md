# Project 45: PyDOS Handheld Touch PC

**Target Device:** Elecrow CrowPanel 2.8" ESP32 Display (`ESP32-2432S028R`)  
**Channel Branding:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## Overview

**PyDOS Handheld PC** turns the Elecrow 2.8" CYD touch display into a standalone retro computing terminal. Featuring a high-contrast amber/green CRT command-line prompt, an on-screen stylus QWERTY virtual keyboard, and an embedded virtual disk operating system, this appliance allows interactive command execution, file manipulation, hardware diagnostic probes, and batch script testing directly from the resistive touch interface.

---

## Key Hardware Pinout

| Function | ESP32 Pin | Interface / Bus | Description |
| :--- | :--- | :--- | :--- |
| **TFT MOSI** | `IO13` | SPI | ILI9341 Serial Data |
| **TFT MISO** | `IO12` | SPI | ILI9341 Data In |
| **TFT SCLK** | `IO14` | SPI | ILI9341 Clock |
| **TFT CS** | `IO15` | GPIO / SPI | Display Chip Select |
| **TFT DC** | `IO2` | GPIO | Display Command / Data |
| **TFT Backlight** | `IO21` | GPIO | Active HIGH Backlight control |
| **Touch CS** | `IO33` | VSPI | XPT2046 Touch Controller Chip Select |
| **Touch IRQ** | `IO36` | GPIO | XPT2046 Touch Interrupt |
| **RGB LED (Red)** | `IO4` | GPIO | Active LOW Status Indicator |
| **RGB LED (Green)** | `IO16` | GPIO | Active LOW Command Processing |
| **RGB LED (Blue)** | `IO17` | GPIO | Active LOW Disk / Ready Indicator |
| **Piezo Speaker** | `IO26` | DAC / PWM | Keyboard click and prompt bell |
| **LDR Photocell** | `IO34` | ADC1_CH6 | Ambient light sensor |

---

## Features

1. **Virtual DOS Shell Environment**:
   - Supports built-in commands: `DIR`, `VER`, `HELP`, `CLS`, `ECHO`, `DATE`, `TIME`, `SYSINFO`, `COLOR`, `MEM`, `FREE`, and `REBOOT`.
   - File system emulation with simulated virtual disk files (`AUTOEXEC.BAT`, `CONFIG.SYS`, `README.TXT`, `TEST.PY`).
2. **On-Screen Stylus QWERTY Keyboard**:
   - 4-row touch-optimized keyboard layout (`Q-P`, `A-L`, `Z-M`, `SPACE`, `BACKSPACE`, `ENTER`).
   - Visual key-press depression feedback and acoustic piezo keyboard feedback on IO26.
3. **Smooth Terminal Display Engine**:
   - 80x25 character equivalent scroll buffer formatted for 320x130 terminal view.
   - Retro styling with configurable amber (`0xFD20`), green (`0x07E0`), or classic cyan phosphor themes.
4. **Hardware Diagnostic Commands**:
   - Real-time ESP32 heap inspection, chip revision, CPU frequency (240MHz), flash geometry, and LDR ambient lux readings.

---

## Quick Start & Build Instructions

### PlatformIO
1. Open this directory in VS Code or Terminal:
   ```bash
   cd elecrow_projects/45-pydos-handheld-pc
   ```
2. Build firmware:
   ```bash
   pio run
   ```
3. Upload to CrowPanel 2.8" CYD via USB-C:
   ```bash
   pio run --target upload
   ```

---

## Usage Guide

1. **Terminal Interaction**: Tap on-screen keys with the stylus to type commands at the `C:\>` prompt.
2. **Execution**: Tap the green `ENTER` key to execute commands.
3. **Editing**: Tap `BS` (Backspace) to correct mistyped input characters.
4. **Command List**: Type `HELP` followed by `ENTER` to see the complete list of supported DOS shell commands.
