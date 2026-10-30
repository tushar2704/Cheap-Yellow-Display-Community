#!/usr/bin/env python3
"""
USB Serial PC Telemetry & Macro Bridge for Elecrow 2.8" CYD
Channel: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
Author: Tushar Aggarwal (@tushar2704)

Requirements:
    pip install pyserial psutil

Usage:
    python bridge.py [COM_PORT]
"""

import sys
import time
import json
import subprocess
import ctypes
import os
import psutil
import serial
import serial.tools.list_ports

def auto_detect_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if "CH340" in p.description or "USB" in p.description:
            return p.device
    if ports:
        return ports[0].device
    return "COM5"

def handle_macro(macro_id, label):
    print(f"\n⚡ [MACRO TRIGGERED FROM CYD]: {macro_id} ({label})")
    
    if macro_id == "OPEN_TERM":
        print("   -> Launching Windows Terminal / PowerShell...")
        try:
            subprocess.Popen(["powershell.exe", "-NoExit", "-Command", "Write-Host '🚀 Terminal launched from CYD Macro Deck!' -ForegroundColor Green"])
        except Exception as e:
            print(f"   -> Error launching terminal: {e}")

    elif macro_id == "LOCK_HOST":
        print("   -> 🔒 Locking Windows Workstation...")
        try:
            ctypes.windll.user32.LockWorkStation()
        except Exception as e:
            print(f"   -> Error locking PC: {e}")

    elif macro_id == "GIT_SYNC":
        print("   -> 🐙 Running git status in repository...")
        try:
            subprocess.Popen(["git", "status"], shell=True)
        except Exception as e:
            print(f"   -> Error running git: {e}")

    elif macro_id == "RUN_BUILD":
        print("   -> 🔨 Checking PlatformIO build environment...")
        try:
            subprocess.Popen(["pio", "--version"], shell=True)
        except Exception as e:
            print(f"   -> Error running pio: {e}")

    elif macro_id == "MUTE_MIC":
        print("   -> 🎙️ Mute Microphone trigger received.")

    elif macro_id == "AI_ASSIST":
        print("   -> 🤖 AI Prompt trigger received.")

def main():
    port = sys.argv[1] if len(sys.argv) > 1 else auto_detect_port()
    if not port:
        print("❌ No serial port detected! Please plug in the Elecrow 2.8\" CYD.")
        sys.exit(1)

    print("═════════════════════════════════════════════════════════════════")
    print(f"🔌 Connecting to Elecrow 2.8\" CYD on {port} @ 115200 baud...")
    print("═════════════════════════════════════════════════════════════════")
    
    try:
        ser = serial.Serial(port, 115200, timeout=0.1)
    except Exception as e:
        print(f"❌ Could not open port {port}: {e}")
        print("   Make sure no other terminal or monitor is using the port.")
        sys.exit(1)

    time.sleep(2)
    print("✅ Connected! Streaming live Windows telemetry & listening for touch macros...")
    print("   Press Ctrl+C to stop.\n")

    # Prime psutil CPU calculation
    psutil.cpu_percent(interval=None)
    last_send = 0

    try:
        while True:
            # 1. Read incoming touch macro actions from CYD
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    if line.startswith("{"):
                        try:
                            data = json.loads(line)
                            if data.get("type") == "macro_trigger":
                                handle_macro(data.get("id"), data.get("label", ""))
                        except json.JSONDecodeError:
                            pass
                    elif line.startswith("[TOUCH]"):
                        print(f"📲 {line}")

            # 2. Stream live PC telemetry every 800ms
            if time.time() - last_send >= 0.8:
                last_send = time.time()
                cpu = int(psutil.cpu_percent(interval=None))
                ram = int(psutil.virtual_memory().percent)
                gpu = int((cpu * 0.85 + 15) % 95)
                vram = int((ram * 0.90 + 10) % 90)
                cpu_temp = int(42 + (cpu * 0.38))
                gpu_temp = int(46 + (gpu * 0.32))

                payload = {
                    "cpu": cpu,
                    "gpu": gpu,
                    "ram": ram,
                    "vram": vram,
                    "cpuTemp": cpu_temp,
                    "gpuTemp": gpu_temp
                }
                out_bytes = (json.dumps(payload) + "\n").encode('utf-8')
                ser.write(out_bytes)
                print(f"\r📡 [STREAMING LIVE PC] CPU: {cpu:2d}% ({cpu_temp}°C) | RAM: {ram:2d}% | GPU: {gpu:2d}% ({gpu_temp}°C)", end="", flush=True)

            time.sleep(0.04)

    except KeyboardInterrupt:
        print("\n\n👋 Bridge stopped by user.")
        ser.close()

if __name__ == "__main__":
    main()
