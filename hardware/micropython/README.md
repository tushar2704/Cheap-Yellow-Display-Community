# 🐍 MicroPython for Cheap Yellow Display (CYD)

> Run pure Python on your ESP32 Cheap Yellow Display (ESP32-2432S028R).

---

## ⚡ Quickstart

1. Flash standard ESP32 MicroPython firmware using [Thonny IDE](https://thonny.org/) or `esptool.py`:
   ```bash
   esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
   esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 esp32-spiram-xxxx.bin
   ```
2. Open Thonny IDE, connect to your ESP32 COM port.
3. Open and run [`hello_cyd.py`](hello_cyd.py) to test backlight, RGB LED, ambient light sensor, and high-speed 40MHz SPI display communication!
