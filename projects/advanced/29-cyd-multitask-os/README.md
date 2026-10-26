# ⚡ 29 - CYD MultiTask OS: FreeRTOS Dual-Core Multitasking Operating System

> Preemptive dual-core embedded operating system for the Cheap Yellow Display (ESP32-2432S028R). Features background daemons on Core 0, a 60 FPS UI engine on Core 1, card-style app switcher, floating Picture-in-Picture (PiP) widget, and thread-safe FreeRTOS queues.

[![Web Flasher](https://img.shields.io/badge/Web%20Flasher-Ready%20to%20Flash-brightgreen.svg)](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/)
[![License: GNU AGPLv3](https://img.shields.io/badge/License-AGPLv3-blue.svg)](../../LICENSE)

---

## 📺 Community Channels
- 🎬 **Tech, Coding & Build Videos**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printing, Enclosures & CAD**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🎯 Architecture: Dual-Core FreeRTOS

```
┌───────────────────────────────────────────────┐
│              ESP32 DUAL-CORE SOC              │
├───────────────────────┬───────────────────────┤
│    CORE 0 (DAEMONS)   │    CORE 1 (UI ENGINE) │
├───────────────────────┼───────────────────────┤
│ • Music Synthesizer   │ • 60 FPS Window Mgr   │
│ • WiFi RF Sniffer     │ • Task Switcher Cards │
│ • LDR Sensor Sampler  │ • Floating PiP Widget │
│ • System Profiler     │ • Toast Notification  │
└───────────────────────┴───────────────────────┘
```

- **Core 0 (Background Daemons)**:
  - **Chiptune Audio Player**: Streams non-blocking 8-bit melodies through GPIO 26 without audio stutter or lag when heavy graphics are rendered.
  - **WiFi RF Sniffer**: Continuously scans 2.4GHz channels and broadcasts discovered AP counts via thread-safe FreeRTOS queues (`xQueueSend`).
  - **LDR Sensor Daemon**: High-frequency 12.5Hz sampling of ambient light levels and delta alerts.
  - **System Profiler**: Real-time CPU utilization % and FreeRTOS stack high-water mark tracking.
- **Core 1 (Foreground UI Engine)**:
  - **Card-Style Task Switcher**: Tap `[TASKS]` to see all running background daemons with live memory usage.
  - **Floating Picture-in-Picture (PiP)**: Mini audio player and track indicator floating in the top corner while multitasking.
  - **5 Built-In Apps**: Audio Synth Player, LDR Sensor Oscilloscope, Freehand Touch Sketchpad, Millisecond Lap Stopwatch, and FreeRTOS Task Manager.

---

## 🛠️ Hardware Pinout (ESP32-2432S028R)

| Component | Pin / GPIO | Notes |
|---|---|---|
| **Display** | ILI9341 SPI (MOSI: 13, MISO: 12, SCLK: 14, CS: 15, DC: 2, BL: 21) | 320x240 Landscape |
| **Touchscreen** | XPT2046 SPI (CS: 33, IRQ: 36, CLK: 14, MISO: 12, MOSI: 13) | VSPI Bus |
| **Audio Amp** | GPIO 26 (LEDC / DAC PWM) | FM8002A 2W Amp |
| **RGB LED** | GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) | Active LOW |
| **LDR Sensor** | GPIO 34 (ADC1 Channel 6) | Light Sensor |

---

## 🚀 Quick Start (Web Flasher)

1. Open the [CYD Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/flasher/) in Google Chrome or Edge.
2. Select **29 — CYD MultiTask OS**.
3. Click **Install** to flash in under 60 seconds with zero compiler setup!

---

## 💻 Building with PlatformIO

```bash
git clone https://github.com/tushar2704/Cheap-Yellow-Display-Community.git
cd Cheap-Yellow-Display-Community/projects/advanced/29-cyd-multitask-os
pio run --target upload
```

---

## 📜 License
Licensed under GNU AGPLv3. See [LICENSE](../../LICENSE) for details.
