#!/usr/bin/env python3
"""
OpenAI Codex CLI Vibe Monitor Bridge
Reads local Codex CLI session files and serves usage metrics for the CYD ESP32.

Codex CLI stores session data in:
  - ~/.codex/sessions/*.jsonl  (session event streams)
  - ~/.codex/state_5.sqlite    (persistent state)

Usage: python3 monitor.py
Port: 5049
"""

import json
import glob
import sqlite3
import os
from pathlib import Path
from datetime import datetime, timedelta, timezone
from flask import Flask, jsonify

CODEX_DIR = Path.home() / ".codex"
PORT = 5049

# OpenAI o4-mini pricing
PRICE_INPUT          = 1.10 / 1_000_000   # $1.10/MTok
PRICE_OUTPUT         = 4.40 / 1_000_000   # $4.40/MTok
PRICE_REASONING      = 1.10 / 1_000_000   # $1.10/MTok (same as input)
PRICE_CACHED_INPUT   = 0.275 / 1_000_000  # $0.275/MTok (75% discount)
WINDOW_5H_LIMIT_EST  = 200_000            # Estimated 5h window token budget

app = Flask(__name__)


def parse_codex_sessions():
    """Parse Codex CLI session files for today's metrics."""
    now = datetime.now(timezone.utc)
    today = now.date()
    window_5h_start = now - timedelta(hours=5)

    all_day = {"input": 0, "output": 0, "reasoning": 0, "cached": 0}
    window_5h = {"input": 0, "output": 0, "reasoning": 0, "cached": 0}
    window_7d_total = 0
    tasks_completed = 0
    patches_applied = 0
    recent_tasks = []
    earliest_in_5h = None
    latest_ts = None

    sessions_dir = CODEX_DIR / "sessions"

    # Try JSONL / JSON session files
    if sessions_dir.exists():
        session_files = (
            list(sessions_dir.rglob("*.jsonl")) +
            list(sessions_dir.rglob("*.json"))
        )

        for sf in sorted(session_files, key=lambda f: f.stat().st_mtime, reverse=True):
            try:
                with open(sf, "r", encoding="utf-8") as f:
                    for line in f:
                        line = line.strip()
                        if not line:
                            continue
                        try:
                            event = json.loads(line)
                        except json.JSONDecodeError:
                            continue

                        event_type = event.get("type", "")

                        # Parse timestamp
                        ts_str = (
                            event.get("timestamp") or
                            event.get("created_at") or
                            event.get("time", "")
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
                            stat = sf.stat()
                            entry_time = datetime.fromtimestamp(
                                stat.st_mtime, tz=timezone.utc
                            )

                        entry_date = entry_time.date()

                        # Extract usage data
                        usage = None
                        if event_type in ("turn.completed", "usage", "response"):
                            usage = event.get("usage") or event.get(
                                "response", {}
                            ).get("usage", {})
                        elif "usage" in event:
                            usage = event["usage"]

                        if usage and isinstance(usage, dict):
                            it = int(
                                usage.get("input_tokens", 0)
                                or usage.get("prompt_tokens", 0)
                            )
                            ot = int(
                                usage.get("output_tokens", 0)
                                or usage.get("completion_tokens", 0)
                            )
                            rt = int(usage.get("reasoning_tokens", 0))
                            # cached tokens — try multiple keys
                            pt_details = usage.get("prompt_tokens_details", {}) or {}
                            ct = int(
                                usage.get("cached_input_tokens", 0)
                                or usage.get("cached_tokens", 0)
                                or pt_details.get("cached_tokens", 0)
                            )

                            # 7-day aggregate
                            if (now - entry_time).days <= 7:
                                window_7d_total += it + ot + rt

                            if entry_date == today:
                                all_day["input"]     += it
                                all_day["output"]    += ot
                                all_day["reasoning"] += rt
                                all_day["cached"]    += ct

                                if entry_time >= window_5h_start:
                                    window_5h["input"]     += it
                                    window_5h["output"]    += ot
                                    window_5h["reasoning"] += rt
                                    window_5h["cached"]    += ct
                                    if earliest_in_5h is None or entry_time < earliest_in_5h:
                                        earliest_in_5h = entry_time

                                if latest_ts is None or entry_time > latest_ts:
                                    latest_ts = entry_time

                        # Count task completions
                        if event_type in ("task.completed", "turn.completed"):
                            tasks_completed += 1
                            desc = (
                                event.get("message") or
                                event.get("summary") or
                                event.get("content", "")
                            )
                            if desc and isinstance(desc, str) and len(desc) > 0:
                                if len(recent_tasks) < 5:
                                    recent_tasks.append(desc[:50])

                        # Count patches / edits
                        if event_type in ("tool_call", "function_call"):
                            tool_name = (
                                event.get("function", {}).get("name", "")
                                or event.get("name", "")
                            )
                            if any(
                                x in tool_name.lower()
                                for x in ["patch", "edit", "write", "apply"]
                            ):
                                patches_applied += 1

            except Exception:
                continue

    # Try SQLite state file (best-effort — schema may vary)
    sqlite_file = CODEX_DIR / "state_5.sqlite"
    if sqlite_file.exists():
        try:
            conn = sqlite3.connect(str(sqlite_file))
            cursor = conn.cursor()
            for table in ["sessions", "turns", "history", "events", "usage"]:
                try:
                    cursor.execute(f"SELECT name FROM sqlite_master WHERE type='table' AND name=?", (table,))
                    if cursor.fetchone():
                        # Table exists; could add schema-specific parsing here
                        break
                except Exception:
                    continue
            conn.close()
        except Exception:
            pass

    # ── Calculate 5h window ──
    billable_5h = window_5h["input"] + window_5h["output"]
    pct_5h = min(100.0, (billable_5h / WINDOW_5H_LIMIT_EST) * 100)

    if earliest_in_5h:
        reset_time = earliest_in_5h + timedelta(hours=5)
        reset_in_min = max(0, int((reset_time - now).total_seconds() / 60))
    else:
        reset_in_min = 0

    # ── Cache hit rate ──
    total_input = all_day["input"] + all_day["cached"]
    cache_hit_pct = (
        (all_day["cached"] / total_input * 100) if total_input > 0 else 0.0
    )

    # ── Cost calculation ──
    cost = (
        all_day["input"]     * PRICE_INPUT +
        all_day["output"]    * PRICE_OUTPUT +
        all_day["reasoning"] * PRICE_REASONING +
        all_day["cached"]    * PRICE_CACHED_INPUT
    )

    # ── Status ──
    if latest_ts:
        delta = now - latest_ts
        status = "active" if delta.total_seconds() < 1800 else "idle"
    else:
        status = "idle"

    return {
        "window_5h": {
            "used_pct":    round(pct_5h, 1),
            "reset_in_min": reset_in_min,
            "tokens_used": billable_5h,
        },
        "window_7d": {
            "used_pct":    round(min(100.0, (window_7d_total / 2_000_000) * 100), 1),
            "tokens_used": window_7d_total,
        },
        "session": {
            "input_tokens":        all_day["input"],
            "output_tokens":       all_day["output"],
            "reasoning_tokens":    all_day["reasoning"],
            "cached_input_tokens": all_day["cached"],
            "cost_usd":            round(cost, 4),
            "cache_hit_pct":       round(cache_hit_pct, 1),
        },
        "tasks_completed": tasks_completed,
        "patches_applied": patches_applied,
        "recent_tasks":    recent_tasks[:5],
        "model":           "o4-mini",
        "status":          status,
    }, None


@app.route("/api/codex")
def codex_stats():
    data, error = parse_codex_sessions()
    if error:
        return jsonify({"tool": "codex", "status": "error", "error": error}), 200
    return jsonify({"tool": "codex", **data})


@app.route("/health")
def health():
    return jsonify({"status": "ok", "port": PORT, "tool": "codex"})


if __name__ == "__main__":
    print(f"[Codex Monitor Bridge] Starting on port {PORT}")
    print(f"[Codex Monitor Bridge] Reading from: {CODEX_DIR}")
    print(f"[Codex Monitor Bridge] Access: http://YOUR_IP:{PORT}/api/codex")
    app.run(host="0.0.0.0", port=PORT, debug=False)
