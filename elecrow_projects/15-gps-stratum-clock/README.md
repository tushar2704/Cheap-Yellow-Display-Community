# 🛰️ Project 15: GPS Stratum Triple Clock & Satellite Monitor

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **100% Offline Atomic Stratum Timekeeping**:
   - Interfaces directly with external UART NMEA GPS modules (NEO-6M, NEO-8M) via the onboard HY2.0 expansion port (IO27 RX, IO22 TX).
   - Zero Wi-Fi or Internet required — receives direct atomic time pulses from orbital GPS satellites.
2. **Synchronized Triple-Time Display**:
   - Large atomic **UTC / Zulu** time readout.
   - Synchronized **Local Time** with on-screen touch offset adjustment (+/- 30 min increments).
   - Geodesic telemetry status: Latitude, Longitude, Altitude (MSL), and speed.
3. **Real-Time Satellite Constellation SNR Analyzer**:
   - Visual bar chart displaying Signal-to-Noise Ratio (dB-Hz) across 8 tracked orbital satellites.
   - Color-coded signal quality indicators (Green > 35 dB-Hz, Yellow 25-35 dB-Hz, Red < 25 dB-Hz).
4. **Precision Geodesic Fix Metrics**:
   - HDOP (Horizontal Dilution of Precision) readout, 3D fix status, and satellite count.
5. **Hardware Indicators & Atomic PPS Audio Tick**:
   - Active-low RGB LED glows Green for a solid 3D lock (>4 satellites, HDOP < 2.0), Yellow for 2D lock, and Red when searching for orbital fix.
   - Subtle acoustic PPS tone tick every second replicating a laboratory atomic frequency standard.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 27** | GPS UART RX | HY2.0 Port (Connects to GPS Module TX) |
| **GPIO 22** | GPS UART TX | HY2.0 Port (Connects to GPS Module RX) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — 3D Lock Status |
| **GPIO 26** | Audio / Buzzer | Atomic 1Hz PPS tick and interaction chirp |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/15-gps-stratum-clock
pio run -t upload
```
