# Project 22: PN532 NFC & RFID Tag Reader / Access Card UID Analyzer

A standalone High-Frequency 13.56 MHz RFID and NFC smart card analyzer using the Cheap Yellow Display (ESP32-2432S028R) paired with a PN532 / RC522 transceiver module. Decodes card Unique Identifiers (UID in Hex and Dec), card technology (Mifare Classic 1K/4K, Ultralight, NTAG213/215/216), SAK/ATQA metadata, and sector 0 memory blocks, with acoustic access grant/deny validation.

Target Board: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)  
Author: tushar2704 <tushar.inseec@gmail.com>  
YouTube: [@IamNotIntoTech](https://www.youtube.com/@IamNotIntoTech) | [@Makerrdesigns](https://www.youtube.com/@Makerrdesigns)  
Web Flasher: [Launch Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/)

---

## Features
- **NFC Tag Protocol Identification**: ISO/IEC 14443 Type A/B card detection with ATQA, SAK, and BCC check.
- **UID Inspection & Hex Dump**: Clear 4-byte or 7-byte hex serial readout and Manufacturer Block 0 preview.
- **Access Control Simulator**: Toggle authorization rules with dual-state Green (Granted) and Red (Rejected) screens.
- **Acoustic Feedback**: Distinct access granted high-pitched double-beep vs low denial buzz.
- **Touch Interaction**: Scan simulation trigger and access list toggle.

---

## Hardware Pinout

| Function | ESP32 GPIO | Description |
|---|---|---|
| TFT MISO / MOSI / SCK | 12 / 13 / 14 | SPI Bus |
| TFT CS / DC / Backlight | 15 / 2 / 21 | Display Control |
| Touch CS / IRQ | 33 / 36 | XPT2046 Resistive Touch Controller |
| Touch SPI (MOSI/MISO/CLK) | 32 / 39 / 25 | Dedicated Touch SPI |
| PN532 I2C SDA / SCL (CN1) | 22 / 27 | External NFC module communication |
| RGB LED | 4 (R), 16 (G), 17 (B) | Access granted / denied indicator |
| Audio Buzzer | 26 | Card scan chime |
| Ambient Light (LDR) | 34 | Auto-dimming night mode |

---

## License
Dual Licensed:
- **Firmware Source Code**: [GNU Affero General Public License v3.0 (AGPL-3.0)](https://www.gnu.org/licenses/agpl-3.0.html)
- **Media, Documentation & 3D Assets**: [Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 (CC BY-NC-ND 4.0)](https://creativecommons.org/licenses/by-nc-nd/4.0/)
