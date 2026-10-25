# Project 18: WiFi Spectrum Security Auditor & Wardrive Scanner

A defensive wireless network security auditor and 2.4 GHz RF spectrum analyzer for the Cheap Yellow Display (ESP32-2432S028R). Scans all ambient Access Points, classifies encryption vulnerabilities (Open, WEP, WPA, WPA2, WPA3), charts 14-channel RF utilization histograms to locate clean channels, and flags potential rogue/evil-twin APs.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **Defensive Security Audit**: Scans ambient BSSIDs and audits cipher suites (Open unencrypted, deprecated WEP, robust WPA2/WPA3).
- **2.4 GHz RF Spectrum Histogram**: Real-time channel congestion bar chart across channels 1 to 14.
- **Rogue AP Detection**: Highlights open unencrypted networks advertising enterprise SSIDs.
- **Touch Navigation**: Tab bar toggle between detailed Access Point List and RF Channel Congestion view.
- **Visual Alert**: Tri-color RGB LED indicates scanning progress and detected network security risks.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| RGB LED | 4 (R), 16 (G), 17 (B) | Scanning / security risk status |
| Audio Buzzer | 26 | Touch navigation feedback chime |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)
