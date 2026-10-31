# Project 45: PyDOS Handheld Computer

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

**PyDOS Handheld Computer** turns the 2.8" Cheap Yellow Display (CYD) into an interactive, standalone retro command-line terminal environment reminiscent of MS-DOS and MicroPython palmtop computers. Featuring an on-screen stylus QWERTY keyboard with tactile mechanical audio keyclicks, an amber phosphor scrolling CRT display buffer, and built-in interactive shell commands, this project brings tactile command-line hacking to embedded microcontrollers.

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
| **Buzzer** | GPIO 26 | Mechanical keyboard keyclick tones & BEEP command |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Command execution indicator (Active LOW) |

---

## Shell Commands & Keyboard Layout

1. **Integrated Shell Commands**:
   - `HELP`: Lists all supported shell commands.
   - `VER`: Displays OS build info and `@IamNotIntoTech` authorship.
   - `MEM`: Queries real-time free heap and minimum historical heap.
   - `DIR` / `LS`: Lists the virtual root directory and file sizes.
   - `CLS`: Clears the 9-line CRT scrolling buffer.
   - `BEEP`: Emits acoustic test chirp on GPIO 26.
   - `LED <R|G|B|OFF>`: Dynamically sets onboard RGB LED color.
   - `ECHO <msg>`: Echoes input string back to terminal.
   - `PY <expr>`: Evaluates embedded expression mock.

2. **Stylus QWERTY Virtual Keyboard**:
   - **Row 0**: Numeric keys (`1` through `0`, `-`, `=`).
   - **Row 1**: Upper QWERTY row (`Q` through `P`).
   - **Row 2**: Home row (`A` through `L` plus dedicated `CLS` screen clearing key).
   - **Row 3**: Bottom row (`Z` through `M`, `SPC` spacebar, `BK` backspace, and `ENT` enter).
   - **Row 4**: 4 quick shortcut macro keys (`DIR`, `MEM`, `BEEP`, `HELP`) for one-tap execution.

3. **Touch Isolation & Calibration**:
   - Dedicated VSPI hardware configuration prevents SPI bus collision with the display.
   - Polling touch acquisition eliminates interrupt contention on GPIO 36.
   - Calibrated landscape non-inverted Y coordinates ensure pinpoint stylus key hit detection.

---

## Building and Flashing

Build and upload using PlatformIO:

```bash
cd elecrow_projects/45-pydos-handheld-pc
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```

