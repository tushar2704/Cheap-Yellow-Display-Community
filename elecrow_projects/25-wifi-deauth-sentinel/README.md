# 🛡️ Project 25: Wi-Fi Deauth Defense Sentinel

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Promiscuous 802.11 Frame Sentinel**:
   - Puts the ESP32 Wi-Fi radio into promiscuous management frame sniffing mode to detect wireless denial-of-service (DoS) attacks.
   - Specifically captures 802.11 Deauthentication (Subtype 12) and Disassociation (Subtype 10) frames targeting IoT devices and security cameras.
2. **Velocity Rate Meter & Attack Thresholds**:
   - Calculates live frame velocity in frames/second with peak hold tracking.
   - Classifies threat levels: **SECURE** (<5 fps), **WARNING** (5-15 fps), and **ACTIVE ATTACK** (>15 fps).
3. **Forensic Event Logging Table**:
   - Real-time logging displaying Attacker Source MAC address, Victim Station MAC, Wi-Fi channel, and packet RSSI.
4. **Channel Hopping & Lock Options**:
   - Seamlessly hops across 2.4GHz channels 1 to 13 or locks onto a single sensitive channel (e.g. your home surveillance camera channel).
5. **Hardware Safety Sirens & Strobe Indicator**:
   - Active-low RGB LED glows Green during normal operation, switching to flashing Red under active RF jamming attacks.
   - Onboard piezo buzzer sounds a multi-tone defensive siren during intrusion events (with touch mute toggle).

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — Attack Alert |
| **GPIO 26** | Audio / Buzzer | Defensive siren alarm pulses |
| **Wi-Fi Radio** | 802.11 Promiscuous | Management frame sniffer |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/25-wifi-deauth-sentinel
pio run -t upload
```
