#!/usr/bin/env python3
"""
Antigravity Vibe Monitor Bridge
================================
Reads local Antigravity session data from ~/.gemini/antigravity/brain/
and serves it as JSON for the CYD ESP32 display.

Usage:
    pip3 install -r requirements.txt
    python3 monitor.py

Then configure BRIDGE_HOST in src/main.cpp to this machine's IP.
Port: 5047
"""

import json
import time
import os
from pathlib import Path
from datetime import datetime, timezone
from flask import Flask, jsonify

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

AGY_DIR = Path.home() / ".gemini" / "antigravity"
PORT    = 5047

app = Flask(__name__)

# ---------------------------------------------------------------------------
# Stats collection
# ---------------------------------------------------------------------------

def count_today_stats():
    """
    Parse every transcript.jsonl beneath ~/.gemini/antigravity/brain/
    and aggregate today's session statistics.

    Returns (stats_dict, error_string_or_None).
    """
    today = datetime.now(timezone.utc).date()

    conversations      = 0
    tool_calls         = 0
    tokens_in          = 0
    tokens_out         = 0
    subagents          = 0
    last_activity_time = None

    brain_dir = AGY_DIR / "brain"
    if not brain_dir.exists():
        return {}, f"No Antigravity data found at {brain_dir}"

    transcript_files = list(brain_dir.rglob("transcript.jsonl"))
    if not transcript_files:
        return {}, f"No transcript files found under {brain_dir}"

    for tf in transcript_files:
        conversation_has_today = False
        try:
            with open(tf, "r", encoding="utf-8", errors="replace") as fh:
                for raw in fh:
                    raw = raw.strip()
                    if not raw:
                        continue
                    try:
                        step = json.loads(raw)
                    except json.JSONDecodeError:
                        continue

                    created_at_str = step.get("created_at", "")
                    if not created_at_str:
                        continue

                    # Parse ISO-8601 timestamp (handle trailing Z)
                    try:
                        created_at = datetime.fromisoformat(
                            created_at_str.replace("Z", "+00:00")
                        )
                        step_date = created_at.astimezone(timezone.utc).date()
                    except Exception:
                        continue

                    if step_date != today:
                        continue

                    # ---- This step belongs to today ----
                    conversation_has_today = True

                    if last_activity_time is None or created_at > last_activity_time:
                        last_activity_time = created_at

                    step_type = step.get("type", "")

                    if step_type == "PLANNER_RESPONSE":
                        # Count tool calls
                        tc_list = step.get("tool_calls", [])
                        tool_calls += len(tc_list)

                        # Count subagent invocations
                        for tc in tc_list:
                            if isinstance(tc, dict):
                                fn = tc.get("function", {})
                                if isinstance(fn, dict):
                                    if "invoke_subagent" in fn.get("name", ""):
                                        subagents += 1

                        # Rough token estimate from content length
                        content = step.get("content", "")
                        if content:
                            tokens_out += max(1, len(content) // 4)

                    elif step_type == "USER_INPUT":
                        content = step.get("content", "")
                        if content:
                            tokens_in += max(1, len(content) // 4)

        except Exception:
            # Skip unreadable files
            continue

        if conversation_has_today:
            conversations += 1

    # ---- Last-activity string ----
    if last_activity_time:
        delta = datetime.now(timezone.utc) - last_activity_time
        mins  = int(delta.total_seconds() / 60)
        if mins < 1:
            last_activity = "just now"
        elif mins < 60:
            last_activity = f"{mins}m ago"
        else:
            last_activity = f"{mins // 60}h {mins % 60}m ago"
    else:
        last_activity = "unknown"

    # ---- Active minutes (heuristic) ----
    active_minutes = max(0, min(len(transcript_files) * 15, 480))

    # ---- Streak (from optional cache file, else derive) ----
    streak_days = 1 if conversations > 0 else 0
    streak_best = streak_days

    stats_cache = AGY_DIR / "antigravity_stats_cache.json"
    if stats_cache.exists():
        try:
            with open(stats_cache) as sc:
                cache = json.load(sc)
            streak_days = cache.get("streak_days", streak_days)
            streak_best = cache.get("streak_best", max(streak_days, streak_best))
        except Exception:
            pass

    return {
        "conversations_today": conversations,
        "tool_calls_today":    tool_calls,
        "tokens_in":           tokens_in,
        "tokens_out":          tokens_out,
        "subagents_spawned":   subagents,
        "active_minutes":      active_minutes,
        "streak_days":         streak_days,
        "streak_best":         streak_best,
        "last_activity":       last_activity,
    }, None


# ---------------------------------------------------------------------------
# Routes
# ---------------------------------------------------------------------------

@app.route("/api/antigravity")
def antigravity_stats():
    stats, error = count_today_stats()

    if error:
        return jsonify({
            "tool":          "antigravity",
            "status":        "error",
            "error":         error,
            "session":       {
                "conversations_today": 0,
                "tool_calls_today":    0,
                "tokens_in":           0,
                "tokens_out":          0,
                "subagents_spawned":   0,
                "active_minutes":      0,
            },
            "streak":        {"days": 0, "best": 0},
            "model":         "unknown",
            "last_activity": "unknown",
        }), 200  # Always 200 so the ESP32 can display the error state

    status = "active" if stats.get("conversations_today", 0) > 0 else "idle"

    return jsonify({
        "tool":   "antigravity",
        "status": status,
        "session": {
            "conversations_today": stats["conversations_today"],
            "tool_calls_today":    stats["tool_calls_today"],
            "tokens_in":           stats["tokens_in"],
            "tokens_out":          stats["tokens_out"],
            "subagents_spawned":   stats["subagents_spawned"],
            "active_minutes":      stats["active_minutes"],
        },
        "streak": {
            "days": stats["streak_days"],
            "best": stats["streak_best"],
        },
        "model":         "gemini-2.5-pro",
        "last_activity": stats["last_activity"],
    })


@app.route("/health")
def health():
    return jsonify({
        "status": "ok",
        "port":   PORT,
        "tool":   "antigravity",
        "agy_dir": str(AGY_DIR),
    })


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import socket

    # Determine LAN IP for display
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
    except Exception:
        local_ip = "127.0.0.1"

    print("=" * 60)
    print(" Antigravity Vibe Monitor Bridge")
    print("=" * 60)
    print(f" Reading from : {AGY_DIR}")
    print(f" Listening on : http://0.0.0.0:{PORT}")
    print(f" Your LAN IP  : {local_ip}")
    print(f" ESP32 endpoint: http://{local_ip}:{PORT}/api/antigravity")
    print()
    print(f" Set BRIDGE_HOST in src/main.cpp:")
    print(f'   const char* BRIDGE_HOST = "{local_ip}";')
    print("=" * 60)

    app.run(host="0.0.0.0", port=PORT, debug=False)
