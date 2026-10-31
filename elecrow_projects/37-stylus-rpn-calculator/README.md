# Project 37: Stylus RPN Calculator

A dedicated hardware reverse-polish-notation scientific calculator and programmer's bitwise workstation engineered for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
Engineered for software developers, electrical engineers, and computer scientists, this pocket appliance provides an authentic 4-level RPN calculation stack (X, Y, Z, T) with zero-parenthesis operational flow. It features simultaneous multi-base display (Hexadecimal, Decimal, and Binary), a 32-bit stylus bit-toggle grid for single-bit flipping directly on the display, and full bitwise/arithmetic operator suites.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Stack Indicator**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue) pulsing during stack push/pop operations
- **Buzzer**: Piezo buzzer on GPIO 26 for tactile keyclick chirps

## Features
- **4-Level Classic RPN Stack Architecture**:
  - Live display of primary registers: X (active accumulator), Y (intermediate), Z, and T (top).
  - Stack management controls: `ENT` (Push), `x<>y` (Swap top registers), `R v` (Roll stack down), `DROP` (Discard bottom register).
- **Programmer's Multi-Base Telemetry**:
  - Simultaneous real-time conversion across Hexadecimal (`0x...`), Unsigned Decimal, and Binary.
- **Interactive 32-Bit Stylus Bit-Toggle Matrix**:
  - Visual 32-bit ladder allowing users to tap individual bits with a stylus to invert bit values immediately.
- **Full Computing Instruction Set**:
  - Arithmetic: Addition (`+`), Subtraction (`-`), Multiplication (`*`), Floating-point Division (`/`).
  - Bitwise Logic: `AND`, `OR`, `NOT`, Bitwise Shift Left (`<<`), Shift Right (`>>`).
  - Radix Inputs: Digits 0 through 9 and Hexadecimal nibbles A through D.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups

