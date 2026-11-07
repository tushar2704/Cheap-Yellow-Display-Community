# 💾 Project 65 — CYD PyDOS Vintage MS-DOS Handheld Computer

> **A standalone retro 1980s/1990s MS-DOS 6.22 micro-computer terminal running directly on the Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch). Features a BIOS POST boot sequence, authentic 640KB RAM counter, interactive DOS command line interface (CLI), on-screen tactile QWERTY chiclet keyboard, CRT phosphor theme switcher, simulated floppy/HDD drive activity LED, and IBM PC speaker audio chimes!**

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Display: ILI9341](https://img.shields.io/badge/Display-ILI9341_2.8%22-yellow.svg)](https://platformio.org/)
[![Touch: XPT2046](https://img.shields.io/badge/Touch-XPT2046_Resistive-red.svg)](https://platformio.org/)
[![Brand: IamNotIntoTech](https://img.shields.io/badge/Branded-IamNotIntoTech-FEA000.svg)](https://youtube.com/@IamNotIntoTech)

---

### 📺 Watch the Masterclass & 3D Enclosure
- 🎬 **Tutorial & Code Walkthrough**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printable Retro Handheld Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🌟 Key Features

1. **Authentic IBM PC / MS-DOS Boot & POST Sequence**:
   - ROM BIOS Version 4.04 `(C) IamNotIntoTech Corporation`.
   - Rapid memory count test: `640 KB Conventional Memory OK` + `384 KB Extended Memory OK`.
   - Classic 800 Hz PC Speaker POST beep on `GPIO 26`.
2. **Interactive DOS Command Shell (CLI)**:
   - `DIR` — Catalogs internal disk drives with filenames, extensions, byte sizes, file timestamps, and remaining free clusters (`472,192 bytes free`).
   - `VER` — Displays `IamNotIntoTech MS-DOS Version 6.22` and ESP32 hardware specs (240MHz dual-core, 520KB SRAM).
   - `MEM` — Comprehensive memory report table breaking down Conventional, Upper, and Extended (XMS) memory.
   - `CLS` — Instant console viewport sweep back to the top prompt.
   - `TYPE <file>` — Displays document contents (e.g. `TYPE README.TXT` and `TYPE AUTOEXEC.BAT`).
   - `DATE` & `TIME` — Reports system date and elapsed session uptime.
   - `BEEP` — Plays classic PC speaker frequency tone sequence.
   - `COLOR` — Cycles CRT phosphor themes (Amber Phosphor `0xFD20`, Matrix Emerald Green `0x07E0`, Cyber Cyan `0x07FF`, and Paper White `0xFFFF`).
   - `RUN` — Executes internal demo benchmark calculating prime sieves with acoustic feedback.
   - `HELP` — Built-in quick command reference.
3. **Tactile On-Screen QWERTY Chiclet Keyboard**:
   - 42-key layout covering Numeric row, QWERTY, ASDF, ZXCV, and dedicated fast command buttons (`DIR`, `CLS`, `VER`, `MEM`, `COL`).
   - Visual keypress inversion highlight and mechanical typewriter acoustic click.
4. **Hardware Drive Activity LED**:
   - Active-low green LED on `GPIO 16` flashes during disk lookups (`DIR`, `TYPE`) mirroring authentic 5.25" / 3.5" floppy disk seek sounds.
5. **Auto-Scrolling CRT Buffer**:
   - 9-line circular console window with blinking cursor block (`_`) and smooth line feed.

---

## 🔌 Hardware Pinout (ESP32-2432S028R)

| Component | Pin Function | ESP32 GPIO |
|---|---|---|
| **ILI9341 Display** | MOSI / MISO / SCLK / CS / DC | `GPIO 13` / `GPIO 12` / `GPIO 14` / `GPIO 15` / `GPIO 2` |
| **Backlight PWM** | TFT Backlight Enable | `GPIO 21` (Active HIGH) |
| **XPT2046 Touch** | CLK / MISO / MOSI / CS / IRQ | `GPIO 25` / `GPIO 39` / `GPIO 32` / `GPIO 33` / `GPIO 36` |
| **HDD/Floppy LED** | Drive Access Indicator | `GPIO 16` (Green Active LOW) |
| **PC Speaker** | Audio POST & Beep | `GPIO 26` |
| **LDR Sensor** | Ambient Light ADC | `GPIO 34` |

---

## 🛠️ How to Compile & Flash

### PlatformIO (Recommended)
```bash
cd projects/advanced/65-cyd-pydos-handheld-pc
pio run --target upload
```

### Arduino IDE
1. Open `65-cyd-pydos-handheld-pc.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and configure `User_Setup.h` for ILI9341.
4. Install **XPT2046_Touchscreen**.
5. Click **Upload**.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a></b> &bull; 
  <b>3D Enclosures by <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a></b>
</p>
