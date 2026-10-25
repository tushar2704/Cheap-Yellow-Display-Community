# Project 23: Standalone Outdoor GPS Navigator, Waypoint Compass & Speedometer

A rugged standalone GPS navigation dashboard and digital heads-up speedometer for vehicles, boats, and hiking, powered by the Cheap Yellow Display (ESP32-2432S028R) and a NEO-6M / NEO-M8N GNSS receiver. Displays real-time speed in km/h or mph, 360° rotating compass rose heading needle, MSL altitude, satellite lock telemetry, and current WGS84 geographic coordinates.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **High-Visibility Speedometer**: Large seven-segment style readout with KM/H and MPH unit switching.
- **Rotating Heading Compass**: Graphical compass rose with cardinal directions and bearing indicator.
- **WGS84 Coordinates**: Precise latitude and longitude with HDOP horizontal dilution of precision metrics.
- **Altitude MSL & Satellite Tracking**: Displays true altitude above sea level and active 3D satellite constellation count.
- **Touch Waypoint Marker**: Touch button to capture coordinates and acoustic feedback.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| GPS UART TX / RX (CN1) | 22 / 27 | NMEA serial input from GPS receiver |
| RGB LED | 4 (R), 16 (G), 17 (B) | GPS 3D fix status indicator |
| Audio Buzzer | 26 | Speed alert & touch chirp |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)
