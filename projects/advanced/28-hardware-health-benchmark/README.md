# ⚡ 28 - Hardware Health & Benchmark Suite for CYD 2.8"

> Comprehensive diagnostic, stress-testing, and hardware calibration suite for the Cheap Yellow Display (ESP32-2432S028R).

[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Ready%20to%20Flash-brightgreen.svg)](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../LICENSE)

---

## 📺 Community Channels
- 🎬 **Tech, Coding & Build Videos**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printing, Enclosures & CAD**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🎯 Overview

When receiving a new Cheap Yellow Display or troubleshooting hardware anomalies, this diagnostic suite exercises every onboard subsystem:
- **SPI Throughput Benchmark**: Measures full-screen fill rate, vector line drawing, circle rendering, and font blitting to calculate true FPS and render bandwidth in Megapixels/sec.
- **Display Purity & Defect Inspector**: Full-screen RGB primary color inspection, white/black backlight bleed test, and a 16-step grayscale gamma ramp.
- **Touchscreen Accuracy & Linearity**: Live raw ADC coordinates $(X, Y, Z)$ vs calibrated screen coordinates, 9-point grid alignment check targets, and continuous drawing canvas.
- **Audio & Amplifier Diagnostic**: Tests onboard FM8002A class-AB amp on GPIO 26 with 440Hz, 1kHz, 2kHz tones, and a continuous 100–3000Hz frequency chirp sweep.
- **Sensor Suite**: Real-time oscilloscope line graph for LDR light sensor (GPIO 34), ESP32 internal core temperature, Hall effect sensor, and RGB LED color exerciser.
- **System Profile & RF Analyzer**: Detailed CPU architecture profile, SPI flash memory speed/size, heap fragmentation, and active 2.4GHz WiFi spectrum scanner with RSSI bars.

---

## 🛠️ Hardware Pinout (ESP32-2432S028R)

| Component | Pin / GPIO | Notes |
|---|---|---|
| **TFT Display** | ILI9341 SPI (MOSI: 13, MISO: 12, SCLK: 14, CS: 15, DC: 2, BL: 21) | 320x240 Landscape |
| **Touchscreen** | XPT2046 SPI (CS: 33, IRQ: 36, CLK: 14, MISO: 12, MOSI: 13) | VSPI Bus |
| **Audio Amp** | GPIO 26 (DAC / LEDC PWM) | FM8002A 2W Amp |
| **RGB LED** | GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) | Active LOW |
| **LDR Sensor** | GPIO 34 (ADC1 Channel 6) | GL5528 Divider |

---

## 🚀 Quick Start (Web Flasher)

No installation required! Plug your CYD into your computer via USB-C or Micro-USB:
1. Open the [CYD Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/) in Chrome or Edge.
2. Select **28 - Hardware Health & Benchmark Suite**.
3. Click **Install** and enjoy instant zero-install testing!

---

## 💻 Building with PlatformIO

```bash
# Clone the repository
git clone https://github.com/tushar2704/Cheap-Yellow-Display-Community.git
cd Cheap-Yellow-Display-Community/projects/advanced/28-hardware-health-benchmark

# Build and upload to CYD
pio run --target upload
```

---

## 📜 License
Licensed under GNU AGPLv3. See [LICENSE](../../LICENSE) for details.
