# 📡 Project 24: Pocket RF & IR Universal Multi-Tool

> **Elecrow 2.8" CYD Series**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  

---

## 📺 Channel & Tutorial

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  

---

## 🚀 Key Features

1. **Universal Infrared Touch Remote**:
   - Touchpad controller with essential buttons (Power, Mute, Source/HDMI, Volume +/-, Channel +/-).
   - Generates precision 38kHz modulated carrier pulses on expansion pin IO27.
2. **IR Sniffer, Analyzer & Signal Cloner**:
   - Decodes 32-bit hex command codes across NEC, Sony, Samsung, and RC5 infrared protocols.
   - Graphical pulse-train waveform visualizer displaying raw mark/space microsecond timing.
   - Built-in `LEARN / SNIFF` and `REPLAY SIGNAL` action buttons for cloning household remotes.
3. **Dynamic Guest Wi-Fi QR Generator**:
   - Embedded QR code generator rendering high-contrast 2D barcodes for instant camera scanning.
   - Eliminates typing complex network passwords for guests or homelab visitors.
4. **Hardware Multi-Sensory Feedback**:
   - Active-low RGB LED emits Purple flashes during IR transmission and Cyan upon protocol capture.
   - Piezo speaker sounds confirmation clicks upon touch commands.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 27** | IR Transmitter Out | 38kHz Modulated Carrier (HY2.0) |
| **GPIO 22** | IR Demodulator In | 38kHz Receiver (HY2.0) |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) — IR Activity |
| **GPIO 26** | Audio / Buzzer | Frequency tone clicks |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/24-pocket-rf-ir-tool
pio run -t upload
```
