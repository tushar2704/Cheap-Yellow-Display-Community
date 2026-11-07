# 📡 Project 69 — CYD LoRa APRS Tactical Position Tracker & Beacon

> **A standalone tactical amateur radio APRS (Automatic Packet Reporting System) position tracker and emergency beacon console for the Cheap Yellow Display (ESP32-2432S028R 2.8" Resistive Touch). Features a Maidenhead tactical radar grid (FN31pr), real-time GPS telemetry, AX.25 raw packet sniffer, beacon configuration deck, emergency SOS distress broadcaster with acoustic Bell 202 AFSK audio modulation, and status RGB LED indicators!**

[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Display: ILI9341](https://img.shields.io/badge/Display-ILI9341_2.8%22-yellow.svg)](https://platformio.org/)
[![Touch: XPT2046](https://img.shields.io/badge/Touch-XPT2046_Resistive-red.svg)](https://platformio.org/)
[![Brand: IamNotIntoTech](https://img.shields.io/badge/Branded-IamNotIntoTech-FEA000.svg)](https://youtube.com/@IamNotIntoTech)

---

### 📺 Watch the Masterclass & 3D Enclosure
- 🎬 **Tutorial & Code Walkthrough**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Printable Tactical Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 🌟 Key Features

1. **4 Touch-Navigated Tactical Views**:
   - **View 1: Tactical Radar Grid** — Polar range grid with Maidenhead grid locator (`FN31pr`), local station position (`40°42.82' N / 074°00.41' W`), elevation (`142m`), speed (`28 km/h`), and plotted nearby APRS stations (`[Car]`, `[Base]`, `[Balloon]`, `[Hiker]`). Includes a 1-tap manual `[SEND BEACON]` trigger.
   - **View 2: AX.25 Packet Sniffer Log** — Raw terminal monitor displaying incoming decoded APRS frames with source callsign, destination routing (`APRS,WIDE1`), telemetry message payload, and signal RSSI in dBm.
   - **View 3: Beacon Configuration Deck** — Station callsign & SSID editor (`N0TECH-9`), broadcast rate timer (SmartBeaconing), and RF power monitor (100mW / 144.390 MHz).
   - **View 4: Emergency SOS Distress Beacon** — High-priority distress broadcaster that transmits continuous emergency position packets with audio alert siren and crimson LED strobe.
2. **Bell 202 AFSK Audio Synthesis**:
   - Synthesizes authentic 1200 Hz mark / 2200 Hz space AFSK packet bursts on onboard piezo `GPIO 26`.
3. **Hardware LED Telemetry**:
   - Blue pulse on packet transmission (TX).
   - Green pulse on packet reception (RX).
   - Flashing crimson LED strobe on Emergency SOS active.
4. **LDR Sensor**:
   - Ambient room light monitoring on `GPIO 34`.

---

## 🔌 Hardware Pinout (ESP32-2432S028R)

| Component | Pin Function | ESP32 GPIO |
|---|---|---|
| **ILI9341 Display** | MOSI / MISO / SCLK / CS / DC | `GPIO 13` / `GPIO 12` / `GPIO 14` / `GPIO 15` / `GPIO 2` |
| **Backlight PWM** | TFT Backlight Enable | `GPIO 21` (Active HIGH) |
| **XPT2046 Touch** | CLK / MISO / MOSI / CS / IRQ | `GPIO 25` / `GPIO 39` / `GPIO 32` / `GPIO 33` / `GPIO 36` |
| **Status RGB LED** | TX (Blue) / RX (Green) / SOS (Red) | `GPIO 4` / `GPIO 16` / `GPIO 17` (Active LOW) |
| **Piezo Speaker** | Bell 202 AFSK Tones | `GPIO 26` |
| **LDR Sensor** | Ambient Light ADC | `GPIO 34` |

---

## 🛠️ How to Compile & Flash

### PlatformIO (Recommended)
```bash
cd projects/advanced/69-cyd-lora-aprs-tactical-beacon
pio run --target upload
```

### Arduino IDE
1. Open `69-cyd-lora-aprs-tactical-beacon.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and configure `User_Setup.h` for ILI9341.
4. Install **XPT2046_Touchscreen**.
5. Click **Upload**.

---

<p align="center">
  <b>Built with 🟡 by <a href="https://youtube.com/@IamNotIntoTech">IamNotIntoTech</a></b> &bull; 
  <b>3D Enclosures by <a href="https://youtube.com/@Makerrdesigns">Makerrdesigns</a></b>
</p>
