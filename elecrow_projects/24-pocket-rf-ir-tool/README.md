# Project 24: Pocket RF & IR Universal Multi-Tool

A pocket universal infrared remote, signal cloner, and dynamic QR code generator built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This compact handheld diagnostic multi-tool features three versatile modes: a universal 6-key IR appliance remote with 38kHz NEC carrier modulation on GPIO 27, an IR sniffer and cloner with raw timing waveform graphics and 32-bit hex decoder, and an onboard QR code generator for instant phone-scannable Wi-Fi guest network onboarding.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **IR Transmitter**: GPIO 27 (38kHz carrier pulse modulation)
- **IR Receiver**: GPIO 22 (demodulated receiver sensor)
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for transmission chirps

## Features
- **6-Key Touch Remote**: Intuitive touch layout for Power, Mute, Source, Vol+, Vol-, and Ch+ transmitting standard consumer AV codes.
- **IR Protocol Sniffer & Cloner**: Captures incoming 32-bit remote pulses, decodes manufacturer signatures (NEC, Sony, LG), and displays pulse train timing waveforms with replay capability.
- **Dynamic Wi-Fi QR Generator**: Generates high-contrast QR codes directly on the LCD for touch-free camera scanning and instant network credentials.
- **Visual & Acoustic Feedback**: Active-low LED pulses purple during IR transmission while triggering a crisp 3kHz transmitter chirp.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
