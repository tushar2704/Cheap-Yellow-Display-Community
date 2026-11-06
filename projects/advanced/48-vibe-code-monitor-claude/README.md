# 48 — Claude Code Vibe Monitor

> **Real-time Claude Code rate-limit and token usage monitor on a 2.8" desk display.**  
> Never get rate-limited mid-flow again.

[![Difficulty](https://img.shields.io/badge/Difficulty-⭐⭐⭐⭐-orange)]()
[![Platform](https://img.shields.io/badge/Platform-ESP32--2432S028R-blue)]()
[![Framework](https://img.shields.io/badge/Framework-Arduino%20%2F%20PlatformIO-green)]()

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## Overview

This project turns your **Cheap Yellow Display (CYD)** into a live desk dashboard for **Claude Code** — Anthropic's AI coding tool. It tracks the critical **5-hour rolling window** (the Pro rate-limit window), 7-day cumulative tokens, today's session cost, and coding activity — all from your local machine, no cloud credentials on the device.

A **Python bridge** running on your Mac/PC reads Claude Code's local `~/.claude/projects/**/*.jsonl` files and serves the parsed metrics over HTTP to the ESP32. The display polls every 30 seconds and lights up an **RGB LED** as a heads-up warning when you're approaching the limit.

---

## Features

| Feature | Detail |
|---------|--------|
| **3 screens** | Tap to cycle: Window Gauge → Token Economics → Warning/Standby |
| **5-hour window gauge** | Large visual progress bar, % used, reset countdown |
| **7-day window** | Thinner secondary bar showing weekly cumulative |
| **Token economics** | Per-category breakdown: input / output / cache write / cache read with mini bars |
| **Cost tracking** | Live USD cost estimate using Claude 3.5 Sonnet pricing |
| **RGB LED warning** | Orange breathe → amber pulse → red rapid blink as you approach limit |
| **Warning screen** | Pulsing red border + big "SLOW DOWN!" when >80% used |
| **Standby clock** | NTP clock + last session summary when idle |
| **Local-only** | No API keys on device — bridge reads local JSONL files |
| **Offline resilient** | Shows clock + "BRIDGE OFFLINE" notice when bridge is unreachable |

---

## Hardware Required

| Component | Details |
|-----------|---------|
| ESP32-2432S028R | Cheap Yellow Display 2.8" (CYD) |
| ILI9341 display | 320×240 TFT, built-in |
| XPT2046 touch | Resistive touch, built-in |
| RGB LED | Built-in (GPIO 4/16/17, active LOW) |

---

## Project Structure

```
48-vibe-code-monitor-claude/
├── platformio.ini          ← PlatformIO build config
├── src/
│   └── main.cpp            ← Full ESP32 firmware
├── bridge/
│   ├── monitor.py          ← Python Flask bridge
│   ├── requirements.txt    ← Python deps (flask)
│   └── README.md           ← Bridge setup guide
└── README.md               ← This file
```

---

## Quick Start

### Step 1 — Run the Python bridge on your Mac/PC

```bash
cd bridge/
pip3 install -r requirements.txt
python3 monitor.py
```

The bridge will scan `~/.claude/projects/` and start serving on port **5048**.

### Step 2 — Find your computer's local IP

```bash
# macOS
ipconfig getifaddr en0

# Linux
hostname -I

# Windows
ipconfig | findstr "IPv4"
```

It will look like `192.168.1.42`.

### Step 3 — Configure the firmware

Open `src/main.cpp` and edit the config block at the top:

```cpp
// ===== USER CONFIGURATION =====
const char* WIFI_SSID     = "YourWiFiName";
const char* WIFI_PASSWORD = "YourPassword";
const char* BRIDGE_HOST   = "192.168.1.42";  // ← your Mac/PC IP
const int   BRIDGE_PORT   = 5048;
// ==============================
```

### Step 4 — Flash the ESP32

```bash
pio run --target upload
pio device monitor
```

The display will show a boot screen, connect to WiFi, fetch data, and display the **5-Hour Window Gauge**.

---

## Screens

### Screen 0 — 5-Hour Window Gauge (Primary)

```
┌─ ⚡ CLAUDE CODE ────────────────────── ● LIVE ─┐
│═══════════════════════════════════════════════│
│          5-HOUR WINDOW                        │
│ ████████████████████░░░░░░░░░░░░░  64%        │
│ 128.0K / 200.0K   Resets: 1h 23m             │
│───────────────────────────────────────────────│
│          7-DAY WINDOW                         │
│ ████░░░░░░░░░░░░░░░░░░░░░░░░░░░  12%          │
│ 240.0K tokens used this week                  │
│───────────────────────────────────────────────│
│    COST: $0.284   FILES: 12   CMDS: 34        │
│                                               │
│            TAP TO CYCLE SCREENS               │
└───────────────────────────────────────────────┘
```

### Screen 1 — Token Economics

```
┌─ $ SESSION ECONOMICS ───────────────── LIVE ──┐
│ INPUT   ████████████████░░  38,200  $0.1146   │
│ OUTPUT  █████░░░░░░░░░░░░░  12,400  $0.1860   │
│ CACHE+  ████░░░░░░░░░░░░░░   8,000  $0.0300   │
│ CACHE-  ████████████████░░  95,000  $0.0285   │
│═══════════════════════════════════════════════│
│ TOTAL  153.6K tokens   $0.3591                │
│ FILES EDITED: 12   CMDS RUN: 34              │
└───────────────────────────────────────────────┘
```

### Screen 2a — Warning (>80% used)

```
┌══════════════════════════════════════════════╗
║           !! WARNING !!                      ║
║                                              ║
║          ! SLOW DOWN!                        ║
║                                              ║
║               87%                            ║
║          of 5h window used                   ║
║                                              ║
║             42m                              ║
║          until window resets                 ║
║                                              ║
║ ████████████████████████████████████  87%   ║
╚══════════════════════════════════════════════╝
```

### Screen 2b — Standby (idle)

```
┌─ ⚡ CLAUDE CODE — STANDBY ─────────────────┐
│                                             │
│            14:32:08                         │
│         Friday, 06 Nov 2026                 │
│─────────────────────────────────────────────│
│       LAST SESSION SUMMARY                  │
│  Tokens: 38.2K in / 12.4K out             │
│  Cost today: $0.359                        │
│  Files edited: 12   Commands: 34           │
└─────────────────────────────────────────────┘
```

---

## RGB LED Behavior

| Condition | LED |
|-----------|-----|
| <50% window used | 🟠 Orange — slow breathe (2s) |
| 50–80% used | 🟡 Amber — fast pulse (0.8s) |
| >80% used | 🔴 Red — rapid blink (0.3s) |
| Bridge offline / no data | 🔵 Blue dim solid |
| Status: idle | 🟢 Green solid |

---

## How It Works

```
Claude Code (your terminal)
      │
      ▼ writes
~/.claude/projects/**/*.jsonl
      │
      ▼ reads every request
bridge/monitor.py  (Python Flask, port 5048)
      │
      ▼ HTTP GET /api/claude every 30s
ESP32 CYD (src/main.cpp)
      │
      ▼ renders
ILI9341 2.8" TFT Display
```

Claude Code writes one JSONL line per conversation turn, each containing token usage data. The bridge aggregates these into rolling windows and serves a clean JSON payload. The ESP32 fetches, parses, and renders — entirely on your local network.

---

## Configuration Reference

### `src/main.cpp`

| Constant | Default | Description |
|----------|---------|-------------|
| `WIFI_SSID` | `"YOUR_WIFI_SSID"` | Your WiFi network name |
| `WIFI_PASSWORD` | `"YOUR_WIFI_PASSWORD"` | Your WiFi password |
| `BRIDGE_HOST` | `"192.168.1.100"` | IP of Mac/PC running bridge |
| `BRIDGE_PORT` | `5048` | Bridge port (match monitor.py) |
| `FETCH_INTERVAL` | `30000` | Poll interval in ms (default 30s) |

### `bridge/monitor.py`

| Constant | Default | Description |
|----------|---------|-------------|
| `WINDOW_5H_LIMIT` | `200000` | Tokens in 5h window (Pro tier) |
| `WINDOW_7D_LIMIT` | `2000000` | Tokens in 7-day window |
| `PORT` | `5048` | HTTP port |

---

## Dependencies

### Firmware (PlatformIO)

| Library | Version |
|---------|---------|
| `bodmer/TFT_eSPI` | ^2.5.43 |
| `paulstoffregen/XPT2046_Touchscreen` | ^1.4 |
| `bblanchon/ArduinoJson` | ^7.0.0 |
| `arduino-libraries/NTPClient` | ^3.2.1 |

### Bridge (Python)

| Package | Version |
|---------|---------|
| `flask` | ≥3.0.0 |

---

## TFT_eSPI Configuration

This project uses the **ESP32-2432S028R** (CYD). Ensure your `User_Setup.h` (or the library's auto-config) has:

```cpp
#define ILI9341_DRIVER
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TOUCH_CS 33
#define SPI_FREQUENCY       55000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
```

If using the community `User_Setup_Select.h`, include the CYD-specific setup file.

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| "BRIDGE OFFLINE" on display | Verify IP in `main.cpp`, check bridge is running, check firewall allows port 5048 |
| All zeros / no data | Run `curl http://BRIDGE_IP:5048/debug` to see file list; use Claude Code first to generate data |
| White screen | TFT_eSPI `User_Setup.h` misconfigured for your CYD board |
| Touch not working | Check HSPI pin definitions (CLK=25, MISO=39, MOSI=32, CS=33) |
| WiFi timeout | Move ESP32 closer to router; verify SSID/password |
| NTP time wrong | NTP returns UTC; configure timezone offset in `NTPClient` constructor |

---

## Credits

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## License

MIT © Cheap Yellow Display Community
