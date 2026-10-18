#!/usr/bin/env python3
"""
build_binaries.py — Automated Firmware Binary Exporter for CYD Web Flasher
Author: Tushar Aggarwal (tushar2704)
Repository: https://github.com/tushar2704/Cheap-Yellow-Display-Community

Usage:
    python3 tools/build_binaries.py
"""

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
PROJECTS_DIR = ROOT_DIR / "projects"
OUTPUT_DIR = ROOT_DIR / "flasher" / "binaries"

def build_project(project_path: Path):
    print(f"\n🔨 Building: {project_path.name}...")
    result = subprocess.run(["pio", "run"], cwd=project_path, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"❌ Build failed for {project_path.name}:\n{result.stderr}")
        return False

    build_dir = project_path / ".pio" / "build" / "esp32dev"
    firmware = build_dir / "firmware.bin"
    bootloader = build_dir / "bootloader.bin"
    partitions = build_dir / "partitions.bin"

    if firmware.exists():
        prefix = project_path.name
        shutil.copy(firmware, OUTPUT_DIR / f"{prefix}.bin")
        if bootloader.exists():
            shutil.copy(bootloader, OUTPUT_DIR / f"{prefix}.bootloader.bin")
        if partitions.exists():
            shutil.copy(partitions, OUTPUT_DIR / f"{prefix}.partitions.bin")
        print(f"✅ Exported: {prefix}.bin to flasher/binaries/")
        return True
    return False

def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    success = 0
    total = 0

    for root, dirs, files in os.walk(PROJECTS_DIR):
        if "platformio.ini" in files:
            total += 1
            if build_project(Path(root)):
                success += 1

    print(f"\n🎉 Completed: {success}/{total} project firmwares compiled successfully!")

if __name__ == "__main__":
    main()
