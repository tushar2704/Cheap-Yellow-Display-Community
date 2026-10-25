# 🎛️ Project 11 — CYD 2.8" Stream Deck 12-Key Touch Macro Controller

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a programmable 12-key Stream Deck controller for OBS Studio, Twitch streaming, Discord, and desktop productivity. Features a 4x3 interactive grid with active button glows, live status header, tactile audio feedback, and multi-color RGB state indicators. Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Key Grid**: 12 custom touch tiles (4 columns x 3 rows)
- **Host Integration**: OBS WebSocket, USB Serial Macro, HTTP REST
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `WiFi`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the 12-Key Stream Deck Setup & OBS Integration**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Desktop Wedge Stand**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **12 Interactive Touch Macros**:
  - 🎙️ **MIC MUTE**: Red warning badge when muted; dark slate when live.
  - 📹 **WEBCAM**: Toggle camera feed on/off.
  - 🔴 **STREAM LIVE**: Header turns to glowing `● LIVE` badge; RGB turns solid Red.
  - 🎮 **SCENE 1 (GAME)**: Instantly switch to game capture layout.
  - 💬 **SCENE 2 (CHAT)**: Switch to full webcam + chat screen.
  - ⏸️ **BRB PAUSE**: Amber interlude banner.
  - 🎧 **DISCORD DEAFEN**: One-tap toggle for team audio.
  - 🎵 **SPOTIFY SKIP**: Forward to next track in queue.
  - 📸 **SCREENSHOT**: Desktop screen capture trigger.
  - 💡 **KEY LIGHT**: Studio desk illumination toggle.
  - 📁 **OBS STUDIO**: Window focus & restore macro.
  - 🛑 **PANIC BUTTON**: Emergency scene cut / black screen.
- **RGB Status Indicator**:
  - 🔴 **Red**: Live stream actively transmitting or mic muted.
  - 🟢 **Green**: Standby & ready.
  - 🔵 **Blue Flash**: Scene transition command sent.
- **Acoustic Feedback**: Frequency-varied chimes on every button touch via onboard speaker (GPIO 26).

---

## 🚀 How to Flash & Configure

### 1. Flash Directly in Your Browser (No Coding Required)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install Stream Deck Controller**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/11-stream-deck-controller
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)
