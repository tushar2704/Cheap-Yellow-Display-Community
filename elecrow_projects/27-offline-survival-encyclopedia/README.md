# 🏕️ Project 27: Offline Survival Encyclopedia & SOS Beacon

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **100% Offline Wilderness & Disaster Knowledge Base**:
   - Zero-cloud, standalone survival guide operational without internet, cellular connectivity, or GPS.
   - MicroSD Card database support (IO5, 18, 19, 23) paired with an internal high-speed flash cache.
2. **Four Mission-Critical Survival Domains**:
   - **First Aid & Trauma**: Adult CPR protocols (30:2 ratio @ 100-120 bpm) and combat tourniquet placement.
   - **Water Purification**: Rolling boil duration vs altitude and SODIS solar UV disinfection.
   - **Knots & Rigging**: Step-by-step instructions for the Bowline, Taut-line hitch, and Square knot.
   - **Navigation & Signaling**: Solar shadow-stick compass alignment and universal distress signaling.
3. **Emergency Optical & Acoustic SOS Beacon**:
   - One-touch `SOS BEACON` mode executing universal Morse Code (`... --- ...`).
   - Active-low RGB LED produces high-visibility white optical strobe flashes.
   - Onboard piezo buzzer outputs synchronized audible distress pulses for search & rescue teams.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 5, 18, 19, 23** | MicroSD Card SPI | CS, SCK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — SOS Strobe |
| **GPIO 26** | Audio / Buzzer | Morse code audio distress pulses |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/27-offline-survival-encyclopedia
pio run -t upload
```
