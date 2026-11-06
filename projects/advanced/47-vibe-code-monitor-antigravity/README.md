# 47 — Antigravity Vibe Monitor

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A real-time desk display for [Google Antigravity](https://antigravity.dev) —
the Gemini-powered AI coding tool — showing live session metrics pulled from
your local machine via a tiny Python bridge. No cloud, no API keys: everything
is read straight from `~/.gemini/antigravity/`.

---

## Features

### 📊 Screen 0 — Live Session Dashboard
- Tokens in / out with animated progress bars
- Tool calls and subagent count
- Session duration and active model name
- Live / Offline indicator

### 🔥 Screen 1 — Streak & History
- Current and best streak display
- Today's conversations, tool calls, and subagents
- Coloured status badge (Active / Idle / Error)

### 🕐 Screen 2 — Standby / Clock
- NTP-synced large clock (HH:MM)
- Day and date
- Last session summary

### 💡 RGB LED Feedback
| State | Colour | Pattern |
|-------|--------|---------|
| Active, low usage (<50 tools) | Cyan | Breathe (2 s) |
| Active, high usage (≥50 tools) | Yellow | Pulse (1 s) |
| Idle | Blue | Solid |
| Bridge offline / error | Red | Blink (500 ms) |

---

## Hardware Required

- **ESP32-2432S028R** ("Cheap Yellow Display") — 2.8", ILI9341 320×240 + XPT2046 touch
- USB-C cable for flashing

No extra wiring needed — touch, display, and LED are all on-board.

---

## Quick Start

### 1. Run the Python bridge on your Mac / PC

```bash
cd bridge
pip3 install -r requirements.txt
python3 monitor.py
```

The bridge prints your LAN IP and the exact line to paste into the firmware.

### 2. Find your LAN IP

```bash
# macOS
ipconfig getifaddr en0

# Windows
ipconfig        # look for IPv4 Address
```

### 3. Configure the firmware

Edit `src/main.cpp` and set the four values at the top:

```cpp
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* BRIDGE_HOST   = "192.168.1.xxx";  // ← your machine's IP
const int   BRIDGE_PORT   = 5047;
```

### 4. Flash the CYD

```bash
pio run --target upload
```

---

## How It Works

```
┌─────────────────────┐         HTTP GET /api/antigravity          ┌────────────────┐
│   CYD (ESP32)       │  ──────────────────────────────────────►   │  bridge/       │
│   src/main.cpp      │  ◄──────────────────────────────────────   │  monitor.py    │
│                     │            JSON response                    │                │
│  · 3 info screens   │                                             │  Reads local   │
│  · RGB LED          │                                             │  transcript    │
│  · NTP clock        │                                             │  .jsonl files  │
└─────────────────────┘                                             └────────────────┘
                                                                           │
                                                                           ▼
                                                               ~/.gemini/antigravity/
                                                                   brain/**/transcript.jsonl
```

The bridge scans every `transcript.jsonl` under `~/.gemini/antigravity/brain/`
and counts tool calls, subagent spawns, and token estimates for today's date.
Data refreshes every **30 seconds** on the display.

---

## Wiring

No extra components required. All peripherals are built into the CYD board:

| Peripheral | Interface | Pins (pre-wired) |
|------------|-----------|------------------|
| ILI9341 display | SPI (VSPI) | Built-in |
| XPT2046 touch | SPI (HSPI) | CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36 |
| RGB LED | GPIO (Active LOW) | R=4, G=16, B=17 |
| LDR | ADC | GPIO34 |

---

## PlatformIO Dependencies

```ini
lib_deps =
    bodmer/TFT_eSPI@^2.5.43
    https://github.com/PaulStoffregen/XPT2046_Touchscreen.git
    bblanchon/ArduinoJson@^7.0.0
    arduino-libraries/NTPClient@^3.2.1
```

---

## Credits

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## License

MIT © 2024 Cheap Yellow Display Community
