# Project 01: DevOps Command Vault

**Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
**Target Hardware**: Elecrow ESP32-2432S028R (2.8" TFT ILI9341 320x240 + XPT2046 Resistive Touch)

---

## Overview

The **DevOps Command Vault** turns the 2.8" Cheap Yellow Display (CYD) into an interactive, zero-cloud offline reference tool and USB serial command transmitter for sysadmins, DevOps engineers, and developers. Featuring an categorized touch UI across 5 IT disciplines (Docker, Git, Linux Signals, DNS Records, and HTTP Codes), users can rapidly inspect command syntax, view detailed operational explanations, and tap to stream ready-to-run commands directly over USB serial to their host terminal.

---

## Key Hardware Pinout

| Peripheral | Hardware Pin | Purpose |
| :--- | :--- | :--- |
| **TFT Driver** | HSPI (Pins 12, 13, 14, 15, 2) | 320x240 ILI9341 Color Display |
| **TFT Backlight** | GPIO 21 | Backlight digital control |
| **Touch MOSI** | GPIO 32 | Dedicated VSPI Data In for XPT2046 |
| **Touch MISO** | GPIO 39 | Dedicated VSPI Data Out |
| **Touch CLK** | GPIO 25 | Dedicated VSPI Bus Clock |
| **Touch CS** | GPIO 33 | Chip Select for Touch Controller |
| **Buzzer** | GPIO 26 | Command execution and UI click feedback |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Active-low status indicator: Ready (Green), Send (Blue), Error (Red) |
| **LDR Sensor** | GPIO 34 (ADC1) | Ambient light sensing |

---

## Command Categories

1. **DOCKER**:
   - `Run Detached`: Start container in background with port mapping.
   - `Exec Shell`: Open interactive bash/sh inside running container.
   - `View Logs`: Stream live tailing logs.
   - `Prune All`: Comprehensive reclaimed disk space for unused containers and volumes.
   - `Compose Up`: Build and launch multi-container stacks.

2. **GIT RESCUE**:
   - `Undo Commit`: Soft reset keeping changes in staging.
   - `Rescue Reflog`: Recover deleted or lost branch heads.
   - `Detached HEAD`: Safe branch creation from detached commits.
   - `Stash & Clean`: Temporary shelter for uncommitted work.
   - `Force Safe`: Force push with remote lease validation.

3. **LINUX SIGNALS**:
   - `SIGTERM (15)`: Graceful process shutdown request.
   - `SIGKILL (9)`: Uncatchable kernel-level process termination.
   - `SIGHUP (1)`: Daemon configuration reload without process death.
   - `SIGINT (2)`: Terminal interrupt (Ctrl+C).
   - `SIGUSR1 (10)`: User-defined custom application signal.

4. **DNS RECORDS**:
   - Quick reference queries for `A`, `AAAA`, `CNAME`, `MX`, and `TXT` records via `dig`.

5. **HTTP STATUS CODES**:
   - Critical diagnostic explanations for `401 Unauthorized`, `403 Forbidden`, `404 Not Found`, `500 Server Error`, and `502 Bad Gateway`.

6. **Hardware SPI Bus Isolation & Non-Inverted Y**:
   - Touch bus on dedicated VSPI pins (32, 39, 25, 33) prevents display collisions.
   - Fully calibrated non-inverted landscape mapping for accurate stylus selection.

---

## Building and Flashing

Build and upload using PlatformIO:

```bash
cd elecrow_projects/01-devops-command-vault
pio run -t upload
```

Monitor serial output:

```bash
pio device monitor -b 115200
```

