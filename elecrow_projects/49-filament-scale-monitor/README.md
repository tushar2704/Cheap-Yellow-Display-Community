# Project 49: Smart 3D Printer Filament Scale & Spool Monitor

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

The **Smart 3D Printer Filament Scale & Spool Monitor** transforms the 2.8" Cheap Yellow Display (CYD) into a dedicated 3D printing workshop telemetry station. Interfacing directly with an HX711 24-bit ADC load cell amplifier via the onboard HY2.0 connector (IO27/IO22), this device computes net remaining filament weight, remaining linear meters, remaining print hours, and triggers acoustic and visual runout alarms before prints fail mid-job.

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
| **Load Cell Port** | GPIO 27 (DT), GPIO 22 (SCK) | HX711 24-bit ADC communication |
| **Buzzer** | GPIO 26 | Acoustic runout warning & UI button clicks |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Safe (Green), Caution (Amber), Alarm (Red) |

---

## Features & Databases

1. **Integrated Spool Brand Database**:
   - One-touch cycling through tare weights of popular empty spools:
     - **Bambu Lab**: 250g
     - **Prusament**: 208g
     - **Polymaker**: 145g
     - **eSUN Spool**: 220g
     - **Sunlu Plastic**: 140g
     - **Cardboard Core**: 135g

2. **Material Density Calculator**:
   - Computes exact linear filament meters based on 1.75mm cross-section volume and material density:
     - **PLA**: 2.98 g/m
     - **PETG**: 3.05 g/m
     - **ABS**: 2.50 g/m
     - **TPU**: 2.91 g/m

3. **Remaining Print Time & Runout Alerts**:
   - Dynamic remaining print hour calculation assuming a standard 12g/hour average deposition rate.
   - Dual-threshold safety monitor:
     - **Safe (>150g)**: Solid green RGB indicator.
     - **Caution (50g–150g)**: Amber warning indicator.
     - **Critical Runout (<50g)**: Flashing red RGB indicator and pulsing acoustic alarm.

4. **Hardware SPI Bus Isolation**:
   - Touch controller operates on isolated VSPI pins (32, 39, 25, 33), avoiding display bus conflicts.
   - Calibrated landscape coordinates deliver accurate stylus response across all buttons and canvas regions.

---

## Building and Flashing

Build and upload using PlatformIO:

```bash
cd elecrow_projects/49-filament-scale-monitor
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```

