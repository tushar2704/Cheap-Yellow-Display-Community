# Project 35: Solar & Battery Flow Dashboard

**Channel:** [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Hardware:** Elecrow 2.8" CYD (`ESP32-2432S028R`, ILI9341 320x240 SPI TFT, XPT2046 Resistive Touch)

---

## Overview

The **Solar & Battery Flow Dashboard** turns the Elecrow 2.8" CYD into an animated residential solar and energy storage telemetry monitor. Featuring a dynamic power flow topology, it visualizes the live routing of electric power between the photovoltaic rooftop solar array, central hybrid inverter, home battery storage bank, household AC consumption, and the municipal electrical grid.

Animated power packets move smoothly along connecting buslines at speeds proportional to wattage, providing immediate at-a-glance insight into energy independence and grid net metering.

---

## Features

- **Animated Power Flow Topology**:
  - Live animated power packet particles tracking energy flow directions:
    - **Solar PV -> Inverter**: Golden solar generation wattage.
    - **Inverter <-> Battery**: Bidirectional charge/discharge flow with live SoC percentage.
    - **Inverter -> Home Load**: Real-time household electrical consumption.
    - **Inverter <-> Utility Grid**: Net metering export (+) or import (-) flow.
- **Dual Telemetry Views**:
  - **Live Flow View**: Full animated schematic with central inverter routing and 5 interactive telemetry nodes.
  - **24H History Chart**: 12 two-hour interval bar comparison comparing solar generation yield against home electrical consumption.
- **Energy Self-Sufficiency Index**:
  - Computes and displays real-time self-sufficiency percentage along with daily solar yield (kWh) and net grid contribution (kWh).
- **Status Mood RGB LED**:
  - **Green**: 100% clean solar self-sufficiency and active grid export.
  - **Amber / Yellow**: Self-powered operation drawing from stored battery reserves.
  - **Red**: Net energy deficit importing heavy power from the utility grid.
- **Touch Interaction & Acoustic Audio**:
  - One-touch toggle between the Live Flow schematic and 24H History Chart with acoustic confirmation tones on `IO26`.

---

## Hardware Pinout (ESP32-2432S028R)

| Function | Pin / Channel | Description |
| :--- | :--- | :--- |
| **TFT MOSI / MISO / SCK** | `GPIO 13` / `12` / `14` | High-speed hardware SPI display bus |
| **TFT CS / DC / Backlight**| `GPIO 15` / `2` / `21` | Chip select, Data/Command, PWM Backlight |
| **Touch IRQ / CS / SPI** | `GPIO 36` / `33` / VSPI | XPT2046 resistive touch controller |
| **Grid Status RGB LED** | `GPIO 4` (R), `16` (G), `17` (B)| Active LOW solar independence indicator |
| **Buzzer** | `GPIO 26` | Touch interaction audio confirmation |

---

## Configuration & Flashing

1. Connect your Elecrow 2.8" CYD via micro-USB.
2. Build and upload using PlatformIO:
   ```bash
   cd elecrow_projects/35-solar-energy-flow
   pio run --target upload
   ```
3. The dashboard can ingest live MQTT/REST payloads from your home inverter (e.g. Home Assistant Energy dashboard, SolarEdge, Victron, or SunSynk) or run directly in demo mode out of the box.
