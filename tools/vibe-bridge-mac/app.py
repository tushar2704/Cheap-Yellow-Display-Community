#!/usr/bin/env python3
"""
CYD Vibe Bridge Hub
macOS Desktop GUI application to run and monitor local bridge daemons
for Antigravity (5047), Claude Code (5048), and OpenAI Codex (5049).

Author: Cheap Yellow Display Community
Tutorial: https://youtube.com/@IamNotIntoTech
Enclosure: https://youtube.com/@Makerrdesigns
"""

import sys
import os
import json
import socket
import threading
import time
from pathlib import Path
from datetime import datetime, timezone, timedelta
import tkinter as tk
from tkinter import ttk, messagebox
import logging

# Ensure packages can be found if launched from app bundle
sys.path.insert(0, os.path.expanduser("~/Library/Python/3.9/lib/python/site-packages"))

from flask import Flask, jsonify, request
from werkzeug.serving import make_server

# Suppress default noisy werkzeug logs in terminal
log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)


# -------------------------------------------------------------
# Utility: Detect Local LAN IP Address
# -------------------------------------------------------------
def get_local_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(('8.8.8.8', 80))
        ip = s.getsockname()[0]
    except Exception:
        ip = '127.0.0.1'
    finally:
        s.close()
    return ip


# -------------------------------------------------------------
# Data Extractors (Shared logic with projects 47, 48, 49)
# -------------------------------------------------------------
def get_antigravity_data():
    agy_dir = Path.home() / ".gemini" / "antigravity"
    brain_dir = agy_dir / "brain"
    today = datetime.now(timezone.utc).date()
    conversations = 0
    tool_calls = 0
    tokens_in = 0
    tokens_out = 0
    subagents = 0
    last_act = "unknown"

    if brain_dir.exists():
        for tf in brain_dir.rglob("transcript.jsonl"):
            has_today = False
            try:
                with open(tf, 'r', encoding='utf-8') as f:
                    for line in f:
                        line = line.strip()
                        if not line:
                            continue
                        try:
                            step = json.loads(line)
                        except Exception:
                            continue
                        ts = step.get("created_at", "")
                        if ts and today.isoformat() in ts:
                            has_today = True
                            st = step.get("type", "")
                            if st == "PLANNER_RESPONSE":
                                tc = step.get("tool_calls", [])
                                tool_calls += len(tc)
                                for c in tc:
                                    if isinstance(c, dict) and "invoke_subagent" in str(c):
                                        subagents += 1
                                tokens_out += max(1, len(step.get("content", "")) // 4)
                            elif st == "USER_INPUT":
                                tokens_in += max(1, len(step.get("content", "")) // 4)
                if has_today:
                    conversations += 1
            except Exception:
                continue

    status = "active" if conversations > 0 else "idle"
    return {
        "tool": "antigravity",
        "status": status,
        "session": {
            "conversations_today": conversations,
            "tool_calls_today": tool_calls,
            "tokens_in": tokens_in,
            "tokens_out": tokens_out,
            "subagents_spawned": subagents,
            "active_minutes": max(1, min(conversations * 15, 480)) if conversations else 0
        },
        "streak": {"days": 1 if conversations else 0, "best": 1},
        "model": "gemini-2.5-pro",
        "last_activity": "just now" if conversations else "idle"
    }


def get_claude_data():
    claude_dir = Path.home() / ".claude" / "projects"
    now = datetime.now(timezone.utc)
    today = now.date()
    window_5h_start = now - timedelta(hours=5)

    all_day = {"input": 0, "output": 0, "cache_w": 0, "cache_r": 0}
    window_5h = {"input": 0, "output": 0}
    earliest_in_5h = None
    files_edited = 0
    commands_run = 0

    if claude_dir.exists():
        for jf in claude_dir.rglob("*.jsonl"):
            try:
                with open(jf, 'r', encoding='utf-8') as f:
                    for line in f:
                        line = line.strip()
                        if not line:
                            continue
                        try:
                            e = json.loads(line)
                        except Exception:
                            continue
                        usage = e.get("usage") or e.get("message", {}).get("usage", {})
                        if not usage:
                            continue

                        it = int(usage.get("input_tokens", 0))
                        ot = int(usage.get("output_tokens", 0))
                        cw = int(usage.get("cache_creation_input_tokens", 0))
                        cr = int(usage.get("cache_read_input_tokens", 0))

                        ts_str = e.get("timestamp") or e.get("created_at") or ""
                        e_time = now
                        if ts_str:
                            try:
                                e_time = datetime.fromisoformat(ts_str.replace('Z', '+00:00'))
                            except Exception:
                                pass

                        if e_time.date() == today:
                            all_day["input"] += it
                            all_day["output"] += ot
                            all_day["cache_w"] += cw
                            all_day["cache_r"] += cr

                            if e_time >= window_5h_start:
                                window_5h["input"] += it
                                window_5h["output"] += ot
                                if earliest_in_5h is None or e_time < earliest_in_5h:
                                    earliest_in_5h = e_time

                        for tu in e.get("tool_use", []):
                            n = tu.get("name", "").lower() if isinstance(tu, dict) else ""
                            if "edit" in n or "write" in n:
                                files_edited += 1
                            elif "bash" in n:
                                commands_run += 1
            except Exception:
                continue

    billable_5h = window_5h["input"] + window_5h["output"]
    pct_5h = min(100.0, (billable_5h / 200000.0) * 100)
    reset_min = int(max(0, ((earliest_in_5h + timedelta(hours=5)) - now).total_seconds()) / 60) if earliest_in_5h else 0
    cost = (all_day["input"] * 3.0 + all_day["output"] * 15.0 + all_day["cache_w"] * 3.75 + all_day["cache_r"] * 0.3) / 1000000.0

    return {
        "tool": "claude",
        "status": "active" if billable_5h > 0 else "idle",
        "window_5h": {
            "used_pct": round(pct_5h, 1),
            "reset_in_min": reset_min,
            "tokens_used": billable_5h,
            "tokens_limit": 200000
        },
        "window_7d": {"used_pct": round(pct_5h * 0.6, 1), "tokens_used": billable_5h * 2},
        "session": {
            "input_tokens": all_day["input"],
            "output_tokens": all_day["output"],
            "cache_read_tokens": all_day["cache_r"],
            "cache_write_tokens": all_day["cache_w"],
            "cost_usd": round(cost, 4)
        },
        "files_edited": files_edited,
        "commands_run": commands_run
    }


def get_codex_data():
    codex_dir = Path.home() / ".codex"
    sessions_dir = codex_dir / "sessions"
    now = datetime.now(timezone.utc)
    today = now.date()
    window_5h_start = now - timedelta(hours=5)

    all_day = {"input": 0, "output": 0, "reasoning": 0, "cached": 0}
    window_5h = {"input": 0, "output": 0, "reasoning": 0}
    tasks = []
    earliest_in_5h = None

    if sessions_dir.exists():
        for sf in sessions_dir.rglob("*.jsonl"):
            try:
                with open(sf, 'r', encoding='utf-8') as f:
                    for line in f:
                        line = line.strip()
                        if not line:
                            continue
                        try:
                            ev = json.loads(line)
                        except Exception:
                            continue
                        usage = ev.get("usage") or ev.get("response", {}).get("usage", {})
                        if not usage:
                            continue

                        it = int(usage.get("input_tokens", 0) or usage.get("prompt_tokens", 0))
                        ot = int(usage.get("output_tokens", 0) or usage.get("completion_tokens", 0))
                        rt = int(usage.get("reasoning_tokens", 0))
                        ct = int(usage.get("cached_input_tokens", 0))

                        ts_str = ev.get("timestamp") or ev.get("created_at") or ""
                        e_time = now
                        if ts_str:
                            try:
                                e_time = datetime.fromisoformat(ts_str.replace('Z', '+00:00'))
                            except Exception:
                                pass

                        if e_time.date() == today:
                            all_day["input"] += it
                            all_day["output"] += ot
                            all_day["reasoning"] += rt
                            all_day["cached"] += ct

                            if e_time >= window_5h_start:
                                window_5h["input"] += it
                                window_5h["output"] += ot
                                window_5h["reasoning"] += rt
                                if earliest_in_5h is None or e_time < earliest_in_5h:
                                    earliest_in_5h = e_time

                        desc = ev.get("message") or ev.get("summary") or ""
                        if desc and len(tasks) < 5:
                            tasks.append(str(desc)[:50])
            except Exception:
                continue

    billable_5h = window_5h["input"] + window_5h["output"]
    pct_5h = min(100.0, (billable_5h / 200000.0) * 100)
    reset_min = int(max(0, ((earliest_in_5h + timedelta(hours=5)) - now).total_seconds()) / 60) if earliest_in_5h else 0
    tot_in = all_day["input"] + all_day["cached"]
    hit_pct = (all_day["cached"] / tot_in * 100) if tot_in > 0 else 0.0
    cost = (all_day["input"] * 1.1 + all_day["output"] * 4.4 + all_day["reasoning"] * 1.1 + all_day["cached"] * 0.275) / 1000000.0

    return {
        "tool": "codex",
        "status": "active" if billable_5h > 0 else "idle",
        "window_5h": {
            "used_pct": round(pct_5h, 1),
            "reset_in_min": reset_min,
            "tokens_used": billable_5h
        },
        "window_7d": {"used_pct": round(pct_5h * 0.5, 1), "tokens_used": billable_5h * 2},
        "session": {
            "input_tokens": all_day["input"],
            "output_tokens": all_day["output"],
            "reasoning_tokens": all_day["reasoning"],
            "cached_input_tokens": all_day["cached"],
            "cost_usd": round(cost, 4),
            "cache_hit_pct": round(hit_pct, 1)
        },
        "tasks_completed": len(tasks),
        "patches_applied": len(tasks) // 2,
        "recent_tasks": tasks[:5],
        "model": "o4-mini"
    }


# -------------------------------------------------------------
# Background Bridge Server Thread
# -------------------------------------------------------------
class BridgeServer:
    def __init__(self, name, port, data_fn, log_callback):
        self.name = name
        self.port = port
        self.data_fn = data_fn
        self.log_callback = log_callback
        self.server = None
        self.thread = None
        self.is_running = False

    def start(self):
        if self.is_running:
            return

        app = Flask(self.name)

        @app.route(f"/api/{self.name}")
        def endpoint():
            ip = request.remote_addr
            data = self.data_fn()
            self.log_callback(f"[{datetime.now().strftime('%H:%M:%S')}] {self.name.upper()} request from {ip} -> 200 OK")
            return jsonify(data)

        @app.route("/health")
        def health():
            return jsonify({"status": "ok", "service": self.name, "port": self.port})

        try:
            self.server = make_server("0.0.0.0", self.port, app)
            self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
            self.thread.start()
            self.is_running = True
            self.log_callback(f"[{datetime.now().strftime('%H:%M:%S')}] 🟢 Started {self.name} on port {self.port}")
        except Exception as e:
            self.log_callback(f"[{datetime.now().strftime('%H:%M:%S')}] ❌ Failed to start {self.name}: {e}")
            self.is_running = False

    def stop(self):
        if not self.is_running:
            return
        try:
            if self.server:
                self.server.shutdown()
            self.is_running = False
            self.log_callback(f"[{datetime.now().strftime('%H:%M:%S')}] ⚪ Stopped {self.name} (port {self.port})")
        except Exception as e:
            self.log_callback(f"[{datetime.now().strftime('%H:%M:%S')}] Error stopping {self.name}: {e}")


# -------------------------------------------------------------
# Main Tkinter Desktop Application
# -------------------------------------------------------------
class VibeBridgeApp(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("CYD Vibe Bridge Hub")
        self.geometry("640x720")
        self.minsize(580, 640)
        self.configure(bg="#12151f")

        # Local IP
        self.local_ip = get_local_ip()

        # Server instances
        self.servers = {
            "antigravity": BridgeServer("antigravity", 5047, get_antigravity_data, self.log_message),
            "claude": BridgeServer("claude", 5048, get_claude_data, self.log_message),
            "codex": BridgeServer("codex", 5049, get_codex_data, self.log_message),
        }

        self.status_widgets = {}

        self.build_ui()

        # Handle clean exit
        self.protocol("WM_DELETE_WINDOW", self.on_close)

    def log_message(self, msg):
        self.after(0, self._append_log, msg)

    def _append_log(self, msg):
        self.log_text.configure(state='normal')
        self.log_text.insert(tk.END, msg + "\n")
        self.log_text.see(tk.END)
        self.log_text.configure(state='disabled')

    def copy_ip_to_clipboard(self):
        self.clipboard_clear()
        self.clipboard_append(self.local_ip)
        self.update()
        messagebox.showinfo("Copied!", f"IP {self.local_ip} copied to clipboard!\nPaste it into BRIDGE_HOST in your CYD's main.cpp.")

    def refresh_ip(self):
        self.local_ip = get_local_ip()
        self.ip_value_label.config(text=self.local_ip)
        self.log_message(f"[{datetime.now().strftime('%H:%M:%S')}] Refreshed Local IP: {self.local_ip}")

    def start_all(self):
        for s in self.servers.values():
            s.start()
        self.update_status_ui()

    def stop_all(self):
        for s in self.servers.values():
            s.stop()
        self.update_status_ui()

    def toggle_server(self, name):
        s = self.servers[name]
        if s.is_running:
            s.stop()
        else:
            s.start()
        self.update_status_ui()

    def update_status_ui(self):
        for name, s in self.servers.items():
            badge, btn, url_lbl = self.status_widgets[name]
            if s.is_running:
                badge.config(text="● RUNNING", fg="#00FF65", bg="#15281e")
                btn.config(text="Stop", bg="#4a2222", fg="#ff8888")
                url_lbl.config(text=f"http://{self.local_ip}:{s.port}/api/{name}", fg="#89dceb")
            else:
                badge.config(text="○ STOPPED", fg="#888888", bg="#1f222e")
                btn.config(text="Start", bg="#1f3b2b", fg="#88ffaa")
                url_lbl.config(text=f"Port {s.port} (inactive)", fg="#666666")

    def build_ui(self):
        # Header banner
        header = tk.Frame(self, bg="#181c2b", padx=20, pady=16)
        header.pack(fill=tk.X)

        title = tk.Label(header, text="⚡ CYD VIBE BRIDGE HUB", font=("Helvetica", 18, "bold"), fg="#ffffff", bg="#181c2b")
        title.pack(anchor=tk.W)

        subtitle = tk.Label(header, text="Desktop companion bridge for Cheap Yellow Display 2.8\" AI Monitors", font=("Helvetica", 11), fg="#8ba0b8", bg="#181c2b")
        subtitle.pack(anchor=tk.W, pady=(2, 0))

        # Main Container
        body = tk.Frame(self, bg="#12151f", padx=20, pady=14)
        body.pack(fill=tk.BOTH, expand=True)

        # IP Address Card (Prominent!)
        ip_card = tk.Frame(body, bg="#1b2030", bd=1, relief=tk.FLAT, padx=16, pady=12)
        ip_card.pack(fill=tk.X, pady=(0, 14))

        ip_top = tk.Frame(ip_card, bg="#1b2030")
        ip_top.pack(fill=tk.X)

        ip_title = tk.Label(ip_top, text="YOUR MAC LOCAL IP (Set this as BRIDGE_HOST):", font=("Helvetica", 10, "bold"), fg="#7aa2f7", bg="#1b2030")
        ip_title.pack(side=tk.LEFT)

        btn_refresh = tk.Button(ip_top, text="🔄 Refresh", command=self.refresh_ip, font=("Helvetica", 10), bg="#252c42", fg="#c0caf5", relief=tk.FLAT, padx=6, pady=2)
        btn_refresh.pack(side=tk.RIGHT)

        ip_row = tk.Frame(ip_card, bg="#1b2030")
        ip_row.pack(fill=tk.X, pady=(6, 0))

        self.ip_value_label = tk.Label(ip_row, text=self.local_ip, font=("Menlo", 20, "bold"), fg="#00f0ff", bg="#1b2030")
        self.ip_value_label.pack(side=tk.LEFT)

        btn_copy = tk.Button(ip_row, text="📋 Copy IP", command=self.copy_ip_to_clipboard, font=("Helvetica", 11, "bold"), bg="#3b4261", fg="#ffffff", relief=tk.FLAT, padx=12, pady=4)
        btn_copy.pack(side=tk.RIGHT)

        # Service Cards
        services = [
            ("antigravity", "🔵 Antigravity Monitor", 5047, "Reads ~/.gemini/antigravity/ logs", "#00e5ff"),
            ("claude", "🟠 Claude Code Monitor", 5048, "Reads ~/.claude/projects/ 5h rolling window", "#ff8833"),
            ("codex", "🟢 OpenAI Codex Monitor", 5049, "Reads ~/.codex/ reasoning tokens & tasks", "#00ff66"),
        ]

        for key, name, port, desc, accent in services:
            card = tk.Frame(body, bg="#1b2030", padx=14, pady=10, relief=tk.FLAT)
            card.pack(fill=tk.X, pady=4)

            top_row = tk.Frame(card, bg="#1b2030")
            top_row.pack(fill=tk.X)

            lbl_name = tk.Label(top_row, text=name, font=("Helvetica", 12, "bold"), fg=accent, bg="#1b2030")
            lbl_name.pack(side=tk.LEFT)

            badge = tk.Label(top_row, text="○ STOPPED", font=("Menlo", 9, "bold"), fg="#888888", bg="#1f222e", padx=8, pady=2)
            badge.pack(side=tk.RIGHT)

            mid_row = tk.Frame(card, bg="#1b2030")
            mid_row.pack(fill=tk.X, pady=(2, 4))

            lbl_desc = tk.Label(mid_row, text=desc, font=("Helvetica", 10), fg="#7a889b", bg="#1b2030")
            lbl_desc.pack(side=tk.LEFT)

            bot_row = tk.Frame(card, bg="#1b2030")
            bot_row.pack(fill=tk.X, pady=(2, 0))

            url_lbl = tk.Label(bot_row, text=f"Port {port} (inactive)", font=("Menlo", 10), fg="#666666", bg="#1b2030")
            url_lbl.pack(side=tk.LEFT)

            btn = tk.Button(bot_row, text="Start", font=("Helvetica", 10, "bold"), bg="#1f3b2b", fg="#88ffaa",
                            relief=tk.FLAT, padx=10, pady=2, command=lambda k=key: self.toggle_server(k))
            btn.pack(side=tk.RIGHT)

            self.status_widgets[key] = (badge, btn, url_lbl)

        # Global Actions Row
        actions = tk.Frame(body, bg="#12151f", pady=10)
        actions.pack(fill=tk.X)

        btn_start_all = tk.Button(actions, text="🚀 Start All Bridges", command=self.start_all,
                                  font=("Helvetica", 11, "bold"), bg="#1b4d3e", fg="#55ff99", relief=tk.FLAT, padx=14, pady=6)
        btn_start_all.pack(side=tk.LEFT, padx=(0, 8))

        btn_stop_all = tk.Button(actions, text="⏹ Stop All", command=self.stop_all,
                                 font=("Helvetica", 11), bg="#3e2323", fg="#ff9999", relief=tk.FLAT, padx=14, pady=6)
        btn_stop_all.pack(side=tk.LEFT)

        # Live Activity Console
        log_frame = tk.Frame(body, bg="#12151f")
        log_frame.pack(fill=tk.BOTH, expand=True, pady=(6, 0))

        log_lbl = tk.Label(log_frame, text="ACTIVITY LOG (CYD Requests)", font=("Helvetica", 10, "bold"), fg="#8ba0b8", bg="#12151f")
        log_lbl.pack(anchor=tk.W, pady=(0, 4))

        self.log_text = tk.Text(log_frame, bg="#0d0f17", fg="#a9b1d6", font=("Menlo", 10),
                                insertbackground="white", bd=0, padx=8, pady=6, height=7, state='disabled')
        self.log_text.pack(fill=tk.BOTH, expand=True)

        # Footer Credits
        footer = tk.Frame(self, bg="#0d0f17", padx=16, pady=8)
        footer.pack(fill=tk.X, side=tk.BOTTOM)

        foot_txt = tk.Label(footer, text="Tutorial: youtube.com/@IamNotIntoTech  ·  3D Cases: youtube.com/@Makerrdesigns",
                            font=("Helvetica", 9), fg="#556075", bg="#0d0f17")
        foot_txt.pack(side=tk.LEFT)

        # Initial log message
        self.log_message(f"[{datetime.now().strftime('%H:%M:%S')}] Hub initialized. Click 'Start All Bridges' to begin.")

    def on_close(self):
        for s in self.servers.values():
            s.stop()
        self.destroy()


if __name__ == "__main__":
    app = VibeBridgeApp()
    app.mainloop()

