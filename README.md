# 🟡 Cheap Yellow Display Community — The Ultimate CYD Hub

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESP32](https://img.shields.io/badge/ESP32-Supported-blue.svg)](https://www.espressif.com/)
[![ESP32-S3](https://img.shields.io/badge/ESP32--S3-Supported-blue.svg)](https://www.espressif.com/)
[![Arduino](https://img.shields.io/badge/Arduino-Compatible-teal.svg)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Ready-orange.svg)](https://platformio.org/)

> **The ultimate open-source community hub for ESP32 Cheap Yellow Display (CYD) — covering every size, every variant, and every project you can imagine.**

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

This project is brought to you by two YouTube channels — follow both for tutorials, builds, and behind-the-scenes content:

| Channel | Focus | Link |
|---------|-------|------|
| 🎬 **IamNotIntoTech** | Tech projects with humor — CYD tutorials, ESP32 builds, "I tried it" experiments | [▶️ youtube.com/@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) |
| 🖨️ **Makerrdesigns** | 3D design & printing — CYD enclosures, Fusion 360 tutorials, maker builds | [▶️ youtube.com/@Makerrdesigns](https://youtube.com/@Makerrdesigns) |

---

## 📐 Supported CYD Variants

This repository supports **ALL** Cheap Yellow Display variants currently available. Here's the complete family:

| Model | Screen Size | Resolution | Touch Type | ESP32 Variant | Display Driver |
|-------|------------|------------|------------|---------------|----------------|
| ESP32-2432S024 | 2.4" | 320×240 | Resistive | ESP32-WROOM-32 | ILI9341 |
| **ESP32-2432S028R** | **2.8"** | **320×240** | **Resistive** | **ESP32-WROOM-32** | **ILI9341** |
| ESP32-2432S028C | 2.8" | 320×240 | Capacitive | ESP32-WROOM-32 | ILI9341 |
| ESP32-3248S032 | 3.2" | 320×240 | Resistive | ESP32-WROOM-32 | ILI9341 |
| ESP32-3248S035R | 3.5" | 480×320 | Resistive | ESP32-WROOM-32 | ST7796 |
| ESP32-3248S035C | 3.5" | 480×320 | Capacitive | ESP32-WROOM-32 | ST7796 |
| ESP32-4048S040 | 4.0" | 480×480 | Capacitive | ESP32-S3 | ST7701 |
| ESP32-8048S050 | 5.0" | 800×480 | Capacitive | ESP32-S3 | RGB |
| ESP32-8048S070 | 7.0" | 800×480 | Capacitive | ESP32-S3 | RGB |

> 💡 **Not sure which CYD you have?** Check our [Variant Identification Guide →](docs/VARIANTS.md)

---

## 🚀 Getting Started with Your Cheap Yellow Display

New to CYD? Start here:

1. **[Setup Guide](docs/SETUP.md)** — Install Arduino IDE or PlatformIO and configure for your CYD
2. **[Pin Reference](docs/PINOUT.md)** — Know what every pin does on your board
3. **[Your First Project](projects/beginner/01-hello-cyd/)** — Flash "Hello CYD" to test your display *(coming soon)*
4. **[Troubleshooting](docs/TROUBLESHOOTING.md)** — Fix common issues with drivers, uploads, and display

### Quick Start (Arduino IDE)

```cpp
// Minimal CYD test — displays text on screen
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 100);
  tft.println("Hello, CYD!");
}

void loop() {}
```

---

## 📂 Repository Structure

```
Cheap-Yellow-Display-Community/
├── README.md                    ← You are here
├── docs/                        ← Guides, pinouts, troubleshooting
│   ├── SETUP.md                 ← Getting started
│   ├── PINOUT.md                ← Pin mappings for all variants
│   ├── VARIANTS.md              ← CYD model identification
│   ├── TROUBLESHOOTING.md       ← Common fixes
│   └── FAQ.md                   ← Frequently asked questions
├── projects/                    ← Ready-to-flash projects
│   ├── beginner/                ← Easy starter projects
│   ├── intermediate/            ← IoT dashboards & connected projects
│   └── advanced/                ← Advanced builds & premium projects
├── enclosures/                  ← 3D printable cases (STL/STEP)
├── hardware/                    ← Pinouts, schematics, datasheets
├── web-flasher/                 ← Browser-based firmware flasher (coming soon)
└── assets/                      ← Images, banners, social media assets
```

---

## 🛠️ Projects

### Beginner Projects (Free & Open Source)
| # | Project | Compatible CYD | Difficulty |
|---|---------|---------------|------------|
| 1 | Hello CYD — Display test & touch calibration | All | ⭐ |
| 2 | WiFi Scanner — Scan & display nearby networks | All | ⭐ |
| 3 | Digital Clock — NTP synced clock with themes | All | ⭐⭐ |
| 4 | System Monitor — PC stats via serial | 2.8"+ | ⭐⭐ |
| 5 | Photo Frame — SD card image slideshow | 3.5"+ | ⭐⭐ |

### Intermediate Projects
| # | Project | Compatible CYD | Difficulty |
|---|---------|---------------|------------|
| 1 | Weather Station Dashboard | 2.8"+ | ⭐⭐⭐ |
| 2 | Home Assistant Control Panel | 3.5"+ | ⭐⭐⭐ |
| 3 | Spotify Now Playing Display | 2.8"+ | ⭐⭐⭐ |
| 4 | Smart Thermostat UI | 2.8"+ | ⭐⭐⭐ |
| 5 | Stock & Crypto Ticker | 2.8"+ | ⭐⭐⭐ |
| 6 | YouTube Sub Counter | 2.8"+ | ⭐⭐ |
| 7 | Pomodoro Timer | All | ⭐⭐ |
| 8 | MQTT Dashboard | 3.5"+ | ⭐⭐⭐ |

### Advanced Projects
| # | Project | Compatible CYD | Difficulty |
|---|---------|---------------|------------|
| 1 | CYD OS — Simple launcher/operating system | All | ⭐⭐⭐⭐ |
| 2 | Multi-Screen Dashboard — ESP-NOW linked CYDs | All | ⭐⭐⭐⭐ |
| 3 | 3D Printer Controller — Machine status display | 3.5"+ | ⭐⭐⭐⭐ |

> 📌 **Projects are being added regularly!** Star ⭐ this repo and watch for updates.

---

## 🖨️ 3D Printable Enclosures

Every CYD deserves a home! We design custom 3D printable enclosures for all CYD sizes:

- **Desktop stands** — Angled for comfortable viewing
- **Wall mounts** — For smart home dashboards
- **Flush mounts** — For panel installations
- **Portable cases** — With battery compartments

Enclosure designs are available in **STL** and **STEP** formats. Videos on designing and printing these are available on [Makerrdesigns →](https://youtube.com/@Makerrdesigns)

---

## 🌐 Web Flasher (Coming Soon)

Flash your CYD directly from your browser — no Arduino IDE required! Our upcoming **Web Flasher** will let you:

- Pick a project from the gallery
- Connect your CYD via USB
- Flash firmware with one click
- Configure WiFi credentials before flashing

---

## 🛒 Where to Buy a Cheap Yellow Display

CYDs are available from many sellers. Here are reliable sources:

- **AliExpress** — Search for the model number (e.g., "ESP32-2432S028R")
- **Makerfabs** — [makerfabs.com](https://www.makerfabs.com/) — Great quality, tested boards
- **Amazon** — Search "ESP32 TFT touch display" — usually faster shipping

> 💡 **Tip**: The 2.8" resistive touch model (ESP32-2432S028R) is the most popular and best supported — start there if you're new!

---

## 🤝 Contributing

We welcome contributions from everyone! Whether it's a new project, a bug fix, a documentation improvement, or a 3D enclosure design — we'd love to have you.

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

---

## ❓ FAQ

**Q: What's the difference between resistive and capacitive touch?**
A: Resistive touch requires pressure (like using a stylus), while capacitive touch works with finger proximity (like your phone). Capacitive is smoother but slightly more expensive.

**Q: Can I use these displays with ESPHome?**
A: Yes! Many CYD variants work with ESPHome for Home Assistant integration. Check the [Setup Guide](docs/SETUP.md) for ESPHome configuration.

**Q: Which libraries should I use?**
A: We recommend **TFT_eSPI** for display and **XPT2046_Touchscreen** or **FT6236** for touch (depending on your variant). See [Setup Guide](docs/SETUP.md).

**Q: Can I sell projects I build using this repo?**
A: Yes! This project is MIT licensed. You're free to use, modify, and sell anything you build.

See more in our [FAQ →](docs/FAQ.md)

---

## 📜 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## ⭐ Support This Project

If this repo helps you, please consider:

- ⭐ **Starring** this repository
- 📺 **Subscribing** to [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) and [Makerrdesigns](https://youtube.com/@Makerrdesigns)
- 🔗 **Sharing** with your maker friends
- 🐛 **Reporting issues** or **submitting projects**

---

<p align="center">
  <b>Made with 🟡 by <a href="https://github.com/tushar2704">Tushar Aggarwal</a></b><br>
  <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a> · <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a>
</p>
