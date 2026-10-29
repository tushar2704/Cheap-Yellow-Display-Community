# Project 44: ArdOS Retro Window Manager

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **ArdOS Retro Window Manager** turns the Elecrow 2.8" CYD into a nostalgic 90s-era desktop graphical user interface. Inspired by classic window management architectures, it provides draggable windows with 3D beveled borders, an interactive taskbar with a clickable `[Start]` menu, open window task switcher tabs, and a live system tray clock.

The desktop comes pre-loaded with three fully interactive applications: **Notepad**, **Calculator**, and **Minesweeper**, bringing genuine desktop multitasking to a handheld micro-controller.

---

## Features

- **Draggable Window Management**:
  - Touch-and-drag window titlebars to reposition windows freely across the teal desktop.
  - Active vs Inactive titlebars with 3D beveled borders and dedicated Close `[X]` action buttons.
  - Desktop icon launching for quick multitasking.
- **Classic 90s Taskbar & Start Menu**:
  - `[Start]` button with visual depression and pop-up menu state.
  - Dynamic task tabs in the bottom bar reflecting open applications.
  - Digital clock in the system tray.
- **Three Built-in Desktop Applications**:
  - **Notepad**: Text memo viewer displaying system specifications and hardware info.
  - **Calculator**: Interactive 4-function desktop calculator with numeric LCD display.
  - **Minesweeper**: 6x6 minefield grid with smiley reset button, safe neighbor count reveals, and mine detonation alerts.
- **Acoustic & RGB Feedback**:
  - Satisfying UI keyclicks on `GPIO 26` for button taps and window actions.
  - Green RGB LED indicator on Start menu activation, red flash upon mine explosion.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **OS Status RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW UI status indicator |
| **Acoustic Keyclick Buzzer**| `GPIO 26` | Piezo sound synthesizer for UI clicks |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/44-ardos-window-manager
   pio run --target upload
   ```
3. Use a stylus or finger to click desktop icons, drag window titlebars, solve Minesweeper, and perform calculations.
