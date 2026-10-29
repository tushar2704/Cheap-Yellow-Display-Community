# 🌐 Project 02: Chmod & VLSM Visual Network Architect

> **Elecrow 2.8" CYD Series**  
> **Channel Branding**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R  
> **Open-Source Base**: [lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit (V2)](https://github.com/lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit)

---

## 📺 YouTube Presentation & Angles

* **Channel**: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
* **Video Title**: *"Stop Using Sketchy Websites to Calculate Subnets"*
* **Hook**: Don't leak private IP architectures or risk typos with online subnet calculators during server configurations. This dedicated $15 hardware architect calculates CIDR slices and chmod permission octals completely offline with tactile touch feedback.

---

## 🚀 Key Features

1. **Interactive VLSM Subnet Slicer**:
   - Select CIDR prefixes from `/24` through `/30`.
   - Real-time computation of Subnet Mask, Wildcard Mask, Usable Host Count, IP range, and Broadcast address.
   - Graphical visual split bar illustrating how a `/24` block subdivides into smaller subnets.
2. **Interactive 9-Box Chmod Matrix**:
   - 3 User classes: `Owner (u)`, `Group (g)`, `Public (o)`.
   - 3 Permissions: `Read (4)`, `Write (2)`, `Execute (1)`.
   - Instant live conversion to octal command (e.g. `chmod 755`) and symbolic string (`-rwxr-xr-x`).
   - One-touch fast presets: `[777]`, `[755]`, `[644]`, and `[600]`.
3. **Dual Mode Switcher**:
   - Tap top tabs to toggle seamlessly between VLSM Network calculation and Chmod permission engineering.
4. **Zero Latency & 100% Offline**: Runs on ESP32 silicon with zero network requirements.

---

## 🔌 Hardware Pin Mapping

| Pin | Function | Notes |
|---|---|---|
| **GPIO 13, 12, 14, 15, 2** | ILI9341 Display SPI | MOSI, MISO, SCK, CS, DC |
| **GPIO 21** | Backlight Control | Active HIGH |
| **GPIO 33, 36, 14, 12, 13** | XPT2046 Touch SPI | CS, IRQ, CLK, MISO, MOSI |
| **GPIO 4, 16, 17** | RGB LED | Red, Green, Blue (Active LOW) |
| **GPIO 26** | Audio / Buzzer | Frequency tone feedback on touch |

---

## 🛠️ How to Compile & Flash

Using PlatformIO CLI:
```bash
cd elecrow_projects/02-chmod-vlsm-architect
pio run -t upload
```

---

## 🖨️ 3D Enclosure Concept
* **Server-Rack Magnetic Puck**: Rear recesses for two 10x2mm neodymium magnets so the 2.8" panel snaps directly onto steel server racks or patch panels.
