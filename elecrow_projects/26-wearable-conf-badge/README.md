# 🏷️ Project 26: Wearable Tech Conference Badge & vCard Deck

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Interactive Conference Attendee Name Badge**:
   - High-contrast visual attendee card displaying name, YouTube handle (`@IamNotIntoTech`), VIP badge status, and speaker talk schedule.
   - Built for maker faires, hacker conferences, and tech conventions.
2. **Instant vCard Networking QR Code**:
   - Renders a scannable 2D QR code encoding full contact details, name, channel URL, and organization.
   - Allows attendees to instantly save your contact details into their phone's address book with a single camera scan.
3. **Ambient RF Crowd Activity & Fuel Gauge**:
   - Visual battery gauge monitoring LiPo cell voltage and remaining capacity percentage.
   - Onboard ambient light (IO34 LDR) and 2.4GHz crowd beacon detection indicator.
4. **Hardware Neon Mood Glow & Audio**:
   - Active-low RGB LED cycles through vibrant neon color sweeps (Red, Gold, Green, Cyan, Blue, Purple).
   - Audio chirps provide responsive feedback on touch interactions.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 34** | Ambient LDR Light Sensor | ADC1 CH6 |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Neon mood cycle |
| **GPIO 26** | Audio / Buzzer | Frequency tone chirps |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/26-wearable-conf-badge
pio run -t upload
```
