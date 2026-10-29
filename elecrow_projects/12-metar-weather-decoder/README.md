# 🛫 Project 12: Aviation METAR & TAF Weather Decoder

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Aviation METAR Weather Decoding**:
   - Real-time decoding of aviation routine weather reports (METAR) for major international and regional airports (KJFK, EGLL, RJTT, OMDB, VIDP, KLAX).
   - Instant flight category classification: **VFR** (Visual Flight Rules), **MVFR** (Marginal VFR), **IFR** (Instrument Flight Rules), and **LIFR** (Low IFR).
2. **Visual Flight Category Status Bar & RGB Indicator**:
   - Prominent color-coded banner matching FAA/ICAO standards:
     - **Green**: VFR (> 5 SM visibility, ceiling > 3,000 ft)
     - **Blue**: MVFR (3-5 SM visibility, ceiling 1,000-3,000 ft)
     - **Yellow**: IFR (1-3 SM visibility, ceiling 500-1,000 ft)
     - **Red/Magenta**: LIFR (< 1 SM visibility, ceiling < 500 ft)
   - Onboard active-low RGB LED dynamically mirrors the airport's flight category color.
3. **Wind Vector & Runway Heading Compass**:
   - Graphical compass indicator showing live surface wind vector, speed in knots, and gust velocity.
4. **Atmospheric Instrument Cluster**:
   - Reports cloud ceiling layers, barometric altimeter setting (QNH in inHg / hPa), ambient temperature, and dewpoint spread.
5. **Interactive Touch Station Selector**:
   - Rapid-touch station picker tabs across the display for instant one-tap switching between monitored airports.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — FAA Flight Category |
| **GPIO 26** | Audio / Buzzer | Frequency tone alert on station switch |
| **Wi-Fi** | Aviation Weather API | 802.11 REST Client |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/12-metar-weather-decoder
pio run -t upload
```
