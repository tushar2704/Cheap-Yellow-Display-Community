# 🪙 Project 02 — CYD Live Cryptocurrency Price Tracker & Sparkline

> A real-time crypto ticker for your Cheap Yellow Display (CYD) tracking Bitcoin (BTC), Ethereum (ETH), and Solana (SOL) with 24-hour percentage gains/losses, sparklines, and touch tab switching via CoinGecko's free public API.

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐ Intermediate (Level 3/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **External Components Required**: **None** (100% onboard ESP32 WiFi & TFT)
- **Libraries Used**: `WiFi`, `HTTPClient`, `ArduinoJson`, `TFT_eSPI`, `XPT2046_Touchscreen`

---

## 📺 Video Tutorials

- 🎬 **Watch the Full Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Desktop Crypto Terminal Case**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Real-Time Prices**: Live spot USD prices for Bitcoin, Ethereum, and Solana.
- **Color-Coded 24h Change**: Vibrant green badge for upward momentum, red badge for market dips.
- **Touch Tab Switching**: Tap the top tabs or anywhere on the screen to switch coins with immediate tactile audio feedback.
- **Zero API Key Needed**: Uses public CoinGecko endpoints without account creation or API keys.

---

## 💰 Premium Upgrade Edition (Gumroad)

Looking for more coins, stock tickers, or portfolio tracking? Check out the **CYD FinDesk Pro Edition** on Gumroad ($6.00):
- 📈 Multi-Asset Portfolio Tracker: Enter your holdings and see your total net worth update live.
- 💱 50+ Cryptos & US Stock Market support via Finnhub & Binance WebSockets.
- 📉 Real candlestick charts (1H, 24H, 7D history) stored in SPIFFS flash.
- 🖨️ Complete 3D enclosure with anti-slip rubber footings designed by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 🚀 How to Flash & Run

### 1. Configure WiFi Credentials
In `02-crypto-tracker.ino`:
```cpp
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

### 2. Upload via PlatformIO:
```bash
cd projects/intermediate/02-crypto-tracker
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)

