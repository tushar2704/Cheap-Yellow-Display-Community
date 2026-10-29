# Project 37: Stylus RPN Calculator

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Stylus RPN Calculator** brings the classic Reverse Polish Notation (RPN) scientific and programmer's calculation workflow to the Elecrow 2.8" CYD. Featuring a traditional four-register stack (`X`, `Y`, `Z`, `T`) and a multi-base programmer's numeric display, it allows engineers and computer scientists to perform high-speed calculations without parentheses.

A standout capability is the interactive 32-bit stylus bit grid: by tapping any individual bit on the display with the stylus pen, developers can directly toggle individual bits and instantly observe the resulting values in Hexadecimal, Decimal, and Binary.

---

## Features

- **4-Level RPN Stack Architecture**:
  - Displays top 4 registers simultaneously:
    - **X**: Current active operand or accumulator result.
    - **Y**: Previous stack level (shown in bright yellow).
    - **Z & T**: Higher stack depth levels.
  - Classic stack manipulation functions: `ENTER` (`ENT`), `SWAP` (`x<>y`), `ROLL DOWN` (`R v`), and `DROP`.
- **Programmer's Multi-Base Bit Inspection**:
  - Simultaneous real-time conversion across **HEX**, **DEC**, and **BIN** representations.
  - Interactive 32-bit graphical indicator grid: tap with stylus to toggle any bit (0..31) on the fly.
- **Bitwise Logic & Arithmetic Engine**:
  - Full suite of bitwise operators: `AND`, `OR`, `XOR`, `NOT`, `SHL` (`<<`), `SHR` (`>>`).
  - Standard arithmetic operators: `+`, `-`, `*`, `/`.
- **Tactile Audio Keyclick**:
  - Emits crisp, satisfying 2.2kHz mechanical audio keyclicks on `GPIO 26` for every touch register press.
- **Visual Calculation LED Pulse**:
  - Cyan and Green RGB LED pulses on `IO16`/`IO17` whenever stack calculations execute.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **Operation RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW stack operation pulse |
| **Keyclick Buzzer** | `GPIO 26` | Piezo acoustic keyclick feedback |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/37-stylus-rpn-calculator
   pio run --target upload
   ```
3. Use a resistive stylus or finger to tap numeric keys, enter values onto the stack, and flip individual bits in the bit grid.
