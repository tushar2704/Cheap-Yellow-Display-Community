# 📡 Project 19: Wi-Fi Channel & Spectrum Auditor

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Full 2.4GHz Spectrum Analysis**:
   - Comprehensive spectrum scan across 802.11 channels 1 to 14.
   - Plots parabolic 20MHz/40MHz channel overlap lobes against signal strength (dBm) on the ILI9341 display.
2. **Automated Best Channel Recommender**:
   - Evaluates co-channel and adjacent-channel interference metrics to automatically compute and highlight the clearest channel (among standard non-overlapping channels 1, 6, and 11).
3. **Detailed Access Point (AP) Inspection Table**:
   - Detailed list view displaying detected network SSID names, operating channels, signal strength (dBm), and encryption standard (Open, WEP, WPA, WPA2, WPA3).
4. **Hardware Congestion LED & Audio Sweep**:
   - Active-low RGB LED glows Green for clean RF environments, Yellow for moderate clutter, and Red for saturated 2.4GHz spectrums.
   - Distinctive dual-tone ascending acoustic sweep played upon scan completion.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Spectrum Congestion |
| **GPIO 26** | Audio / Buzzer | Frequency sweep on scan completion |
| **Wi-Fi Radio** | 802.11 b/g/n Sniffer | Channel & AP Scanner |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/19-wifi-channel-auditor
pio run -t upload
```
