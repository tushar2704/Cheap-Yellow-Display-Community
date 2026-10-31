# Project 50: Wet-Finger Multi-Zone Kitchen Hub & Meat Probe

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

The **Wet-Finger Multi-Zone Kitchen Hub & Meat Probe** is an embedded culinary appliance built for the 2.8" Cheap Yellow Display (CYD). Designed specifically with large, forgiving touch target hitboxes suitable for messy kitchen prep work, this device provides 4 independent cooking countdown timers, an internal meat core temperature probe monitor with USDA/culinary doneness profiles, and an interactive recipe protocol checklist.

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
| **Probe Port** | GPIO 27 (Input Pullup) | HY2.0 OneWire thermal sensor / probe bus |
| **Buzzer** | GPIO 26 | Cooking timer buzzer alarms and tap feedback |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Idle (Blue), Active (Green), Alarm (Flashing Red) |

---

## Tabs & Operational Features

1. **4-Zone Cooking Timers Tab**:
   - 2x2 grid of oversized wet-finger timer cards:
     - **Zone 1: Oven** (Default: 25 mins)
     - **Zone 2: Pan Sear** (Default: 8 mins)
     - **Zone 3: Simmer** (Default: 45 mins)
     - **Zone 4: Pasta** (Default: 10 mins)
   - Independent background 1-second countdown tickers.
   - Quick addition buttons: `+1m` and `+5m` for rapid adjustments without numeric keypads.
   - Distinct color-coded card headers and clear running/alarm state indicators.

2. **Meat Probe Tab**:
   - Real-time internal core temperature monitoring in Celsius and Fahrenheit.
   - Pre-programmed doneness profiles:
     - Beef Rare: 52°C (125°F)
     - Beef Medium-Rare: 57°C (135°F)
     - Beef Medium: 63°C (145°F)
     - Pork / Well-Done: 71°C (160°F)
     - Poultry Safe: 74°C (165°F)
   - Visual doneness progress bar from raw to fully cooked.
   - High-temperature acoustic alarm triggering once core temperature reaches target.

3. **Recipe Checklist Tab**:
   - Step-by-step interactive cooking protocol with touch checkboxes.
   - Real-time strike-through and green highlight confirmation when steps are completed.

4. **Hardware SPI Bus Isolation**:
   - Touch bus operates on dedicated VSPI pins (32, 39, 25, 33), eliminating display SPI bus contention.
   - Non-inverted landscape coordinates deliver accurate hit detection across oversized buttons.

---

## Building and Flashing

Build and upload using PlatformIO:

```bash
cd elecrow_projects/50-wetfinger-kitchen-hub
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```

