#!/usr/bin/env python3
"""
Claude Code Vibe Monitor Bridge
Reads local Claude Code session JSONL files and serves usage metrics for the CYD ESP32.

Claude Code stores session data in: ~/.claude/projects/**/*.jsonl
Each line is a conversation turn with token counts.

Usage: python3 monitor.py
Port: 5048
"""

import json
import os
from pathlib import Path
from datetime import datetime, timedelta, timezone
from flask import Flask, jsonify

CLAUDE_DIR = Path.home() / ".claude" / "projects"
PORT = 5048

# Approximate 5-hour rolling window token limit for Claude Code Pro
WINDOW_5H_LIMIT = 200_000
# Approximate 7-day cumulative limit
WINDOW_7D_LIMIT = 2_000_000

# Pricing (per token) — Claude 3.5 Sonnet
PRICE_INPUT   = 3.00  / 1_000_000
PRICE_OUTPUT  = 15.00 / 1_000_000
PRICE_CACHE_W = 3.75  / 1_000_000
PRICE_CACHE_R = 0.30  / 1_000_000

app = Flask(__name__)


def parse_claude_usage():
    """Parse all Claude Code JSONL session files for today's and 5-hour window metrics."""
    now = datetime.now(timezone.utc)
    today = now.date()
    window_5h_start = now - timedelta(hours=5)
    window_7d_start = now - timedelta(days=7)

    all_day_stats = {
        "input_tokens": 0,
        "output_tokens": 0,
        "cache_write_tokens": 0,
        "cache_read_tokens": 0,
    }
    window_5h_stats = {
        "input_tokens": 0,
        "output_tokens": 0,
        "cache_write_tokens": 0,
        "cache_read_tokens": 0,
    }
    window_7d_tokens = 0
    files_edited = 0
    commands_run = 0
    latest_ts = None
    earliest_in_5h_window = None

    if not CLAUDE_DIR.exists():
        return None, f"Claude projects dir not found at {CLAUDE_DIR}"

    jsonl_files = list(CLAUDE_DIR.rglob("*.jsonl"))

    if not jsonl_files:
        return None, f"No JSONL files found in {CLAUDE_DIR}"

    for jf in jsonl_files:
        try:
            with open(jf, "r", encoding="utf-8") as f:
                for line in f:
                    line = line.strip()
                    if not line:
                        continue
                    try:
                        entry = json.loads(line)
                    except json.JSONDecodeError:
                        continue

                    # ---- Extract timestamp ----
                    ts_str = (
                        entry.get("timestamp")
                        or entry.get("created_at")
                        or ""
                    )
                    entry_time = None
                    if ts_str:
                        try:
                            entry_time = datetime.fromisoformat(
                                ts_str.replace("Z", "+00:00")
                            )
                        except Exception:
                            pass

                    if entry_time is None:
                        # Fallback: use file mtime
                        stat = jf.stat()
                        entry_time = datetime.fromtimestamp(stat.st_mtime, tz=timezone.utc)

                    # ---- Extract usage ----
                    usage = (
                        entry.get("usage")
                        or entry.get("message", {}).get("usage", {})
                        or {}
                    )
                    if not usage:
                        content = entry.get("content") or entry.get("message", {})
                        if isinstance(content, dict):
                            usage = content.get("usage", {})

                    it = int(usage.get("input_tokens", 0))
                    ot = int(usage.get("output_tokens", 0))
                    cw = int(usage.get("cache_creation_input_tokens", 0))
                    cr = int(usage.get("cache_read_input_tokens", 0))

                    # ---- 7-day window ----
                    if entry_time >= window_7d_start:
                        window_7d_tokens += it + ot + cw + cr

                    # ---- Track latest timestamp ----
                    if latest_ts is None or entry_time > latest_ts:
                        latest_ts = entry_time

                    # ---- Today only for session stats ----
                    entry_date = entry_time.date()
                    if entry_date != today:
                        continue

                    all_day_stats["input_tokens"] += it
                    all_day_stats["output_tokens"] += ot
                    all_day_stats["cache_write_tokens"] += cw
                    all_day_stats["cache_read_tokens"] += cr

                    # ---- 5-hour window ----
                    if entry_time >= window_5h_start:
                        window_5h_stats["input_tokens"] += it
                        window_5h_stats["output_tokens"] += ot
                        window_5h_stats["cache_write_tokens"] += cw
                        window_5h_stats["cache_read_tokens"] += cr
                        if earliest_in_5h_window is None or entry_time < earliest_in_5h_window:
                            earliest_in_5h_window = entry_time

                    # ---- Tool usage counting ----
                    tool_uses = entry.get("tool_use") or []
                    if isinstance(tool_uses, list):
                        for tu in tool_uses:
                            if not isinstance(tu, dict):
                                continue
                            tool_name = tu.get("name", "").lower()
                            if any(k in tool_name for k in ("edit", "write", "create", "patch")):
                                files_edited += 1
                            elif any(k in tool_name for k in ("bash", "command", "run", "exec", "shell")):
                                commands_run += 1

        except Exception as exc:
            # Skip unreadable files silently
            continue

    # ---- 5h window calculations ----
    billable_5h = window_5h_stats["input_tokens"] + window_5h_stats["output_tokens"]
    pct_5h = min(100.0, (billable_5h / WINDOW_5H_LIMIT) * 100)

    # Time until 5h window resets (from earliest entry in window)
    if earliest_in_5h_window:
        reset_at = earliest_in_5h_window + timedelta(hours=5)
        reset_secs = max(0, (reset_at - now).total_seconds())
        reset_in_min = int(reset_secs / 60)
    else:
        reset_in_min = 0

    # ---- 7-day percentage ----
    pct_7d = min(100.0, (window_7d_tokens / WINDOW_7D_LIMIT) * 100)

    # ---- Cost for today ----
    cost = (
        all_day_stats["input_tokens"]       * PRICE_INPUT   +
        all_day_stats["output_tokens"]      * PRICE_OUTPUT  +
        all_day_stats["cache_write_tokens"] * PRICE_CACHE_W +
        all_day_stats["cache_read_tokens"]  * PRICE_CACHE_R
    )

    # ---- Status ----
    if latest_ts:
        idle_mins = int((now - latest_ts).total_seconds() / 60)
        status = "active" if idle_mins < 30 else "idle"
    else:
        status = "idle"

    return {
        "window_5h": {
            "used_pct":     round(pct_5h, 1),
            "reset_in_min": reset_in_min,
            "tokens_used":  billable_5h,
            "tokens_limit": WINDOW_5H_LIMIT,
        },
        "window_7d": {
            "used_pct":    round(pct_7d, 1),
            "tokens_used": window_7d_tokens,
        },
        "session": {
            "input_tokens":       all_day_stats["input_tokens"],
            "output_tokens":      all_day_stats["output_tokens"],
            "cache_read_tokens":  all_day_stats["cache_read_tokens"],
            "cache_write_tokens": all_day_stats["cache_write_tokens"],
            "cost_usd":           round(cost, 4),
        },
        "files_edited": files_edited,
        "commands_run": commands_run,
        "status":       status,
    }, None


# ---- Routes ----

@app.route("/api/claude")
def claude_stats():
    data, error = parse_claude_usage()
    if error:
        return jsonify({
            "tool":   "claude",
            "status": "error",
            "error":  error,
        }), 200

    return jsonify({
        "tool":   "claude",
        "status": data["status"],
        **data,
    })


@app.route("/health")
def health():
    return jsonify({"status": "ok", "port": PORT, "tool": "claude"})


@app.route("/debug")
def debug():
    """Return raw parsed data plus file list for troubleshooting."""
    files = [str(p) for p in CLAUDE_DIR.rglob("*.jsonl")] if CLAUDE_DIR.exists() else []
    data, error = parse_claude_usage()
    return jsonify({
        "claude_dir": str(CLAUDE_DIR),
        "jsonl_files_found": len(files),
        "files": files[:20],  # first 20
        "data": data,
        "error": error,
    })


if __name__ == "__main__":
    print(f"╔══════════════════════════════════════════════╗")
    print(f"║   Claude Code Vibe Monitor Bridge v1.0       ║")
    print(f"╚══════════════════════════════════════════════╝")
    print(f"  Reading from : {CLAUDE_DIR}")
    print(f"  API endpoint : http://0.0.0.0:{PORT}/api/claude")
    print(f"  Health check : http://0.0.0.0:{PORT}/health")
    print(f"  Debug view   : http://0.0.0.0:{PORT}/debug")
    print()

    # Quick sanity check on startup
    if not CLAUDE_DIR.exists():
        print(f"  [WARN] Claude projects dir not found: {CLAUDE_DIR}")
        print(f"         Make sure Claude Code has been used at least once.")
    else:
        files = list(CLAUDE_DIR.rglob("*.jsonl"))
        print(f"  [OK]  Found {len(files)} JSONL session file(s)")

    print()
    app.run(host="0.0.0.0", port=PORT, debug=False)
