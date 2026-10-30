# 🎛️ Project 08: Multi-Printer Wireless Command Deck (Klipper / OctoPrint)

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Moonraker & Klipper Wireless API Integration**:
   - Communicates via Wi-Fi HTTP REST calls directly to Moonraker (`/printer/gcode/script`).
   - Dispatches emergency halt command (`M112`) instantly from a large physical on-screen button.
2. **Precision Z-Offset Babystepping**:
   - Real-time adjustment buttons: `Z -0.05mm`, `Z +0.05mm`, `Z -0.01mm`, and `Z +0.01mm`.
   - Sends live `SET_GCODE_OFFSET` commands during first-layer calibration.
3. **Preheat & Temperature Macros**:
   - One-touch target presets: `PLA (210°C / 60°C)`, `PETG (240°C / 80°C)`, and `COOLDOWN (0°C / 0°C)`.
4. **DS18B20 OneWire Chamber Monitoring**:
   - Reads external digital temperature probe on the HY2.0 expansion port (GPIO 27).
   - Onboard visual and audible thermal alarm triggers if enclosure chamber exceeds safety limits.
5. **Hardware Signaling**:
   - Active-low RGB LED glows Green for healthy operation and flashes Red on thermal warnings or Emergency Stop.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 25, 39, 32** | XPT2046 Touch SPI | CS, CLK, MISO, MOSI (Dedicated VSPI, Polling Mode) |
| **Touch Calibration** | Resistive Matrix | Non-inverted Y: map(p.y, 240, 3800, 0, 240) |
| **GPIO 27** | DS18B20 OneWire | Digital chamber thermal sensor on HY2.0 port |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **Wi-Fi** | Port 7125 | Moonraker REST API client |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/08-klipper-command-deck
pio run -t upload
```
