# 📚 CYD Integrated IC Datasheets & Component Reference

Essential technical summaries, operating voltages, register maps, and pin descriptions for the core integrated circuits on the Cheap Yellow Display.

---

## 1. ILI9341 — 240x320 RGB TFT LCD Controller

- **Display Resolution**: 240 (RGB) × 320 dots, 262K colors (16-bit RGB565 / 18-bit RGB666).
- **Communication Protocol**: 4-line Serial Peripheral Interface (SPI).
- **Operating Voltage**: Logic `VDDI` = 1.65V to 3.3V; Analog `VCI` = 2.5V to 3.3V.
- **Maximum SPI Clock**: 40MHz write frequency, 16MHz read frequency.
- **Key Command Registers**:
  - `0x01`: Software Reset (`SWRESET`)
  - `0x11`: Sleep Out (`SLPOUT`)
  - `0x28` / `0x29`: Display OFF / Display ON
  - `0x2A`: Column Address Set (X range `0x0000` to `0x00EF`)
  - `0x2B`: Page Address Set (Y range `0x0000` to `0x013F`)
  - `0x2C`: Memory Write (`RAMWR`, streams pixel data)
  - `0x36`: Memory Access Control (`MADCTL`, controls screen orientation and RGB/BGR order)

---

## 2. XPT2046 — 4-Wire Resistive Touch Controller

- **ADC Resolution**: 12-bit SAR Analog-to-Digital Converter.
- **Interface**: High-speed SPI (up to 2.5MHz).
- **Supply Voltage**: 2.7V to 5.25V.
- **Internal Features**: Built-in 2.5V reference voltage, auxiliary analog input, and temperature sensor.
- **Command Bytes**:
  - `0xD0`: Measure X-Position (12-bit differential)
  - `0x90`: Measure Y-Position (12-bit differential)
  - `0xB0`: Measure Z1 Touch Pressure
  - `0xC0`: Measure Z2 Touch Pressure

---

## 3. CST816S / CST820 — Self-Capacitive Touch Screen Controller

- **Interface**: I²C Fast Mode (up to 400kHz).
- **7-Bit I²C Device Address**: `0x15` (`0x2A` write, `0x2B` read).
- **Touch Capabilities**: Single finger coordinate tracking with hardware-accelerated gesture recognition.
- **Gesture Register (`0x01`) Values**:
  - `0x00`: None / Stationary Touch
  - `0x01`: Swipe Up (`UP`)
  - `0x02`: Swipe Down (`DOWN`)
  - `0x03`: Swipe Left (`LEFT`)
  - `0x04`: Swipe Right (`RIGHT`)
  - `0x05`: Click / Tap (`CLICK`)
  - `0x0B`: Double Click (`DOUBLE_CLICK`)
  - `0x0C`: Long Press (`LONG_PRESS`)
- **Coordinate Registers**:
  - `0x03`: X High Byte (bits 3:0)
  - `0x04`: X Low Byte (bits 7:0)
  - `0x05`: Y High Byte (bits 3:0)
  - `0x06`: Y Low Byte (bits 7:0)

---

## 4. FM8002A — 2.0W Audio Power Amplifier

- **Amplifier Topology**: Class-AB Mono Bridge-Tied Load (BTL).
- **Operating Voltage Range**: 2.2V to 5.5V.
- **Power Output**: 2.0W @ 5.0V into 4Ω (10% THD+N); 1.5W @ 5.0V into 8Ω.
- **Low Shutdown Current**: 0.1μA typical.
- **Pop & Click Suppression**: Internal circuitry suppresses turn-on and turn-off transients.

---

## 5. ESP32-WROOM-32 — Dual-Core Wi-Fi & Bluetooth Module

- **Microcontroller**: Xtensa® dual-core 32-bit LX6 microprocessor (up to 240 MHz).
- **Internal Memory**: 448 KB ROM, 520 KB SRAM, 16 KB RTC SRAM.
- **External Flash**: 4MB SPI Flash (Quad-SPI).
- **Wireless Protocols**:
  - Wi-Fi: 802.11 b/g/n (up to 150 Mbps).
  - Bluetooth: Bluetooth v4.2 BR/EDR and BLE specification.
- **Operating Current**: 80mA average in active WiFi mode; peak current bursts up to 260mA during RF transmission.
