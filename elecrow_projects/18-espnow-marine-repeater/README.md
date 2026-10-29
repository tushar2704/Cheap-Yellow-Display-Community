# ⛵ Project 18: ESP-NOW Marine Instrument Repeater

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Router-Free ESP-NOW Marine Mesh Protocol**:
   - Direct 2.4GHz peer-to-peer radio connection listening to remote masthead and hull sensor nodes.
   - Zero access point, zero local Wi-Fi router, and zero cellular connection required.
2. **Polar Rose 360° Sailing Wind Gauge**:
   - Graphical sailing compass dial displaying apparent wind angle with color-coded Port (Red) and Starboard (Green) sectors.
   - High-visibility Apparent Wind Speed readout in Knots or Meters/Second.
3. **Multi-Sensor Cockpit Telemetry Stack**:
   - Water Depth gauge with shallow water safety threshold alarm.
   - High-precision Barometric Pressure and squall storm warning detector.
   - Water temperature sensor readout and masthead transmitter battery monitor.
4. **Night Vision & Nautical Palette Switcher**:
   - Instant touchscreen toggle between full-color daylight display and red-tinted maritime night mode preserving nighttime watch visual acuity.
5. **Hardware Multi-Sensory Safety Alarms**:
   - Active-low RGB LED glows Green for normal telemetry, Yellow for high winds (>22 kts), and flashes Red for shallow water hazards (< 2.5m).
   - Piezo buzzer sounds deep audio squawk warning pulses on shallow depth events.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Marine hazard |
| **GPIO 26** | Audio / Buzzer | Shallow depth and gale alert tones |
| **Wi-Fi Radio** | ESP-NOW Protocol | 2.4GHz Peer-to-Peer Receiver |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/18-espnow-marine-repeater
pio run -t upload
```
