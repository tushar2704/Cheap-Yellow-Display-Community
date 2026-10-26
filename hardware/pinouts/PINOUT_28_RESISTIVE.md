# 📌 ESP32-2432S028R (2.8" Resistive) Pinout Reference

The **ESP32-2432S028R** is the most widespread Cheap Yellow Display (CYD). It combines an ESP32-WROOM-32 module with a 2.8" 320x240 ILI9341 display and an XPT2046 resistive touch controller.

---

## ⚡ Internal Subsystem Pin Mapping

| Subsystem | Signal Name | ESP32 GPIO | Description / Notes |
| :--- | :--- | :--- | :--- |
| **TFT Display (SPI)** | MOSI | **GPIO 13** | SPI Master Out Slave In |
| | MISO | **GPIO 12** | SPI Master In Slave Out |
| | SCK / CLK | **GPIO 14** | SPI Clock (up to 40MHz) |
| | CS | **GPIO 15** | Display Chip Select (Active LOW) |
| | DC / RS | **GPIO 2** | Data / Command selector |
| | RESET | Tied to EN / -1 | Display Reset (handled by system reset) |
| | Backlight (BL) | **GPIO 21** | PWM dimming via NPN transistor (Active HIGH) |
| **Resistive Touch (SPI)**| Touch CS | **GPIO 33** | Touch Chip Select |
| | Touch IRQ | **GPIO 36** | Touch Interrupt (Input only, ADC1_CH0) |
| | Touch CLK | **GPIO 25** | Touch SPI Clock |
| | Touch MISO | **GPIO 39** | Touch SPI MISO (Input only) |
| | Touch MOSI | **GPIO 32** | Touch SPI MOSI |
| **Onboard Audio** | Audio In | **GPIO 26** | Connected to FM8002A amp & speaker header |
| **Light Sensor (LDR)**| LDR Out | **GPIO 34** | Analog light level divider (ADC1_CH6, Input only)|
| **RGB LED** | Red | **GPIO 4** | Active LOW (Drive LOW or PWM to illuminate) |
| | Green | **GPIO 16** | Active LOW |
| | Blue | **GPIO 17** | Active LOW |
| **MicroSD Card (HSPI)**| SD CS | **GPIO 5** | SD Card Chip Select |
| | SD MOSI | **GPIO 23** | HSPI MOSI |
| | SD MISO | **GPIO 19** | HSPI MISO |
| | SD SCK | **GPIO 18** | HSPI Clock |

---

## 🔌 External Expansion Connectors (CN1 & P3)

The board provides two 4-pin 1.25mm JST connectors for external sensors and peripherals:

### CN1 Connector (Right Side / Near USB)
| Pin # | Pin Label | ESP32 GPIO | Functionality |
| :---: | :---: | :---: | :--- |
| **1** | GND | GND | Ground |
| **2** | 3.3V | 3.3V | 3.3V Power Output (from LDO) |
| **3** | IO22 | **GPIO 22** | Free Digital I/O, I²C SCL, or PWM |
| **4** | IO27 | **GPIO 27** | Free Digital I/O, I²C SDA, or DAC |

### P3 Connector (Bottom Side / Near Audio)
| Pin # | Pin Label | ESP32 GPIO | Functionality |
| :---: | :---: | :---: | :--- |
| **1** | GND | GND | Ground |
| **2** | 5V | 5V / VBUS | 5V USB Power Input/Output |
| **3** | TX | **GPIO 1** | UART0 Transmit (Serial Debug) |
| **4** | RX / IO35| **GPIO 35** | Input-only ADC pin (ideal for Battery voltage divider) |
