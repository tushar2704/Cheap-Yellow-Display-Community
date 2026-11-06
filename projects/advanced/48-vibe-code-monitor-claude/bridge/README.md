# Bridge — Claude Code Vibe Monitor

This Python bridge reads Claude Code's local JSONL session files and exposes a simple HTTP JSON API for the CYD ESP32 display.

---

## Prerequisites

- **Python 3.8+** on the same computer where you use Claude Code
- Claude Code must have been used at least once (creates `~/.claude/projects/`)

---

## Setup

### 1. Install dependencies

```bash
cd bridge/
pip3 install -r requirements.txt
```

Or with a virtual environment (recommended):

```bash
cd bridge/
python3 -m venv venv
source venv/bin/activate      # Windows: venv\Scripts\activate
pip install -r requirements.txt
```

### 2. Run the bridge

```bash
python3 monitor.py
```

You should see:

```
╔══════════════════════════════════════════════╗
║   Claude Code Vibe Monitor Bridge v1.0       ║
╚══════════════════════════════════════════════╝
  Reading from : /Users/yourname/.claude/projects
  API endpoint : http://0.0.0.0:5048/api/claude
  ...
  [OK]  Found 23 JSONL session file(s)
```

### 3. Find your Mac/PC's local IP address

**macOS:**
```bash
ipconfig getifaddr en0        # WiFi
# or
ipconfig getifaddr en1
```

**Windows:**
```
ipconfig
# Look for "IPv4 Address" under your WiFi adapter
```

**Linux:**
```bash
ip addr show | grep "inet "
# or
hostname -I
```

Your IP will look like `192.168.1.xxx` or `10.0.0.xxx`.

---

## Configure the ESP32

Open `src/main.cpp` and update the config section at the top:

```cpp
// ===== USER CONFIGURATION =====
const char* WIFI_SSID     = "YourNetworkName";
const char* WIFI_PASSWORD = "YourPassword";
const char* BRIDGE_HOST   = "192.168.1.42";  // ← your Mac/PC IP here
const int   BRIDGE_PORT   = 5048;
// ==============================
```

Then flash with PlatformIO:

```bash
pio run --target upload
pio device monitor
```

---

## Verify it's working

Test the bridge from a browser or terminal **before** flashing:

```bash
# Health check
curl http://localhost:5048/health

# Full data
curl http://localhost:5048/api/claude | python3 -m json.tool

# Debug (shows file list)
curl http://localhost:5048/debug | python3 -m json.tool
```

Expected response from `/api/claude`:

```json
{
  "tool": "claude",
  "status": "active",
  "window_5h": {
    "used_pct": 23.4,
    "reset_in_min": 187,
    "tokens_used": 46800,
    "tokens_limit": 200000
  },
  "window_7d": {
    "used_pct": 11.2,
    "tokens_used": 224000
  },
  "session": {
    "input_tokens": 38200,
    "output_tokens": 12400,
    "cache_read_tokens": 95000,
    "cache_write_tokens": 8000,
    "cost_usd": 0.3842
  },
  "files_edited": 7,
  "commands_run": 14
}
```

---

## Run bridge on startup (optional)

### macOS — launchd plist

Create `~/Library/LaunchAgents/com.claude.vibe.plist`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
  "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key>
  <string>com.claude.vibe</string>
  <key>ProgramArguments</key>
  <array>
    <string>/usr/bin/python3</string>
    <string>/path/to/bridge/monitor.py</string>
  </array>
  <key>RunAtLoad</key>
  <true/>
  <key>KeepAlive</key>
  <true/>
  <key>StandardOutPath</key>
  <string>/tmp/claude-vibe.log</string>
  <key>StandardErrorPath</key>
  <string>/tmp/claude-vibe-err.log</string>
</dict>
</plist>
```

Then:
```bash
launchctl load ~/Library/LaunchAgents/com.claude.vibe.plist
```

### Windows — Task Scheduler

Use Task Scheduler to run `python bridge\monitor.py` at login with "Start in" set to the project directory.

---

## How it reads Claude Code data

Claude Code stores all session data as JSONL (JSON Lines) files in:

```
~/.claude/projects/<project-hash>/<session-id>.jsonl
```

Each line in a file is one conversation turn. The bridge:

1. **Recursively scans** `~/.claude/projects/**/*.jsonl`
2. **Parses each entry** for `usage` fields (`input_tokens`, `output_tokens`, `cache_creation_input_tokens`, `cache_read_input_tokens`)
3. **Buckets entries** by timestamp into:
   - **5-hour rolling window** (the Claude Code Pro rate-limit window)
   - **7-day cumulative**
   - **Today's session total**
4. **Calculates cost** using Claude 3.5 Sonnet pricing
5. **Counts tool uses** (file edits, bash commands) from `tool_use` arrays
6. **Serves everything** as JSON on port 5048

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| `BRIDGE OFFLINE` on display | Check IP in `main.cpp`, ensure bridge is running, check firewall |
| `0 JSONL files found` | Use Claude Code first, or check `~/.claude/projects/` exists |
| All zeros on display | JSONL format may differ — check `/debug` endpoint for raw file list |
| Wrong timezone on clock | NTP uses UTC; display shows UTC time |
| Rate limit not matching | The 5h window is approximate; Claude doesn't expose the exact limit via API |
