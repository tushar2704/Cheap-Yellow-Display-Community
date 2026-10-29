# 📻 Project 13: HamClock Mini Dashboard

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Space Weather & Ionospheric Indicators**:
   - Displays real-time Solar Flux Index (**SFI**), Sunspot Number (**SSN**), Geomagnetic **A-Index**, and Planetary **K-Index**.
   - Real-time color-coded K-index gauge indicating geomagnetic storm status (Quiet, Unsettled, Active, Storm).
2. **HF Propagation Forecast Matrix**:
   - Live band condition matrix covering 80m, 40m, 20m, 15m, and 10m bands.
   - Day vs Night ionospheric propagation status (Good, Fair, Poor) with high-contrast color indicators.
3. **POTA / SOTA Spotter Feed**:
   - Live stream of Parks on the Air (POTA) and Summits on the Air (SOTA) activator stations.
   - Shows activator callsign, operating frequency (MHz), mode (CW, SSB, FT8), and park/summit reference code.
4. **Maidenhead Grid Distance & Bearing Calculations**:
   - Computes real-time great-circle bearing (degrees) and geodesic distance (km) from your home QTH locator.
5. **Hardware Multi-Sensory Feedback**:
   - Active-low RGB LED glows Green for geomagnetic quiet, Amber for unsettled, and Red for storm alerts.
   - Piezo buzzer sounds discrete frequency chirps (1800Hz / 2200Hz / 2500Hz) on tab switches and spot selection.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Geomagnetic alert |
| **GPIO 26** | Audio / Buzzer | Frequency tone chirps |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/13-hamclock-mini
pio run -t upload
```
