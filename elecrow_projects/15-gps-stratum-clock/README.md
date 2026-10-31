# Project 15: GPS Stratum Triple Clock & Satellite Monitor

A standalone atomic precision stratum clock and GNSS constellation monitor built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This project transforms the ESP32 Cheap Yellow Display into a lab-grade Stratum 1 atomic timekeeper and geodesic dashboard. Receiving standard NMEA sentences over the HY2.0 UART interface, it delivers synchronized UTC/Zulu, local time with custom offsets, GPS telemetry, and live signal-to-noise ratio (SNR) bar graphs for up to 8 orbital satellites.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **UART Port**: RX=27, TX=22 (HY2.0 connector / pin headers)
- **Status Indication**: Active-Low RGB LED (Pin 4: Red, Pin 16: Green, Pin 17: Blue)
- **Audio Feedback**: Piezo buzzer on GPIO 26 for PPS micro-ticks and touch chirps

## Features
- **Triple Time Display**: Simultaneous display of Atomic UTC/Zulu, configured Local Time, and satellite telemetry.
- **Constellation Monitor**: Multi-channel bar graph rendering individual satellite PRNs, signal-to-noise ratio (dB-Hz), and active fix health.
- **Geodesic Telemetry**: Real-time readout of Latitude, Longitude, Altitude MSL, and HDOP precision metrics.
- **Accurate Touch Navigation**: Seamlessly toggle between Clocks view, Satellite SNR view, and quick Timezone step offsets (+30 min steps) with anti-bounce filtering.
- **Hardware Status LED**: Automatic 3D fix status reflection (Green = 3D Lock, Amber = 2D fix, Red = Searching).

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
- Acoustic and visual button feedback
