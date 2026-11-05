# ☢️ Project 43: CYD Pip-Boy 3000 (Wasteland Geiger Deck & 2.4GHz RF Rads Meter)

A retro-futuristic **Fallout Pip-Boy 3000** personal information and radiation monitoring terminal engineered for the **Cheap Yellow Display (ESP32-2432S028R)**. 

Combines iconic Vault-Tec CRT monochrome green aesthetics, animated Vault Boy mascot moods, authentic high-frequency Geiger counter clicks, and **real-world electromagnetic radiation detection** by converting ambient 2.4GHz WiFi packet traffic into simulated microSieverts per hour ($\mu\text{Sv/h}$) and RADS!

---

## 📺 Channel & Video Showcase

- **Main Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) — Tech humor, ESP32 experiments, and retro gaming builds.
- **3D Enclosures & Design**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns) — Custom wrist-mountable Pip-Boy 3000 arm chassis 3D models.

---

## ⚡ Features

1. **Real 2.4GHz Promiscuous Sniffer**:
   - Puts the ESP32 radio into promiscuous sniffer mode to capture all surrounding 802.11 b/g/n packets in real time.
   - Channel-hops across all 13 channels every 150ms.
   - Measures raw electromagnetic packet density: walking closer to a WiFi router or microwave violently ramps up detected traffic!
2. **Dynamic Geiger Click Audio Engine**:
   - Synthesizes authentic high-voltage electrostatic ionization clicks through the built-in piezo speaker (GPIO 26).
   - Click frequency dynamically scales from ambient cosmic background (~1 click every 2 seconds) up to a frantic crackling swarm (>60 clicks per second).
3. **Animated Vault Boy Mascot**:
   - **Low Rads (< 35 $\mu\text{Sv/h}$)**: Smiling, winking thumbs-up Vault Boy in Vault 101 jumpsuit.
   - **Medium Rads (35–100 $\mu\text{Sv/h}$)**: Dizzy, sweating Vault Boy with dizzy X eyes and an amber status alert.
   - **Critical Rads (> 100 $\mu\text{Sv/h}$)**: Irradiated Glowing Ghoul Skull with red flashing status LED!
4. **Touchscreen Pip-Boy OS Tabs**:
   - **`[1. STAT]`**: Dosimeter dial, real-time Rads reading, peak radiation history, RF packet counter, HP & AP bars.
   - **`[2. RADAR]`**: 13-channel real-time RF spectrum histogram bars showing active microwave/router channel occupancy.
   - **`[3. RADIO]`**: Diamond City Wasteland Radio chiptune melody player powered by the onboard buzzer.
5. **Hardware RGB Alert Integration**:
   - Vault Green (Safe) $\rightarrow$ Rad Warning Amber $\rightarrow$ Strobe Crimson (Biohazard alert).

---

## 🛠️ Hardware Requirements

| Component | Pin / Specification | Notes |
| :--- | :--- | :--- |
| **Board** | ESP32-2432S028R (CYD) | 2.8" Resistive Touch Screen |
| **Display** | ILI9341 SPI (320x240) | Built-in |
| **Touch** | XPT2046 SPI (CS=33, IRQ=36) | Calibrated resistive touch |
| **Speaker / Buzzer** | GPIO 26 | Built-in Piezo buzzer |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Active LOW status lighting |
| **Radio** | Built-in 2.4GHz RF | Promiscuous packet monitor |

---

## 🚀 Quick Start

### PlatformIO
```bash
cd projects/advanced/43-cyd-pip-boy-fallout-rad-meter
pio run --target upload
```

### Arduino IDE
1. Open `43-cyd-pip-boy-fallout-rad-meter.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and **XPT2046_Touchscreen**.
4. Upload and explore the Wasteland!

---

## 🧪 Test Report

Full hardware testing and memory metrics are documented in [TEST_REPORT.md](TEST_REPORT.md).

