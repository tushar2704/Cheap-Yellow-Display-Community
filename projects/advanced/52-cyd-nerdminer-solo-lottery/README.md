# 52 — CYD NerdMiner Solo Bitcoin Lottery Miner

> ⭐⭐⭐⭐ **Advanced** · ESP32-2432S028R (Cheap Yellow Display 2.8")

A standalone **solo Bitcoin ASIC-style lottery miner** and network telemetry dashboard running directly on the ESP32 Cheap Yellow Display. Features hardware-accelerated dual-core SHA-256 double-hashing, real-time hashrate gauge, valid share counter, block height tracker, halving cycle countdown, and live Bitcoin jackpot prize tracking.

---

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

### 📊 Screen 0 — Miner Dashboard
- **Real-Time Solo Hashrate**: Dual-core hardware-accelerated SHA-256 engine with live KH/s gauge and total hashes calculated.
- **Valid Shares Counter**: Tracks accepted difficulty shares and records your personal best difficulty (`Best Diff`).
- **Current Block Height**: Real-time Bitcoin chain height.
- **Block Reward Jackpot**: Calculates the current block subsidy + fees in BTC and USD (e.g. 3.125 BTC = $270,000+).

### ⏳ Screen 1 — Network & Halving Deck
- **Bitcoin Spot Price**: Real-time BTC/USD price ticker.
- **Global Network Hashrate**: Worldwide exahash meter (EH/s).
- **Halving Cycle Progress**: Visual bar showing progress toward the next Bitcoin block halving with remaining blocks countdown.

### 💻 Screen 2 — Stratum Live Console
- Scrolling Matrix-green terminal log displaying:
  - Mining job template receipts
  - Block header nonces searched
  - Real-time share accepted notifications
  - Blinking terminal cursor

### 💡 RGB LED & Audio Alerts
- **Gold / Amber Pulse**: Mining heartbeat while actively hashing.
- **Matrix Green Flash**: Valid difficulty share accepted!
- **Coin Chime**: Vintage arcade coin audio sound on accepted shares via GPIO 26 buzzer.

---

## 🛠️ Hardware Requirements

- **ESP32-2432S028R** (Cheap Yellow Display 2.8" Resistive Touch)
- Micro-USB or USB-C power cable
- 3D-printed desktop miner stand by `@Makerrdesigns`

---

## 🚀 Quick Start Guide

1. Open `src/main.cpp` and enter your WiFi details and Bitcoin payout address:
   ```cpp
   const char* WIFI_SSID     = "YOUR_WIFI_SSID";
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
   const char* BTC_ADDRESS   = "bc1qxy2kgdygjrsqtzq2n0yrf2493p83kkfjhx0wlh";
   ```
   *(If WiFi is not connected, the miner runs autonomously in high-speed local solo benchmark mode!)*
2. Compile and upload via PlatformIO:
   ```bash
   python3 -m platformio run --target upload
   ```
3. Tap the screen at any time to cycle between Dashboard, Network Deck, and Stratum Terminal!

---

## 📋 Pinout Reference

| Function | Pin | Notes |
|---|---|---|
| TFT SPI MOSI / MISO / SCLK | GPIO 13 / 12 / 14 | Standard CYD display SPI |
| TFT CS / DC / BL | GPIO 15 / 2 / 21 | Backlight active HIGH |
| Touch CS / CLK / MISO / MOSI / IRQ | GPIO 33 / 25 / 39 / 32 / 36 | HSPI bus dedicated to XPT2046 |
| RGB LED (R / G / B) | GPIO 4 / 16 / 17 | Gold pulse on mining, green flash on share |
| Buzzer | GPIO 26 | Arcade coin audio chime |

---

## 📺 Credits

📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)

---

## 📄 License

MIT © Cheap Yellow Display Community

