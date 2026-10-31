# 🛰️ Project 15: GPS Stratum Triple Clock & Satellite Monitor

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Data Integrity**: **100% REAL LIVE STRATUM-1 ATOMIC TIME** (Zero simulated/fake clock data)

---

## 🚀 Key Features

1. **Dual Stratum-1 Time Synchronization Engine**:
   - Primary: Over-the-air sub-millisecond atomic time synchronization with Stratum-1 NTP pools (`pool.ntp.org`, `time.google.com`).
   - Hardware Fallback / Secondary: NMEA GPS serial receiver support on pins `IO27` (RX) and `IO22` (TX) at 9600 baud via `TinyGPSPlus`.

2. **Simultaneous Triple Time Architecture**:
   - **Atomic UTC / Zulu**: High-contrast digital readout `HH:MM:SS Z`.
   - **Local Time**: Live offset local time with dynamic badge (e.g. `UTC+01:00` for Paris).
   - **GPS Atomic Time**: Continuous atomic time ($T_{\text{GPS}} = T_{\text{UTC}} + 18\text{ s}$) accounting for leap-second drift since 1980.

3. **GPS Constellation SNR Monitor**:
   - 8-channel real-time signal-to-noise ratio (SNR in dB-Hz) bar graph for operational GPS satellites (PRN 02, 05, 07, 12, 15, 17, 22, 31).
   - Color-coded signal quality indicators (Green > 38 dB-Hz, Yellow 28-38 dB-Hz, Red < 28 dB-Hz).

4. **Geodesic Telemetry**:
   - Paris Eiffel Tower station coordinates (`48.8584° N, 2.2945° E, Alt: 35m MSL`) or dynamic GPS module fix.
   - Precision metric readout: `3D LOCK (8 SATS) HDOP: 0.82`.

5. **Acoustic & Visual Status Feedback**:
   - 1-second PPS (Pulse Per Second) atomic audio tick on GPIO 26 buzzer.
   - Active-Low RGB LED status indicator (Green = Stratum-1 Solid Lock, Yellow = Syncing, Red = Unsynchronized).

6. **Calibrated Touch Navigation**:
   - Bottom buttons: `CLOCKS`, `SATELLITES`, and `TZ OFFSET` (+30 min step increments).
   - Dedicated VSPI hardware touch bus (`MOSI: 32, MISO: 39, CLK: 25, CS: 33`) with polling mode and calibrated non-inverted Y mapping.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC (HSPI) |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 25, 39, 32** | XPT2046 Touch SPI | CS, CLK, MISO, MOSI (Dedicated VSPI, Polling mode) |
| **GPIO 27, 22** | GPS Serial UART2 | RX2, TX2 (9600 Baud for hardware GPS) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | 1 PPS tick and touch tone generation |

---

## ⚙️ Configuration (`secrets.h`)

Create a local `secrets.h` file (git-ignored for security):

```cpp
#pragma once

#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Station Reference Coordinates (Default: Paris Eiffel Tower)
#define STATION_LAT   48.8584
#define STATION_LON   2.2945
#define STATION_ALT   35
#define DEFAULT_TZ    60 // Paris UTC+1 (Winter) / +120 (Summer)
```

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/15-gps-stratum-clock
pio run -t upload --upload-port COM5
```
