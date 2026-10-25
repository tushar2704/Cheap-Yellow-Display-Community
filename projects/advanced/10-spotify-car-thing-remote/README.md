# 🟢 Project 10 — CYD 2.8" Spotify Car Thing Touch Remote & Now Playing Canvas

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a dedicated Spotify Car Thing replacement touch remote. Features high-contrast Spotify Dark UI, vinyl album art canvas, live track elapsed/duration scrubber bar, oversized touch transport buttons (Play, Pause, Skip, Back), interactive volume touch slider, and RGB playback state glow. Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Audio Service**: Spotify Web API / Media Controller
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `WiFi`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the Spotify Car Thing CYD Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Car Dash & Desktop Mount**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Car Thing Inspired Dark Canvas**: High-legibility typography designed for in-car glanceability or desktop desk companion use.
- **Album Art Frame**: Stylized retro-vinyl spinning record motif with Spotify green accents.
- **Interactive Touch Transport**:
  - `|<<` **PREVIOUS**: Restarts current track or steps back.
  - ⏯️ **PLAY / PAUSE**: High-visibility center control pill with state color shifts.
  - `>>|` **NEXT**: Skips to the next queued song.
- **Continuous Scrubber Bar**: 1-second interval playback progress bar with elapsed and remaining timestamps.
- **Volume Slider**: Tap along the horizontal volume track to adjust output percentage (0-100%).
- **Onboard RGB LED Status**:
  - 🟢 **Green**: Track actively playing.
  - 🔵 **Blue**: Playback paused / idle.
- **Tactile Audio Clicks**: Gentle feedback beeps via onboard piezo (GPIO 26) on all touch controls.

---

## 🚀 How to Flash & Configure

### 1. Flash Directly in Your Browser (No Software to Install)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install Spotify Car Thing Remote**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/10-spotify-car-thing-remote
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)
