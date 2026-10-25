# 🟡 Cheap Yellow Display Community — The Ultimate CYD Hub

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![License: CC BY-NC-ND 4.0](https://img.shields.io/badge/Assets-CC_BY--NC--ND_4.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-nd/4.0/)
[![ESP32](https://img.shields.io/badge/ESP32-Supported-blue.svg)](https://www.espressif.com/)
[![ESP32-S3](https://img.shields.io/badge/ESP32--S3-Supported-blue.svg)](https://www.espressif.com/)
[![Arduino](https://img.shields.io/badge/Arduino-Compatible-teal.svg)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Ready-orange.svg)](https://platformio.org/)
[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Live%20In--Browser-brightgreen.svg)](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

> **The ultimate open-source community hub for ESP32 Cheap Yellow Display (CYD) — covering every size, every variant, 3D enclosures, hardware mods, and projects you can flash in 1-click directly from your browser.**

---

## ⚡ 1-Click In-Browser Web Flasher

**No Arduino IDE or PlatformIO installation required!** You can flash community firmwares onto your CYD right now using Google Chrome, Edge, or Opera:

👉 **[tushar2704.github.io/Cheap-Yellow-Display-Community/](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)**


---

## What is a Cheap Yellow Display (CYD)?

A **Cheap Yellow Display** (or **CYD** for short) is a family of low-cost ESP32-based development boards with **built-in TFT LCD touchscreens**. These boards are commonly sold under model numbers like **ESP32-2432S028R**, **ESP32-3248S035C**, **ESP32-4048S040**, and more. They typically cost between **$10–$25** and come with everything you need to start building touchscreen projects — no soldering required.

The name "Cheap Yellow Display" comes from the distinctive **yellow PCB** that these boards share, making them instantly recognizable in the maker community.

### Why are CYDs so popular?

- 💰 **Incredibly affordable** — $10–$25 for a complete touchscreen dev board
- 🔌 **All-in-one** — ESP32 + display + touch screen + WiFi + Bluetooth on a single board
- 🛠️ **No soldering** — Everything is pre-assembled and ready to program
- 📱 **Multiple sizes** — From 2.4" to 7.0" screens to fit any project
- 🌐 **WiFi & Bluetooth** — Built-in wireless for IoT projects
- 💾 **SD Card slot** — On most variants for data logging and media
- 🎨 **Full color TFT** — 65K colors, great for GUIs and dashboards

---

## 🌟 The CYD Community Advantage — Why Choose This Hub?

Unlike fragmented tutorials and outdated code snippets scattered across the internet, **Cheap Yellow Display Community** is engineered from the ground up as a modern, all-in-one ecosystem:

- ⚡ **1-Click Browser Web Flasher**: Flash ready-to-run firmwares directly from Google Chrome, Edge, or Opera via Web Serial. Zero IDEs, compilers, or driver headaches.
- 📐 **Universal Hardware Scope**: First-class support for **all 7+ CYD display sizes** (2.4", 2.8", 3.2", 3.5", 4.0", 5.0", 7.0"), covering both **Resistive** and **Capacitive** touch controllers.
- 🔌 **Universal Pin Abstraction (`cyd_pins.h`)**: A single unified header that automatically configures display, touch, MicroSD, audio, and RGB LED pins with one line of code.
- 📱 **Smartphone-Grade Embedded UI**: Pre-configured setups for **LVGL v8/v9** and **DMA LovyanGFX** alongside standard TFT_eSPI for ultra-smooth 60 FPS interfaces.
- 🖨️ **Tailor-Made 3D Printable Enclosures**: Custom snap-fit desktop stands, smart home wall bezels, and benchtop cases engineered by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).
- 🎬 **Dedicated Video Masterclasses**: Comprehensive code walkthroughs, hardware teardowns, and CAD tutorials on [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) and [@Makerrdesigns](https://youtube.com/@Makerrdesigns).
- 🛡️ **Dual AGPLv3 + Creative Commons Licensing**: Strong open-source copyleft protection for code combined with commercial protection for 3D enclosures and assets.
- 🤖 **Continuous Integration (CI/CD)**: Every firmware project is automatically compiled with PlatformIO on every commit to ensure 100% bug-free builds.

---

## 📺 YouTube Channels

This project is created and maintained by two YouTube channels — follow both for code tutorials, build logs, and custom 3D enclosures:

| Channel | Focus | Link |
|---|---|---|
| 🎬 **IamNotIntoTech** | Tech projects with humor — CYD tutorials, ESP32 builds, code breakdowns | [▶️ youtube.com/@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) |
| 🖨️ **Makerrdesigns** | 3D design & printing — CYD enclosures, Fusion 360 tutorials, maker builds | [▶️ youtube.com/@Makerrdesigns](https://youtube.com/@Makerrdesigns) |

---

## 📐 Supported CYD Variants

This repository supports **ALL** Cheap Yellow Display variants currently available:

| Model | Screen Size | Resolution | Touch Type | ESP32 Variant | Display Driver |
|---|---|---|---|---|---|
| ESP32-2432S024 | 2.4" | 320×240 | Resistive | ESP32-WROOM-32 | ILI9341 |
| **ESP32-2432S028R** | **2.8"** | **320×240** | **Resistive** | **ESP32-WROOM-32** | **ILI9341** |
| ESP32-2432S028C | 2.8" | 320×240 | Capacitive | ESP32-WROOM-32 | ST7789 |
| ESP32-3248S032 | 3.2" | 320×240 | Resistive | ESP32-WROOM-32 | ILI9341 |
| ESP32-3248S035R | 3.5" | 480×320 | Resistive | ESP32-WROOM-32 | ST7796 |
| ESP32-3248S035C | 3.5" | 480×320 | Capacitive | ESP32-WROOM-32 | ST7796 |
| ESP32-4048S040 | 4.0" | 480×480 | Capacitive | ESP32-S3 | ST7701 |
| ESP32-8048S050 | 5.0" | 800×480 | Capacitive | ESP32-S3 | RGB 16-bit |
| ESP32-8048S070 | 7.0" | 800×480 | Capacitive | ESP32-S3 | RGB 16-bit |

> 💡 **Not sure which CYD you have?** Check our [Variant Identification Guide →](docs/VARIANTS.md)

---

## 📚 Essential Documentation

- ⚡ **[Setup Guide](docs/SETUP.md)** — Step-by-step Arduino IDE & PlatformIO configuration
- 🌟 **[Project & Enclosure Showcase](docs/SHOWCASE.md)** — Visual catalog of all community builds
- 🔌 **[Pinout Reference](docs/PINOUT.md)** — Pin maps for all display sizes
- ⚙️ **[Universal Pin Header](hardware/cyd_pins.h)** — One header file to configure any CYD board
- 👆 **[Capacitive Touch Pack](hardware/capacitive/)** — I2C drivers for CST816T, FT6236, and GT911 CYD variants
- 🎨 **[LVGL v8 Setup Guide](hardware/lvgl/)** — High-performance graphics configuration & zero-boilerplate drivers
- 🏡 **[ESPHome YAML Configurations](hardware/esphome/)** — Native Home Assistant smart touch panel integration
- 🐍 **[MicroPython Setup & Code](hardware/micropython/)** — Pure Python scripts and hardware drivers
- 🖨️ **[3D Printing Master Guide](enclosures/PRINTING_GUIDE.md)** — Slicer profiles, filament choices, and snap tolerances
- 🛠️ **[Hardware Add-ons](docs/ADDONS.md)** — Connecting sensors, rotary encoders, speakers, and I2C devices
- 🔬 **[Hardware Mods & Fixes](docs/MODS.md)** — USB-C PD fix, LDR auto-brightness, audio whine filter, battery power
- 🔧 **[Troubleshooting](docs/TROUBLESHOOTING.md)** — White screen fixes, upload errors, touch calibration
- ❓ **[FAQ](docs/FAQ.md)** — Frequently asked questions & buying guide

---

## 🛠️ Community Projects

### 🟢 Beginner Projects (All for 2.8" CYD — Zero external wiring!)
| # | Project | Description | Difficulty | Guide |
|---|---|---|:---:|:---:|
| 1 | **Hello CYD** | Complete test suite: display, touch crosshair, RGB LED, LDR gauge & SD card | ⭐ (1/5) | [Open](projects/beginner/01-hello-cyd/) |
| 2 | **WiFi Scanner** | 2.4 GHz spectrum analyzer with RSSI signal bars & touch re-scan button | ⭐ (1/5) | [Open](projects/beginner/02-wifi-scanner/) |
| 3 | **CYD Touch Paint** | Stylus drawing canvas with 8-color palette, eraser & RGB LED sync | ⭐ (1/5) | [Open](projects/beginner/03-touch-paint/) |
| 4 | **RGB Touch Sliders** | Interactive RGB color mixer with PWM hardware control of onboard LED | ⭐ (1/5) | [Open](projects/beginner/04-rgb-sliders/) |
| 5 | **MicroSD Slideshow** | Digital photo frame with auto-advance and touch swipe navigation | ⭐⭐ (2/5) | [Open](projects/beginner/05-sd-slideshow/) |
| 6 | **Touch Button Matrix** | Latching toggle buttons, audio chirps, and tap counter card | ⭐ (1/5) | [Open](projects/beginner/06-touch-buttons/) |
| 7 | **Retro Matrix Rain** | Cyberpunk falling glyph streams with LDR auto-dimming & touch ripples | ⭐ (1/5) | [Open](projects/beginner/07-retro-matrix/) |

### 🟡 Intermediate Projects (All for 2.8" CYD — Connected Dashboards)
| # | Project | Description | Difficulty | Guide |
|---|---|---|:---:|:---:|
| 1 | **Weather Station** | Real-time weather dashboard & NTP clock via free Open-Meteo API | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/01-weather-station/) |
| 2 | **Crypto Price Tracker** | Live Bitcoin, Ethereum & Solana ticker with 24h change and sparkline | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/02-crypto-tracker/) |
| 3 | **Rolling Digit Clock** | Retro split-flap rolling clock synchronized with atomic NTP time | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/03-rolling-clock/) |
| 4 | **WiFi Internet Radio** | Live Shoutcast/Icecast radio streaming to onboard speaker via DAC | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/04-internet-radio/) |
| 5 | **LVGL v8 Showcase** | Smartphone-smooth touch animations, tabs, sliders, and arc gauges | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/05-lvgl-widgets-demo/) |
| 6 | **Pomodoro Desk Timer** | 25m Focus / 5m Break productivity timer with progress arc & audio alarms | ⭐⭐ (2/5) | [Open](projects/intermediate/06-pomodoro-timer/) |

### 🔴 Advanced Projects (All for 2.8" CYD — Flagship Firmware)
| # | Project | Description | Difficulty | Guide |
|---|---|---|:---:|:---:|
| 1 | **CYD OS** | Desktop launcher environment with status bar, 6 built-in apps & window manager | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/01-cyd-os/) |
| 2 | **Home Assistant Touch** | Smart Home Wall Control Panel with device toggles & climate thermostat | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/02-home-assistant-touch/) |
| 3 | **Apple HomeKit Terminal** | Cupertino dark UI terminal with multi-accessory controls, climate card & RGB sync | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/03-homekit-terminal/) |
| 4 | **Zigbee2MQTT Commander** | Direct MQTT touchscreen panel with live device control, power draw & mesh telemetry | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/04-zigbee-mqtt-commander/) |
| 5 | **BLE Proximity Radar** | Military-style circular BLE scanning radar scope with target blips & iBeacon tracker | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/05-ble-ibeacon-radar/) |
| 6 | **ESP-NOW Mesh Intercom** | Routerless P2P mesh intercom with touch dispatch (Doorbell, SOS, Ping) & live feed | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/06-esp-now-mesh-node/) |
| 7 | **3D Printer Terminal** | OctoPrint & Klipper monitor with hotend/bed thermals, progress % & touch controls | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/07-octoprint-klipper-terminal/) |
| 8 | **NES Retro Arcade** | 8-Bit space invader arcade engine with on-screen D-pad, chiptune audio & score HUD | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/08-nes-game-emulator/) |
| 9 | **Digital Oscilloscope** | 100kHz DSO with 8x6 graticule grid, Vpp/Vmax metrics, timebases & trace freeze | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/09-digital-oscilloscope-analyzer/) |
| 10 | **Spotify Car Thing Remote** | Dedicated music controller: vinyl art, live scrubber, transport controls & volume | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/10-spotify-car-thing-remote/) |
| 11 | **Stream Deck Controller** | 12-key tactile OBS Studio macro board with mic mute, webcam, live & panic switch | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/11-stream-deck-controller/) |
| 12 | **LoRa Meshtastic Terminal** | Off-grid communicator with chat log, signal RSSI/SNR, emergency SOS & presets | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/12-lora-mesh-communicator/) |
| 13 | **CAN Bus OBD-II Gauge** | Real-time tachometer (0-8k RPM), speedometer (MPH/KMH), coolant & shift lights | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/13-can-bus-obd2-gauge/) |
| 14 | **Thermal Imager (MLX90640)** | Infrared thermal imaging camera: Ironbow/Rainbow heatmaps, min/max spot crosshair & alarm | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/14-flir-thermal-camera/) |
| 15 | **Bitcoin & Lightning Monitor** | Node status: block height, BTC price, mempool sat/vB fees, channel liquidity & routed sats | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/15-bitcoin-lightning-node-monitor/) |
| 16 | **Air Quality AQI & CO2 Station** | Environmental station: EPA AQI, SCD40 CO2 ppm, PM2.5, TVOC & 24h rolling trend | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/16-air-quality-pm25-co2-station/) |
| 17 | **Solar & MPPT Dashboard** | Off-grid power flow: Solar PV watts, LiFePO4 battery SOC%, daily kWh & load | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/17-solar-battery-mppt-dashboard/) |
| 18 | **WiFi Spectrum Auditor** | Wireless security audit: Open/WEP/WPA3 cipher check, 14-ch RF spectrum & rogue APs | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/18-wifi-penetration-tester-wardrive/) |
| 19 | **GRBL CNC Motion Pendant** | Touch jog pendant: 3-axis X/Y/Z D-Pad, 0.1-50mm step selector, WCS readouts & E-STOP | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/19-cnc-grbl-motion-pendant/) |
| 20 | **DSP Audio Spectrum Analyzer** | 16-band graphic equalizer: peak-hold decay, Cyberpunk/Inferno themes & beat sync LED | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/20-dsp-audio-spectrum-analyzer/) |
| 21 | **Plant & Soil Microclimate** | Greenhouse station: 4-zone soil moisture bars, VPD kPa calculation & pump trigger | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/21-plant-soil-microclimate-monitor/) |
| 22 | **NFC & RFID Tag Analyzer** | PN532 card reader: UID hex inspection, Mifare/NTAG type, memory dump & access | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/22-nfc-rfid-access-cloner/) |
| 23 | **GPS Navigator & Speedometer** | Outdoor HUD: digital speedometer (KMH/MPH), rotating compass & WGS84 coords | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/23-gps-nav-speedometer-compass/) |
| 24 | **Doppler Weather Radar Map** | Animated NEXRAD rain radar loops, 20-70 dBZ storm reflectivity & storm warnings | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/24-weather-radar-doppler-map/) |
| 25 | **Electric Meter Pulse Counter** | Optical 1000 imp/kWh pulse counter: real-time kW, daily kWh & electricity tariff bill | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/25-smart-electric-meter-pulse/) |
| 26 | **Air-Gapped Cold Wallet** | Offline crypto signer: BIP39 SegWit derivation, PSBT transaction review & QR emitter | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/26-hardware-crypto-cold-wallet/) |

---

## 🖨️ 3D Printable Enclosures

Custom-engineered 3D printed housings designed by [@Makerrdesigns](https://youtube.com/@Makerrdesigns):

- 📐 **[2.8" Desktop Wedge Stand](enclosures/cyd-2.8-desktop-stand/)** — 60° ergonomic desk angle, 100% snap-fit (no screws!)
- 🧱 **[2.8" Flush Wall Mount](enclosures/cyd-2.8-wall-mount/)** — Slimline smart home wall controller housing
- 📐 **[3.5" Heavy-Duty Angled Stand](enclosures/cyd-3.5-angled-case/)** — 45° instrument stand with M3 hardware

---

## 🤝 Contributing

We welcome contributions! Whether it's a new project, a bug fix, documentation improvements, or 3D enclosure CAD files:
- Please read our [CONTRIBUTING.md](CONTRIBUTING.md) guide.
- Review the [Code of Conduct](CODE_OF_CONDUCT.md).

---

## 📜 Dual License

- 💻 **Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0)
- 🎨 **3D Models, Enclosures, Artwork, Graphics & Media**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)
- 💼 **Commercial Licensing**: All 3D models, animations, and branding are the intellectual property of Tushar Aggarwal. Commercial use, physical sales, or redistribution requires purchasing a commercial license or donating via [GitHub Sponsors](https://github.com/sponsors/tushar2704) / Gumroad. See [LICENSE](LICENSE) for details.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://github.com/tushar2704">Tushar Aggarwal (@tushar2704)</a></b><br>
  <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a> &bull; <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a>
</p>
