# Project 26: Air-Gapped BIP39 Bitcoin & EVM Hardware Cold Wallet

A fully air-gapped cryptographic signing computer and cold storage vault for the Cheap Yellow Display (ESP32-2432S028R). Designed with zero radio transmission (WiFi and Bluetooth physically disabled in silicon at boot), it signs Partially Signed Bitcoin Transactions (PSBT) and EVM typed data purely via visual 2D QR code exchanges with Specter Desktop, Sparrow, or BlueWallet.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **Strict Air-Gap Isolation**: Radios disabled in firmware (`WiFi.mode(WIFI_OFF)`, `btStop()`); no network exposure.
- **BIP39 / BIP84 Derivation**: Native SegWit ($m/84'/0'/0'$) and Taproot key derivation telemetry.
- **PSBT Inspection Screen**: Displays verified recipient address, transfer amount in BTC, and miner fee in satoshis before signing.
- **Visual Animated QR Output**: Emits signed transaction envelope as a high-density 2D QR matrix for optical camera air-gap import.
- **Physical Touch Confirmation**: Tactile confirm and sign button with high-frequency confirmation chime.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| RGB LED | 4 (R), 16 (G), 17 (B) | Cryptographic transaction signing status |
| Audio Buzzer | 26 | Optical signature confirmation chime |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)
