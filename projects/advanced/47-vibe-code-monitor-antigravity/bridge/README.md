# Antigravity Monitor Bridge

Reads your local Antigravity session data from `~/.gemini/antigravity/brain/`
and serves it as JSON for the CYD ESP32 display — **no cloud credentials needed**.

## Setup

```bash
pip3 install -r requirements.txt
python3 monitor.py
```

The bridge will print your LAN IP and the exact string to paste into `src/main.cpp`.

## Find your IP manually

| OS      | Command                      |
|---------|------------------------------|
| macOS   | `ipconfig getifaddr en0`     |
| Windows | `ipconfig` → IPv4 Address    |
| Linux   | `hostname -I`                |

Then set `BRIDGE_HOST` in `src/main.cpp` to that IP and flash your CYD.

## Endpoints

| Path                   | Description                   |
|------------------------|-------------------------------|
| `GET /api/antigravity` | Full session JSON for the CYD |
| `GET /health`          | Liveness check                |

## Example response

```json
{
  "tool": "antigravity",
  "status": "active",
  "session": {
    "conversations_today": 7,
    "tool_calls_today": 142,
    "tokens_in": 48200,
    "tokens_out": 12800,
    "subagents_spawned": 3,
    "active_minutes": 94
  },
  "streak": { "days": 12, "best": 21 },
  "model": "gemini-2.5-pro",
  "last_activity": "5m ago"
}
```

## Data source

The bridge scans every `transcript.jsonl` file under
`~/.gemini/antigravity/brain/` and counts:

- **PLANNER_RESPONSE** steps → tool calls, subagent invocations, output tokens
- **USER_INPUT** steps → input tokens
- Conversations with at least one step today → `conversations_today`

Tokens are estimated as `len(content) / 4` characters — a rough approximation.

## Streak cache (optional)

Create `~/.gemini/antigravity/antigravity_stats_cache.json` with:

```json
{ "streak_days": 12, "streak_best": 21 }
```

to persist streak data across days (the bridge cannot track historical dates on
its own without this file).
