# 🔌 Hardware Add-ons & Peripheral Expansion — Cheap Yellow Display (CYD)

> A complete practical guide to connecting external sensors, displays, rotary encoders, buttons, and actuators to your ESP32 Cheap Yellow Display.

---

## 🧭 Expansion Connectors Overview

Most CYD boards (especially the common 2.8" ESP32-2432S028R) have two or three JST 1.25mm connectors on the rear:

```
      ┌───────────────────────────────┐
      │         MicroSD Slot          │
      │                               │
      │   [ CN1 ]           [ P3 ]    │
      │  (4-pin JST)      (4-pin JST) │
      │                               │
      │         [ Micro-USB ]         │
      │         [ Type-C    ]         │
      └───────────────────────────────┘
```

> [!NOTE]
> The connector type is **JST-GH 1.25mm pitch 4-pin** (commonly found in drone accessories and micro RC). Many sellers include one or two pigtail cables in the CYD box.

---

## 📌 Connector Pinout Details

### CN1 Connector (I2C / Power)
| Pin # | Label | ESP32 GPIO | Function | Voltage / Max Current |
|:---:|:---:|:---:|:---|:---|
| 1 | GND | GND | System Ground | 0V |
| 2 | SCL | GPIO 22 | I2C SCL (Hardware I2C) | 3.3V Logic |
| 3 | SDA / IO | GPIO 27 | General I/O or Touch RST / PWM | 3.3V Logic |
| 4 | 3V3 / VCC | 3V3 | Power rail from onboard LDO | 3.3V (~300mA available) |

### P3 Connector (Extended IO)
| Pin # | Label | ESP32 GPIO | Function | Notes |
|:---:|:---:|:---:|:---|:---|
| 1 | GND | GND | System Ground | 0V |
| 2 | ADC | GPIO 35 | Analog Input **ONLY** | Input-only! No internal pullup/pulldown |
| 3 | IO | GPIO 22 | Shared with CN1 SCL | Can be used as general IO or I2C |
| 4 | IO | GPIO 27 | Shared with CN1 SDA | Can be used as general IO |

### P1 / Speaker Connector (2-Pin JST)
| Pin # | Label | ESP32 GPIO | Function |
|:---:|:---:|:---:|:---|
| 1 | SPK+ | GPIO 26 | Driven by onboard NS4168 / PAM8302 audio amp (DAC Channel 2) |
| 2 | SPK- | GND | Audio ground |

---

## 🛠️ Popular Add-on Modules & Wiring Guides

### 1. I2C Environmental Sensors (BME280 / SHT31 / AHT20 / BMP280)
The best way to add temperature, humidity, and barometric pressure is via CN1 using hardware I2C:

```
CYD (CN1)          Sensor (BME280)
Pin 1 (GND)   ───► GND
Pin 2 (GPIO22)───► SCL
Pin 3 (GPIO27)───► SDA (Wire.begin(27, 22))
Pin 4 (3.3V)  ───► VCC / VIN
```

**Arduino Initialization:**
```cpp
#include <Wire.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

void setup() {
  // Initialize I2C with SDA = 27, SCL = 22
  Wire.begin(27, 22);
  
  if (!bme.begin(0x76, &Wire)) {
    Serial.println("Could not find a valid BME280 sensor!");
  }
}
```

---

### 2. Rotary Encoder with Push Button (EC11)
A rotary encoder makes menu navigation smooth without touching the screen:

```
CYD Pin               Rotary Encoder (EC11)
CN1 Pin 1 (GND)   ───► GND (Common ground)
CN1 Pin 2 (GPIO22)───► CLK (Encoder Pin A)
CN1 Pin 3 (GPIO27)───► DT  (Encoder Pin B)
P3  Pin 2 (GPIO35)───► SW  (Pushbutton switch, with external 10kΩ pull-up to 3.3V)
```

> [!WARNING]
> **GPIO 35 is INPUT ONLY** and does not have an internal pull-up resistor. You **must** connect a 10kΩ external pull-up resistor between GPIO 35 and 3.3V.

---

### 3. External Speaker / Buzzer
The onboard 2-pin JST speaker port is connected to an onboard Class-D audio amplifier driven by **GPIO 26**.
- **Speaker specs**: 8Ω 1W or 4Ω 1W miniature cavity speaker.
- **Tone Generation**: Use standard Arduino `tone(26, frequency, duration)` or ESP32 `ledcWriteTone` for simple beeps and chimes.
- **WAV / MP3 Playback**: Use the `ESP32-audioI2S` library or 8-bit DAC output routines to stream audio from SPIFFS or SD card.

---

### 4. DS18B20 1-Wire Temperature Probe
- Signal Pin ──► **GPIO 27**
- Requires a 4.7kΩ pull-up resistor between GPIO 27 and 3.3V.
- Excellent for outdoor waterproof sensing, aquarium monitoring, or beer fermentation chambers.

---

### 5. Ultrasonic Distance Sensor (RCWL-1601 / HC-SR04P)
Use the 3.3V-compatible version (HC-SR04**P** or RCWL-1601):
- `TRIG` ──► **GPIO 27** (Output)
- `ECHO` ──► **GPIO 35** (Input-only, perfect for echo return!)
- `VCC`  ──► **3.3V**
- `GND`  ──► **GND**

---

## 📺 Recommended Community Channels
- 🎬 Project Builds & Humor: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ 3D Enclosures & Sensor Brackets: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)
