#!/usr/bin/env python3
"""
build_binaries.py — Automated Firmware Binary Exporter for CYD Web Flasher
Author: Tushar Aggarwal (tushar2704)
Repository: https://github.com/tushar2704/Cheap-Yellow-Display-Community

Usage:
    python3 tools/build_binaries.py                  # Build all projects
    python3 tools/build_binaries.py <project_folder> # Build a specific project
"""

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
PROJECTS_DIR = ROOT_DIR / "projects"
OUTPUT_DIR = ROOT_DIR / "flasher" / "binaries"

PIO_CMD = shutil.which("pio") or "/Users/tushar/Library/Python/3.9/bin/pio"

def build_project(project_path: Path):
    print(f"\n🔨 Building: {project_path.name}...")
    result = subprocess.run([PIO_CMD, "run"], cwd=project_path, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"❌ Build failed for {project_path.name}:\n{result.stderr[-500:]}")
        return False

    pio_build_dir = project_path / ".pio" / "build"
    firmware = None
    bootloader = None
    partitions = None

    if pio_build_dir.exists():
        for env_dir in sorted(pio_build_dir.iterdir()):
            if env_dir.is_dir() and (env_dir / "firmware.bin").exists():
                firmware = env_dir / "firmware.bin"
                bootloader = env_dir / "bootloader.bin"
                partitions = env_dir / "partitions.bin"
                break

    if firmware and firmware.exists():
        prefix = project_path.name
        shutil.copy(firmware, OUTPUT_DIR / f"{prefix}.bin")
        if bootloader and bootloader.exists():
            shutil.copy(bootloader, OUTPUT_DIR / f"{prefix}.bootloader.bin")
        if partitions and partitions.exists():
            shutil.copy(partitions, OUTPUT_DIR / f"{prefix}.partitions.bin")
        print(f"✅ Exported: {prefix}.bin to flasher/binaries/")
        return True
    else:
        print(f"⚠️ No firmware.bin found in {pio_build_dir}")
        return False

def main():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    if len(sys.argv) > 1:
        target = sys.argv[1]
        matches = list(PROJECTS_DIR.glob(f"**/{target}"))
        if not matches:
            print(f"❌ Could not find project matching '{target}' in {PROJECTS_DIR}")
            sys.exit(1)
        build_project(matches[0])
        return

    success = 0
    total = 0

    for root, dirs, files in sorted(os.walk(PROJECTS_DIR)):
        if "platformio.ini" in files:
            total += 1
            if build_project(Path(root)):
                success += 1

    print(f"\n🎉 Completed: {success}/{total} project firmwares compiled successfully!")

if __name__ == "__main__":
    main()
