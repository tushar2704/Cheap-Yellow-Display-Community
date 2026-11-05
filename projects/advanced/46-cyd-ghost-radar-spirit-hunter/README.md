# 👻 Project 46: CYD Ghost Radar & EMF Spirit Tracker (Paranormal Hunter Deck)

Turn the **Cheap Yellow Display (ESP32-2432S028R)** into a full **Phasmophobia / Ghostbusters P.K.E. paranormal hunting deck**!

Featuring a 360-degree rotating CRT sonar radar with phosphor decay, dynamic EMF Level 1 to 5 meter, "cold spot" ambient shadow temperature gauge, and an authentic **Spirit Box EVP radio sweep** that decodes eerie ghostly words into the on-screen terminal accompanied by acoustic static chirps!

---

## 📺 Channel & Video Showcase

- **Main Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech) — Tech humor, paranormal gadgets, and ESP32 experiments.
- **3D Enclosures & Design**: [@Makerrdesigns](https://youtube.com/@Makerrdesigns) — Ghostbusters P.K.E. meter handheld case with dual antenna wings.

---

## ⚡ Features

1. **360° Circular CRT Radar Scope**:
   - Rotating sweep beam with authentic green phosphor decay.
   - 4-ring range gate with precision crosshairs.
   - Sonar acoustic ping on each complete revolution.
2. **Dynamic Entity / Ghost Blip Tracking**:
   - Paranormal anomalies appear on the radar scope with randomized bearing and distance.
   - Entity blips flash and fade with persistence.
   - Hitting an entity with the sweep beam triggers a high-pitched proximity alert!
3. **EMF 5-Level Visual Meter & Siren**:
   - 5-stage LED bar: Level 1 (Safe) $\rightarrow$ Level 3 (Activity) $\rightarrow$ **Level 5 (Paranormal Event)**!
   - Level 5 triggers full Red RGB LED strobe and screeching piezo alarm.
4. **"Spirit Box" EVP Frequency Sweeper**:
   - Emulates rapid radio channel scanning static through the onboard buzzer (GPIO 26).
   - Decodes 12+ chilling EVP words (*"BEHIND YOU"*, *"RUN"*, *"COLD"*, *"WATCHING"*, *"FOUND"*, *"LEAVE NOW"*).
5. **Ambient "Cold Spot" Detection**:
   - Samples the onboard LDR light sensor (GPIO 34).
   - Passing hands or shadows cast over the sensor drop simulated temperatures into freezing negative values (e.g. `-4.2°C FREEZING`).
6. **Creator Video Prank Touch Trigger**:
   - Tap directly inside the radar scope to manually summon an entity blip on demand during video shoots!

---

## 🛠️ Hardware Requirements

| Component | Pin / Specification | Notes |
| :--- | :--- | :--- |
| **Board** | ESP32-2432S028R (CYD) | 2.8" Resistive Touch Screen |
| **Display** | ILI9341 SPI (320x240) | Built-in |
| **Touch** | XPT2046 SPI (CS=33, IRQ=36) | Tap to spawn anomalies or trigger EVP |
| **Speaker / Buzzer** | GPIO 26 | Sonar pings, static sweeps, EMF 5 siren |
| **RGB LED** | GPIO 4 (R), 16 (G), 17 (B) | Dynamic EMF threat lighting |
| **Light Sensor** | GPIO 34 (LDR) | Shadow / cold spot thermal detector |

---

## 🚀 Quick Start

### PlatformIO
```bash
cd projects/advanced/46-cyd-ghost-radar-spirit-hunter
pio run --target upload
```

### Arduino IDE
1. Open `46-cyd-ghost-radar-spirit-hunter.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Install **TFT_eSPI** and **XPT2046_Touchscreen**.
4. Upload and search for ghosts in the dark!

---

## 🧪 Test Report

Full hardware testing and memory metrics are documented in [TEST_REPORT.md](TEST_REPORT.md).

