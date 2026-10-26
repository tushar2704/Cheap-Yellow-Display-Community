# 🐍 MicroPython Suite for Cheap Yellow Display (CYD)

Run pure Python on your ESP32 Cheap Yellow Display (`ESP32-2432S028R` & `ESP32-2432S028C`).

---

## 📁 Included Python Modules & Scripts

| File | Purpose |
| :--- | :--- |
| **[`cyd.py`](cyd.py)** | Core helper module wrapping onboard RGB LED (active-LOW PWM), backlight dimming, audio beeper, and LDR lux sensor. |
| **[`hello_cyd.py`](hello_cyd.py)** | SPI display diagnostic, RGB light cycle, and light meter test. |
| **[`wifi_scanner.py`](wifi_scanner.py)** | 2.4 GHz wireless spectrum auditor printing SSID, RSSI (dBm), and channels. |

---

## ⚡ Getting Started with Thonny IDE

1. Flash standard ESP32 MicroPython firmware using [Thonny IDE](https://thonny.org/) or `esptool.py`:
   ```bash
   esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
   esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 esp32-20230426-v1.20.0.bin
   ```
2. Open **Thonny IDE**, set interpreter to **MicroPython (ESP32)** and select your USB COM port.
3. Upload `cyd.py` to the ESP32 root directory.
4. Run `wifi_scanner.py` or `hello_cyd.py` to interact with your board in real time!
