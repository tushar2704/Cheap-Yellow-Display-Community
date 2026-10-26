# 📌 ESP32-2432S028C (2.8" Capacitive) Pinout Reference

The **ESP32-2432S028C** is the **Capacitive Touch** variant of the 2.8" Cheap Yellow Display. It replaces the resistive overlay with a smooth **optical glass panel** driven by an **I²C CST816S / CST820 controller**.

---

## ⚡ Internal Subsystem Pin Mapping

| Subsystem | Signal Name | ESP32 GPIO | Description / Notes |
| :--- | :--- | :--- | :--- |
| **TFT Display (SPI)** | MOSI | **GPIO 13** | SPI Master Out Slave In |
| | MISO | **GPIO 12** | SPI Master In Slave Out |
| | SCK / CLK | **GPIO 14** | SPI Clock (up to 40MHz) |
| | CS | **GPIO 15** | Display Chip Select (Active LOW) |
| | DC / RS | **GPIO 2** | Data / Command selector |
| | RESET | Tied to EN / -1 | Display Reset |
| | Backlight (BL) | **GPIO 21** | PWM dimming (Active HIGH) |
| **Capacitive Touch (I²C)**| **Touch SDA** | **GPIO 33** | I²C Data (Fast Mode 400kHz, 7-bit Addr: 0x15) |
| | **Touch SCL** | **GPIO 32** | I²C Clock |
| | **Touch RST** | **GPIO 25** | Hardware Touch Reset line |
| | **Touch INT** | **GPIO 21** | Touch Wakeup Interrupt |
| **Onboard Audio** | Audio In | **GPIO 26** | Connected to FM8002A amp & speaker header |
| **Light Sensor (LDR)**| LDR Out | **GPIO 34** | Analog light level divider (ADC1_CH6, Input only)|
| **RGB LED** | Red | **GPIO 4** | Active LOW |
| | Green | **GPIO 16** | Active LOW |
| | Blue | **GPIO 17** | Active LOW |
| **MicroSD Card (HSPI)**| SD CS | **GPIO 5** | SD Card Chip Select |
| | SD MOSI | **GPIO 23** | HSPI MOSI |
| | SD MISO | **GPIO 19** | HSPI MISO |
| | SD SCK | **GPIO 18** | HSPI Clock |

---

## 🔍 Resistive vs Capacitive: Key Pinout Differences

| Feature | Resistive (`S028R`) | Capacitive (`S028C`) |
| :--- | :--- | :--- |
| **Touch Controller** | XPT2046 | CST816S / CST820 |
| **Bus Protocol** | Secondary SPI Bus | **I²C Bus** |
| **Touch Data Lines** | GPIO 32 (MOSI), 39 (MISO), 25 (CLK), 33 (CS) | **GPIO 33 (SDA), GPIO 32 (SCL)** |
| **Touch Interrupt** | GPIO 36 (IRQ) | **GPIO 21 (INT)** |
| **Hardware Gestures** | None | **Swipe L/R/U/D, Double-Tap, Long-Press** |
| **Glass Surface** | Flexible plastic film | **Toughened scratch-resistant glass** |
