# Project 02: Chmod & VLSM Visual Network Architect

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

The **Chmod & VLSM Visual Network Architect** turns the 2.8" Cheap Yellow Display (CYD) into an interactive sysadmin and network engineering pocket computer. Featuring two core offline calculation modules—a Variable Length Subnet Masking (VLSM) /24 visual subnet divider and an interactive 9-box POSIX file permissions (chmod) matrix—this appliance eliminates guesswork in IP planning and access rights.

---

## Key Hardware Pinout

| Peripheral | Hardware Pin | Purpose |
| :--- | :--- | :--- |
| **TFT Driver** | HSPI (Pins 12, 13, 14, 15, 2) | 320x240 ILI9341 Color Display |
| **TFT Backlight** | GPIO 21 | Backlight digital control |
| **Touch MOSI** | GPIO 32 | Dedicated VSPI Data In for XPT2046 |
| **Touch MISO** | GPIO 39 | Dedicated VSPI Data Out |
| **Touch CLK** | GPIO 25 | Dedicated VSPI Bus Clock |
| **Touch CS** | GPIO 33 | Chip Select for Touch Controller |
| **Buzzer** | GPIO 26 | Keypress audio click and mode switch tones |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Active-low status indicator |
| **LDR Sensor** | GPIO 34 (ADC1) | Ambient light sensing |

---

## Operating Modes

1. **VLSM Subnet Architect (`[1] VLSM SUBNETS`)**:
   - Interactive CIDR prefix buttons for `/24` through `/30`.
   - Dynamic real-time calculation of:
     - Subnet Mask (e.g., `255.255.255.192` for `/26`)
     - Wildcard Mask (e.g., `0.0.0.63`)
     - Usable Host Capacity & Total IP count
     - Usable Host Range (`.1` through `.(total - 2)`)
     - Broadcast Address
     - Subnet count within a parent `/24` block
   - Visual color-coded split diagram showing physical distribution of subnets across a `/24` address space.

2. **Chmod Matrix (`[2] CHMOD MATRIX`)**:
   - Interactive 3x3 touch grid across classes (Owner, Group, Public) and permissions (Read = 4, Write = 2, Execute = 1).
   - Real-time calculation of octal command format (`chmod 755`, `chmod 644`, etc.) and symbolic string representation (`-rwxr-xr-x`).
   - One-touch standard presets:
     - `777`: Full open permissions (`rwxrwxrwx`)
     - `755`: Standard executable / directory (`rwxr-xr-x`)
     - `644`: Standard secure file (`rw-r--r--`)
     - `600`: Private key / SSH secret (`rw-------`)

3. **Hardware SPI Bus Isolation & Non-Inverted Y**:
   - Touch bus on dedicated VSPI pins (32, 39, 25, 33) prevents display collisions.
   - Fully calibrated non-inverted landscape mapping for accurate stylus selection.

---

## Building and Flashing

Build and upload using PlatformIO:

```bash
cd elecrow_projects/02-chmod-vlsm-architect
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```

