# 49 — OpenAI Codex Vibe Monitor

**Difficulty:** ⭐⭐⭐⭐

A real-time OpenAI Codex CLI usage monitor for the **Cheap Yellow Display (CYD)** ESP32-2432S028R.
Watch your Codex token consumption, reasoning costs, cache efficiency, and task history
live on a 2.8" display — Matrix-green terminal aesthetic included.

---

## Features

- **3 touch-cycled screens:**
  - **[CODEX CLI] Command Center** — 5-hour window bar, reasoning & cache-hit mini-bars, tasks/patches/cost summary
  - **[REASONING] Breakdown** — 4-bar token type comparison (Input / Output / Reasoning / Cached) with cost and efficiency
  - **[RECENT TASKS] History** — Last 5 task descriptions with task + patch count; falls back to NTP terminal clock when idle
- **RGB LED status:**
  - 🟢 Green breathe — active session
  - 🟡 Yellow pulse — 5h window >70% used
  - 🔴 Red rapid blink — 5h window >90% used (slow down!)
  - 🔴 Red solid — bridge offline / no data
- **Reasoning token tracking** — unique o4-mini/o3 feature highlighted with magenta accent
- **Cache hit efficiency** — colored indicator (green/yellow/red) based on cache hit percentage
- **NTP clock fallback** on idle with blinking `_` cursor
- **Matrix/terminal aesthetic** — black background, Matrix green (`#00FF41`), ASCII decorators

---

## Hardware

| Component | Detail |
|-----------|--------|
| Board | ESP32-2432S028R (Cheap Yellow Display 2.8") |
| Display | ILI9341 320×240, rotation=1 |
| Touch | XPT2046 (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36) |
| RGB LED | R=GPIO4, G=GPIO16, B=GPIO17 (Active LOW) |
| Connectivity | WiFi → HTTP GET from bridge |

---

## Quick Start

### 1. Run the Python bridge on your computer

```bash
cd bridge/
pip install -r requirements.txt
python3 monitor.py
```

Find your computer's IP:
```bash
# macOS
ipconfig getifaddr en0
```

### 2. Configure the ESP32

Edit the top of `src/main.cpp`:

```cpp
const char* WIFI_SSID     = "YourNetworkName";
const char* WIFI_PASSWORD = "YourPassword";
const char* BRIDGE_HOST   = "192.168.1.42";  // Your computer's IP
const int   BRIDGE_PORT   = 5049;
```

### 3. Flash

```bash
pio run -t upload
pio device monitor   # 115200 baud
```

### 4. Use it

Tap the display to cycle through the 3 screens. The CYD polls every **30 seconds**.

---

## How It Works

```
OpenAI Codex CLI
     │
     ▼  writes to
~/.codex/sessions/*.jsonl
     │
     ▼  read by
bridge/monitor.py  (Flask, port 5049)
     │
     ▼  HTTP GET /api/codex (JSON)
ESP32 CYD  (WiFi)
     │
     ▼
ILI9341 Display  +  RGB LED
```

The bridge scans all JSONL session files, aggregates token counts by time window
(today / 5-hour rolling / 7-day rolling), calculates cost using o4-mini pricing,
and counts task completions and patch tool calls.

---

## API Response (bridge)

```json
{
  "tool": "codex",
  "status": "active",
  "window_5h":  { "used_pct": 12.3, "reset_in_min": 240, "tokens_used": 24600 },
  "window_7d":  { "used_pct": 5.1,  "tokens_used": 102000 },
  "session": {
    "input_tokens": 18000, "output_tokens": 4200,
    "reasoning_tokens": 2400, "cached_input_tokens": 1000,
    "cost_usd": 0.0421, "cache_hit_pct": 5.3
  },
  "tasks_completed": 7,
  "patches_applied": 12,
  "recent_tasks": ["Fix login bug", "Add dark mode"],
  "model": "o4-mini"
}
```

---

## Project Structure

```
49-vibe-code-monitor-codex/
├── platformio.ini
├── 49-vibe-code-monitor-codex.ino   (stub — PlatformIO project)
├── src/
│   └── main.cpp                     (full firmware)
├── bridge/
│   ├── monitor.py                   (Python Flask bridge)
│   ├── requirements.txt
│   └── README.md
└── assets/                          (screenshots, enclosure files)
```

---

## Dependencies

**Firmware (PlatformIO):**
- `bodmer/TFT_eSPI` ^2.5.43
- `paulstoffregen/XPT2046_Touchscreen` ^1.4
- `bblanchon/ArduinoJson` ^7.0.0
- `arduino-libraries/NTPClient` ^3.2.1

**Bridge (Python):**
- `flask>=3.0.0`

---

## Pricing Reference (o4-mini)

| Token type   | Price          |
|--------------|----------------|
| Input        | \$1.10 / MTok  |
| Output       | \$4.40 / MTok  |
| Reasoning    | \$1.10 / MTok  |
| Cached Input | \$0.275 / MTok |

> Update `PRICE_*` constants in `bridge/monitor.py` if OpenAI changes pricing.

---

## Credits

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## License

MIT © Cheap Yellow Display Community
