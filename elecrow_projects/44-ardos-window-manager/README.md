# Project 44: ArdOS Retro Window Manager

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

**ArdOS** is a retro 90s-style desktop graphical operating system and window manager built for the Cheap Yellow Display (CYD). Featuring classic 3D beveled borders, draggable and stackable windows, an authentic bottom taskbar with an interactive Start menu, and built-in micro-applications (Calculator, Minesweeper, and Notepad), this firmware demonstrates full multitasking GUI architecture on an embedded microcontroller.

---

## Key Hardware Pinout

| Peripheral | Hardware Pin | Purpose |
| :--- | :--- | :--- |
| **TFT Driver** | HSPI (Pins 12, 13, 14, 15, 2) | 320x240 ILI9341 Color Display |
| **TFT Backlight** | GPIO 21 | Backlight control |
| **Touch MOSI** | GPIO 32 | Dedicated VSPI Data In for XPT2046 |
| **Touch MISO** | GPIO 39 | Dedicated VSPI Data Out |
| **Touch CLK** | GPIO 25 | Dedicated VSPI Bus Clock |
| **Touch CS** | GPIO 33 | Chip Select for Touch Controller |
| **Buzzer** | GPIO 26 | Classic UI keyclicks, chimes, and mine detonations |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Start menu and game state feedback (Active LOW) |

---

## Operating System Features

1. **Window Manager Engine**:
   - Draggable windows with classic navy blue active titlebars and slate inactive titlebars.
   - Titlebar close buttons `[X]` to dismiss apps.
   - 3D raised and sunken bevel drawing routines for vintage window chrome.

2. **Interactive Taskbar & Start Menu**:
   - Bottom taskbar (Y: 216–239) with live app task buttons for quick foreground switching.
   - Working **Start** button that toggles an authentic cascading fly-out menu with app shortcuts.
   - System tray clock and status indicators.

3. **Desktop Applications**:
   - **Calculator**: 16-key touch calculator supporting addition, subtraction, multiplication, and division with floating point precision.
   - **Minesweeper**: 6x6 minefield grid with randomized bomb planting, proximity counters, flagged states, smiley face reset, and acoustic cues.
   - **Notepad**: System information memo viewer with hardware specs and channel branding.

4. **Hardware-Isolated Touch Driver**:
   - Dedicated VSPI bus configuration prevents SPI bus collision with display transactions.
   - Fully calibrated non-inverted landscape mapping for accurate stylus pointing.

---

## Building and Flashing

Build and upload using PlatformIO:

```bash
cd elecrow_projects/44-ardos-window-manager
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```
