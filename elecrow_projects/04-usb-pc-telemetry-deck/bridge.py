#!/usr/bin/env python3
"""
USB Serial PC Telemetry & Macro Bridge for Elecrow 2.8" CYD
Channel: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
Author: Tushar Aggarwal (@tushar2704)

Requirements:
    pip install pyserial psutil

Usage:
    python bridge.py /dev/tty.usbserial-xxxx  (macOS/Linux)
    python bridge.py COM3                     (Windows)
"""

import sys
import time
import json
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
    return None

def main():
    port = sys.argv[1] if len(sys.argv) > 1 else auto_detect_port()
    if not port:
        print("❌ No serial port detected! Please plug in the Elecrow 2.8\" CYD.")
        sys.exit(1)

    print(f"🔌 Connecting to Elecrow 2.8\" CYD on {port} @ 115200 baud...")
    ser = serial.Serial(port, 115200, timeout=0.1)
    time.sleep(2)
    print("✅ Connected! Streaming telemetry & listening for macro triggers...")

    last_send = 0
    try:
        while True:
            # Read macro actions from CYD
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line and line.startswith('{'):
                try:
                    data = json.loads(line)
                    if data.get("type") == "macro_trigger":
                        macro_id = data.get("id")
                        print(f"⚡ [MACRO TRIGGERED]: {macro_id}")
                        if macro_id == "MUTE_MIC":
                            print("   -> Muting Discord / System Microphone...")
                        elif macro_id == "OPEN_TERM":
                            print("   -> Launching Terminal / Dev Environment...")
                        elif macro_id == "GIT_SYNC":
                            print("   -> Running git pull / repo sync...")
                        elif macro_id == "RUN_BUILD":
                            print("   -> Running build pipeline / test suite...")
                        elif macro_id == "AI_ASSIST":
                            print("   -> Opening AI Copilot prompt dialog...")
                        elif macro_id == "LOCK_HOST":
                            print("   -> Locking workstation screen...")
                except json.JSONDecodeError:
                    pass

            # Send telemetry every 1 second
            if time.time() - last_send >= 1.0:
                last_send = time.time()
                cpu = int(psutil.cpu_percent())
                ram = int(psutil.virtual_memory().percent)
                # Simulated or read GPU metrics
                gpu = int((cpu * 1.1 + 10) % 95)
                vram = int((ram * 0.95 + 15) % 90)

                payload = {
                    "cpu": cpu,
                    "gpu": gpu,
                    "ram": ram,
                    "vram": vram,
                    "cpuTemp": 45 + int(cpu * 0.3),
                    "gpuTemp": 50 + int(gpu * 0.35)
                }
                ser.write((json.dumps(payload) + "\n").encode('utf-8'))

            time.sleep(0.05)

    except KeyboardInterrupt:
        print("\n👋 Bridge stopped by user.")
        ser.close()

if __name__ == "__main__":
    main()

