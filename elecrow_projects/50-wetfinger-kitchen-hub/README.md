# Project 50: Wet-Finger Multi-Zone Kitchen Hub & Meat Probe

**Target Device:** Elecrow CrowPanel 2.8" ESP32 Display (`ESP32-2432S028R`)  
**Channel Branding:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## Overview

**Wet-Finger Multi-Zone Kitchen Hub & Meat Probe** is a dedicated countertop culinary assistant built for the Elecrow 2.8" CYD touch display. Designed specifically for kitchen environments with large, wet-finger-friendly touch targets, it features 4 independent simultaneous cooking countdown zones (Oven, Pan Sear, Simmer, Pasta), a precision core meat probe monitor with culinary doneness presets (Rare to Poultry Safe), an interactive recipe step checklist, and multi-tier acoustic and visual alarms.

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
| **HY2.0 Probe Pin** | `IO27` | GPIO (1-Wire / ADC) | Meat temperature probe interface |
| **RGB LED (Red)** | `IO4` | GPIO | Active LOW Timer Expired / Overheat Siren |
| **RGB LED (Green)** | `IO16` | GPIO | Active LOW Cooking Active Indicator |
| **RGB LED (Blue)** | `IO17` | GPIO | Active LOW Idle Ready Indicator |
| **Piezo Speaker** | `IO26` | DAC / PWM | Loud acoustic kitchen alarm beeper |
| **LDR Photocell** | `IO34` | ADC1_CH6 | Ambient kitchen lighting sensor |

---

## Features

1. **4 Independent Cooking Timer Zones**:
   - **Zone 1: Oven / Roast** (Default 25m)
   - **Zone 2: Pan Sear** (Default 8m)
   - **Zone 3: Simmer** (Default 45m)
   - **Zone 4: Pasta / Boil** (Default 10m)
   - Independent start, pause, +1 min, +5 min, and reset touch controls per zone.
   - Distinct color themes and countdown formatting (`MM:SS`).
2. **Meat Probe Temperature Monitor**:
   - Live dual-scale temperature readout in both Celsius and Fahrenheit.
   - Built-in USDA/Chef culinary doneness presets:
     - **Beef Rare**: $52^\circ\text{C}$ ($125^\circ\text{F}$)
     - **Beef Med-Rare**: $57^\circ\text{C}$ ($135^\circ\text{F}$)
     - **Beef Medium**: $63^\circ\text{C}$ ($145^\circ\text{F}$)
     - **Pork / Well-Done**: $71^\circ\text{C}$ ($160^\circ\text{F}$)
     - **Poultry Safe**: $74^\circ\text{C}$ ($165^\circ\text{F}$)
   - Dynamic doneness progress bar with automatic thermal alert when core temperature is achieved.
3. **Interactive Recipe Checklist**:
   - Step-by-step culinary protocol with tap-to-toggle strike-through and checkmarks.
4. **Wet-Finger UI Design**:
   - Large touch button tap targets engineered for knuckles, wet thumbs, or styluses.
   - High-contrast color palettes for glanceable visibility across a bright kitchen.
5. **Multi-Sensory Alarm Architecture**:
   - Repetitive pulsating piezo siren tone on IO26.
   - Flashing red alert on RGB LED when any zone expires or internal meat target is reached.

---

## Quick Start & Build Instructions

### PlatformIO
1. Open this directory in VS Code or Terminal:
   ```bash
   cd elecrow_projects/50-wetfinger-kitchen-hub
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

1. **Timers View**:
   - Tap any of the 4 big zone cards to select it.
   - Tap **START / PAUSE** to run or pause the active timer.
   - Tap **+1m** or **+5m** to quickly append cooking time.
   - When a timer reaches `00:00`, the hub flashes red and sounds an alarm. Tap the card or **RESET** to silence.
2. **Meat Probe View**:
   - Tap **CHANGE DONENESS** to select your target doneness temperature profile.
   - Monitor the internal temperature climb on the large digital display and progress bar.
   - Tap **-2C** / **+2C** to simulate thermal changes when testing without a physical probe.
3. **Recipe View**:
   - Tap individual recipe steps to mark them complete as you prepare meals.
