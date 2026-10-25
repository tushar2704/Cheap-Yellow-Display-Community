# Project 16: Environmental AQI, PM2.5, CO2 & TVOC Station

A precision indoor and ambient air quality monitoring station on the Cheap Yellow Display (ESP32-2432S028R). Features real-time EPA AQI color scoring, Sensirion SCD40 / BME680 / PMS5003 sensor telemetry, rolling 24-hour AQI sparkline trends, acoustic ventilation breach alerts, and touch controls.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **EPA Air Quality Index**: Color-coded 0-500 scale (Good, Moderate, Sensitive, Unhealthy, Hazardous).
- **Multi-Sensor Metrics**: Carbon Dioxide ($CO_2$ ppm), fine particulate ($PM_{2.5}$ & $PM_{10}$), TVOC, temperature & humidity.
- **Historical Sparkline**: 30-sample rolling trend view for trend analysis.
- **Acoustic & Optical Alert**: Warning tone and tri-color RGB indicator when $CO_2$ breaches safe thresholds (>1200 ppm).
- **Touch Actions**: Mute buzzer alarms and zero-calibrate sensor baseline.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| I2C SDA / SCL (CN1) | 22 / 27 | External Sensor Bus (SCD40, BME680, SGP30) |
| RGB LED | 4 (R), 16 (G), 17 (B) | Environmental health status indicator |
| Audio Buzzer | 26 | Fresh-air ventilation reminder alarm |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)
