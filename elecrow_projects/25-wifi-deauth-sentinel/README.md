# Project 25: Wi-Fi Deauth Defense Sentinel

A hardware 802.11 promiscuous intrusion detector and jamming sentinel built for the **Elecrow 2.8" CYD (ESP32-2432S028R)** touch display, developed exclusively for **IamNotIntoTech** (`@IamNotIntoTech`).

## Overview
This compact cybersecurity sentinel monitors 2.4GHz airspace for malicious 802.11 management frame floods (Deauthentication Subtype 12 and Disassociation Subtype 10). Capable of channel hopping across Channels 1–13 or locking onto a specific target network, it meters deauth packet velocities in frames-per-second, logs attacker BSSIDs and victim client MACs, and triggers visual and acoustic alarms.

## Hardware Specifications
- **Display**: 2.8" ILI9341 320x240 SPI TFT with resistive touch (XPT2046)
- **Host Controller**: ESP32-WROOM-32 (Dual Core 240MHz, 520KB SRAM)
- **Touch Pins**: `TOUCH_MOSI=32`, `TOUCH_MISO=39`, `TOUCH_CLK=25`, `TOUCH_CS=33`
- **RGB Status LED**: Active-low multi-color indicator on GPIOs 4 (Red), 16 (Green), 17 (Blue)
- **Buzzer**: Piezo buzzer on GPIO 26 for attack siren alarm and touch confirmation

## Features
- **Promiscuous Packet Inspection**: Native ESP-IDF promiscuous RX callback filtering 802.11 management frames at microsecond speed.
- **Deauth Rate Velocity Meter**: Visual analog bar and real-time counter logging frames/sec velocity alongside peak-hold statistics.
- **Detailed Forensic Event Log**: Captures Attacker BSSID, Victim Device MAC, Operating Channel, and RSSI signal level for forensic inspection.
- **Channel Hopping vs Channel Lock**: Cycle continuously through 2.4GHz channels (300ms dwell time) or tap to lock on a single vulnerable channel.
- **Intrusion Alarm System**:
  - LED Flashing Red upon jamming flood detection (> 10 frames/s).
  - Audible dual-tone siren alarm alerting homelab admins to IoT security camera drops.

## Calibration Standard
- Hardware-verified non-inverted Y coordinate mapping `map(p.y, 240, 3800, 0, 240)`
- Hardware-isolated SPI bus routing preventing ILI9341 bus contention
- Touch polling mode bypassing hardware IRQ lockups
