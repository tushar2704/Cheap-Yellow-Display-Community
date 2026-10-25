# Project 15: Bitcoin & Lightning Network Node Telemetry Monitor

A dedicated desk monitor for Bitcoin Core and Lightning Network (LND / CLN / Umbrel / RaspiBlitz) nodes on the Cheap Yellow Display (ESP32-2432S028R). Tracks latest block height, live BTC/USD spot price, real-time mempool transaction fee rates (High/Med/Low priority sat/vB), channel liquidity distribution (local vs remote balance), and 24-hour routed sats.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **Bitcoin Core Telemetry**: Synced block height ticker, node connection health, and BTC/USD spot pricing.
- **Mempool Fee Estimator**: Live recommended sat/vB fee levels (Low, Medium, High Next-Block).
- **Lightning Channel Balances**: Visual liquidity split bar (local outbound vs remote inbound sats).
- **Interactive Touch Tab Bar**: Seamless touch navigation between Overview, Mempool, and Channels views.
- **Activity Indicator**: Onboard RGB LED pulse on newly detected transactions and fee adjustments.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| RGB LED | 4 (R), 16 (G), 17 (B) | Block & fee activity LED indicator |
| Audio Buzzer | 26 | Tab tap feedback chime |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)
