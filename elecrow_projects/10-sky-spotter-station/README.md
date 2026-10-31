# 🔭 Project 10: Sky-Spotter Flight Deck

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 🚀 Key Features

1. **Spotter Flight Board View**:
   - Real-time tabular tracking of overhead commercial and cargo aircraft with airline identity color tagging.
   - Calculates real-time compass bearing, distance in nautical miles, and line-of-sight elevation angles.
2. **Interactive 3D Spotter Vector**:
   - Tap any flight on the board to launch the 3D Spotter Line calculator.
   - Dual-gauge visualization: analog compass needle pointing to azimuth bearing and arc inclinometer indicating exact binocular tilt angle (horizon to zenith).
3. **Calibrated Touch Navigation**:
   - Instant touch tab switching between Flight Board and Spotter Vector.
   - Direct-tap aircraft selection with acoustic and visual feedback.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 25, 39, 32** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI (Hardware Calibrated) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **Wi-Fi** | ADS-B Telemetry | 802.11 REST Client |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/10-sky-spotter-station
pio run -t upload
```
