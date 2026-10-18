# 🟡 Cheap Yellow Display Community — The Ultimate CYD Hub

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL_v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)
[![License: CC BY-NC-ND 4.0](https://img.shields.io/badge/Assets-CC_BY--NC--ND_4.0-lightgrey.svg)](https://creativecommons.org/licenses/by-nc-nd/4.0/)
[![ESP32](https://img.shields.io/badge/ESP32-Supported-blue.svg)](https://www.espressif.com/)
[![ESP32-S3](https://img.shields.io/badge/ESP32--S3-Supported-blue.svg)](https://www.espressif.com/)
[![Arduino](https://img.shields.io/badge/Arduino-Compatible-teal.svg)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Ready-orange.svg)](https://platformio.org/)
[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Live%20In--Browser-brightgreen.svg)](flasher/)

> **The ultimate open-source community hub for ESP32 Cheap Yellow Display (CYD) — covering every size, every variant, 3D enclosures, hardware mods, and projects you can flash in 1-click directly from your browser.**

---

## ⚡ 1-Click In-Browser Web Flasher

**No Arduino IDE or PlatformIO installation required!** You can flash community firmwares onto your CYD right now using Google Chrome, Edge, or Opera:

👉 **[Launch the CYD Web Flasher →](flasher/)**

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
- 🔌 **[Pinout Reference](docs/PINOUT.md)** — Pin maps for all display sizes
- ⚙️ **[Universal Pin Header](hardware/cyd_pins.h)** — One header file to configure any CYD board
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

### 🟡 Intermediate Projects (All for 2.8" CYD — Connected Dashboards)
| # | Project | Description | Difficulty | Guide |
|---|---|---|:---:|:---:|
| 1 | **Weather Station** | Real-time weather dashboard & NTP clock via free Open-Meteo API | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/01-weather-station/) |
| 2 | **Crypto Price Tracker** | Live Bitcoin, Ethereum & Solana ticker with 24h change and sparkline | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/02-crypto-tracker/) |
| 3 | **Rolling Digit Clock** | Retro split-flap rolling clock synchronized with atomic NTP time | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/03-rolling-clock/) |
| 4 | **WiFi Internet Radio** | Live Shoutcast/Icecast radio streaming to onboard speaker via DAC | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/04-internet-radio/) |
| 5 | **LVGL v8 Showcase** | Smartphone-smooth touch animations, tabs, sliders, and arc gauges | ⭐⭐⭐ (3/5) | [Open](projects/intermediate/05-lvgl-widgets-demo/) |

### 🔴 Advanced Projects (All for 2.8" CYD — Flagship Firmware)
| # | Project | Description | Difficulty | Guide |
|---|---|---|:---:|:---:|
| 1 | **CYD OS** | Desktop launcher environment with status bar, 6 built-in apps & window manager | ⭐⭐⭐⭐ (4/5) | [Open](projects/advanced/01-cyd-os/) |

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
