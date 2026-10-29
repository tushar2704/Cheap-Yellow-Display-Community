# 🖥️ Project 03: Standalone Homelab Wake-on-LAN (WoL) Touch Deck

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Standard 102-Byte WoL Magic Packet Engine**:
   - Sends the industry-standard 6x `0xFF` + 16x target MAC payload over UDP broadcast port 9.
2. **Security PIN Lock Protection**:
   - 4-digit security PIN keypad on the resistive touch panel prevents unauthorized or accidental server power-ups. Default PIN is `1337`.
3. **Multi-Node Homelab Rack Management**:
   - Pre-configured node slots for Proxmox VE, TrueNAS Scale, Plex/Jellyfin, and high-power AI GPU workstations.
4. **Booting Lifecycle Visualization**:
   - Node status dots and border highlights glow Red (Offline), Amber (Booting in progress), and Green (Online).
5. **Acoustic & Tricolor LED Alerts**:
   - Audio chirps on keypress and packet dispatch; RGB LED pulses Amber while booting and glows Green when ready.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone generation |
| **Wi-Fi** | 802.11 b/g/n | UDP broadcast transmission |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/03-homelab-wol-deck
pio run -t upload
```
