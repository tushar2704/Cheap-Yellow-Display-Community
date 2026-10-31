# Project 23: Multi-Firmware SD Bootloader & OTA Launcher

A standalone PC-free multi-boot firmware selector and OTA flasher built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This bootloader utility allows users to carry dozens of compiled application binaries on a standard MicroSD card and flash them directly to the ESP32 partition table on the fly without plugging into a computer. An interactive touch UI displays `.bin` file sizes, slot indexes, active selection highlights, and writes the selected firmware to flash memory via `Update.h` with an animated progress bar.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **MicroSD Interface (HSPI)**: `SD_CS=5`, `SD_MOSI=23`, `SD_MISO=19`, `SD_SCK=18`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for flash complete fanfares and touch confirmation

## Features
- **MicroSD Card Auto-Scanning**: Traverses the root directory of FAT32 cards for compiled ESP32 `.bin` firmware payloads.
- **Hardware-Decoupled Dual SPI**: Isolates the high-speed SD card read stream (HSPI) from the display and resistive touch controller (VSPI) to eliminate bus collisions.
- **OTA Flash Engine**: Streams binary blocks into the secondary OTA app partition with checksum validation and reboot orchestration.
- **Animated Progress Bar**: Real-time percentage indicator and incremental acoustic sweep pitch during flash write.
- **Fallback Simulation Engine**: Pre-populates built-in firmware candidates when booted without a MicroSD card.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
