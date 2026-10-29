# 🛡️ Project 06: ESP32 Ad-Blocking DNS Sinkhole & Traffic Monitor

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **UDP Port 53 DNS Sinkhole Server**:
   - Resolves legitimate LAN queries and redirects telemetric, tracking, and advertising domains to `0.0.0.0`.
   - Upstream forwarder configured to high-speed privacy resolvers (e.g. `1.1.1.1`).
2. **5-Minute Temporary Snooze Button**:
   - Giant touchscreen button pauses blocking for 300 seconds to allow affiliate or broken shopping links to function without reconfiguring router settings.
   - Live visual countdown timer with instant manual resume.
3. **Live Traffic Telemetry**:
   - Displays real-time metrics: Total Queries, Ads Blocked, and Block Percentage.
   - Dual-channel domain ticker showing the most recently sinkholed and allowed domains.
4. **Hardware Visual & Audio Alerts**:
   - Active-low RGB LED pulses Red whenever an ad tracker is sunk, maintaining steady Green during nominal operation.
   - Frequency audio tone confirmation on snooze toggle.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **Wi-Fi** | UDP Port 53 | DNS Sinkhole daemon |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/06-dns-sinkhole-monitor
pio run -t upload
```
