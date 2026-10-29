# Project 49: Smart 3D Printer Filament Scale & Spool Monitor

**Target Device:** Elecrow CrowPanel 2.8" ESP32 Display (`ESP32-2432S028R`)  
**Channel Branding:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## Overview

**Smart 3D Printer Filament Scale & Spool Monitor** turns the Elecrow 2.8" CYD touch display into an intelligent workshop scale and filament management appliance. Interfacing with an external HX711 24-bit ADC load cell through the onboard HY2.0 port (`IO27:DT`, `IO22:SCK`), the unit automatically deducts manufacturer spool tare weights (Bambu Lab, Prusament, Polymaker, eSUN, Sunlu, Cardboard), calculates remaining filament length in meters using material density curves (PLA, PETG, ABS, TPU), and triggers visual/acoustic runout alarms when a spool has insufficient material remaining.

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
| **HX711 DT** | `IO27` | GPIO (Input) | HY2.0 Load Cell Serial Data line |
| **HX711 SCK** | `IO22` | GPIO (Output) | HY2.0 Load Cell Serial Clock pulse |
| **RGB LED (Red)** | `IO4` | GPIO | Active LOW Spool Runout Alarm Indicator |
| **RGB LED (Green)** | `IO16` | GPIO | Active LOW Nominal Capacity Indicator |
| **RGB LED (Blue)** | `IO17` | GPIO | Active LOW Tare / Calibrate Indicator |
| **Piezo Speaker** | `IO26` | DAC / PWM | Spool runout siren and button feedback |
| **LDR Photocell** | `IO34` | ADC1_CH6 | Ambient light sensor |

---

## Features

1. **Precision Tare Spool Database**:
   - Pre-loaded empty spool tare weights:
     - **Bambu Lab**: 250g
     - **Prusament**: 208g
     - **Polymaker**: 145g
     - **eSUN**: 220g
     - **Sunlu**: 140g
     - **Cardboard Core**: 135g
2. **Volumetric Density & Length Estimation**:
   - Automatically computes true net filament grams:
     $$\text{Net Grams} = \text{Gross Weight} - \text{Spool Tare} - \text{Zero Offset}$$
   - Converts net mass into remaining linear meters for 1.75mm filament based on polymer densities:
     - PLA ($1.24\text{ g/cm}^3 \rightarrow 2.98\text{ g/m}$)
     - PETG ($1.27\text{ g/cm}^3 \rightarrow 3.05\text{ g/m}$)
     - ABS ($1.04\text{ g/cm}^3 \rightarrow 2.50\text{ g/m}$)
     - TPU ($1.21\text{ g/cm}^3 \rightarrow 2.91\text{ g/m}$)
3. **Print Readiness & Time Remaining**:
   - Calculates estimated remaining print time based on standard volumetric deposition rates.
   - 0-100% capacity gauge bar showing physical spool exhaustion.
4. **Hardware & Standalone Simulation**:
   - Fully compatible with HX711 5kg/10kg strain gauge load cells.
   - Built-in simulation mode allows testing and verification even when physical load cells are not attached.
5. **Critical Runout Alerts**:
   - Flashing red LED and repetitive piezo alert tone when net material drops below 50g.

---

## Quick Start & Build Instructions

### PlatformIO
1. Open this directory in VS Code or Terminal:
   ```bash
   cd elecrow_projects/49-filament-scale-monitor
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

1. **Zeroing Scale**: With an empty scale platform, tap **TARE** to zero the scale offset.
2. **Select Brand**: Tap **SPOOL** to cycle through Bambu Lab, Prusa, Polymaker, eSUN, Sunlu, or Cardboard spools. The monitor automatically deducts the exact empty tare weight.
3. **Select Polymer**: Tap **MATERIAL** to cycle PLA, PETG, ABS, or TPU. The linear meter estimation recalculates immediately.
4. **Simulation Mode**: If testing without a load cell, tap **-** or **+** on the bottom right to adjust simulated spool weight and verify alarm thresholds.
