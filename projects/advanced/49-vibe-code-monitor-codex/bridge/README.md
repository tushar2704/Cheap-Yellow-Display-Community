# Bridge — Codex Vibe Monitor

Python Flask bridge that reads OpenAI Codex CLI local session files and
serves real-time metrics to the CYD ESP32 display over HTTP.

---

## Requirements

- Python 3.9+
- OpenAI Codex CLI installed and used (creates `~/.codex/` files)
- Flask

---

## Setup

```bash
cd bridge/
pip install -r requirements.txt
python3 monitor.py
```

The server starts on **port 5049** and binds to `0.0.0.0` (all interfaces).

---

## Find your computer's IP address

**macOS / Linux:**
```bash
# macOS
ipconfig getifaddr en0

# Linux
hostname -I | awk '{print $1}'
```

**Windows:**
```cmd
ipconfig
# Look for "IPv4 Address" under your active adapter
```

You should see something like `192.168.1.42`. That is your `BRIDGE_HOST`.

---

## Configure the ESP32

Open `src/main.cpp` and update the config block at the top:

```cpp
// ===== USER CONFIGURATION =====
const char* WIFI_SSID     = "YourNetworkName";
const char* WIFI_PASSWORD = "YourPassword";
const char* BRIDGE_HOST   = "192.168.1.42";  // ← Your computer's IP
const int   BRIDGE_PORT   = 5049;
// ==============================
```

Then flash the firmware:
```bash
pio run -t upload
pio device monitor
```

---

## Verify the bridge is working

Open in a browser or use curl:
```bash
curl http://localhost:5049/api/codex | python3 -m json.tool
curl http://localhost:5049/health
```

Expected response shape:
```json
{
  "tool": "codex",
  "status": "active",
  "window_5h": { "used_pct": 12.3, "reset_in_min": 240, "tokens_used": 24600 },
  "window_7d": { "used_pct": 5.1, "tokens_used": 102000 },
  "session": {
    "input_tokens": 18000,
    "output_tokens": 4200,
    "reasoning_tokens": 2400,
    "cached_input_tokens": 1000,
    "cost_usd": 0.0421,
    "cache_hit_pct": 5.3
  },
  "tasks_completed": 7,
  "patches_applied": 12,
  "recent_tasks": ["Fix login bug", "Add dark mode", "..."],
  "model": "o4-mini"
}
```

---

## Where Codex CLI stores data

| Path | Contents |
|------|----------|
| `~/.codex/sessions/*.jsonl` | Session event streams (turn.completed, tool_call, etc.) |
| `~/.codex/sessions/*.json`  | Alternative session format |
| `~/.codex/state_5.sqlite`   | Persistent state (best-effort read) |

The bridge reads all `.jsonl` and `.json` files in `~/.codex/sessions/`,
sorted by modification time newest-first. It aggregates today's token usage
and the rolling 5-hour window.

---

## Pricing (o4-mini — as of Nov 2025)

| Token type | Price |
|------------|-------|
| Input | \$1.10 / MTok |
| Output | \$4.40 / MTok |
| Reasoning | \$1.10 / MTok |
| Cached Input | \$0.275 / MTok |

Update `PRICE_*` constants in `monitor.py` if prices change.

---

## Run as a background service (macOS)

```bash
# Simple background run
nohup python3 bridge/monitor.py > /tmp/codex-bridge.log 2>&1 &
echo "Bridge running. PID: $!"
```

Or create a launchd plist for auto-start on login (optional).

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| CYD shows `BRIDGE OFFLINE` | Check IP, ensure bridge is running, same WiFi network |
| All zeros in stats | Codex CLI may store files in a different path; check `~/.codex/` |
| `ModuleNotFoundError: flask` | Run `pip install -r requirements.txt` |
| Port already in use | Kill existing process: `lsof -ti:5049 \| xargs kill` |
