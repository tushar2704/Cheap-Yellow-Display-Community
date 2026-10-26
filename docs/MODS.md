# 🛠️ CYD Hardware Mods & Fixes — Cheap Yellow Display

> Proven hardware modifications, solder hacks, and fixes to supercharge your ESP32 Cheap Yellow Display.

---

## ⚡ 1. The USB-C to USB-C Cable Fix (CC Resistor Mod)

### The Problem
Many cheap CYD boards with USB-C ports only power on with an old-school **USB-A to USB-C cable**. If you plug them into a modern MacBook, laptop, or PD (Power Delivery) charger with a **C-to-C cable**, the board refuses to power up at all.

### Why This Happens
The manufacturer omitted two inexpensive 5.1kΩ pulldown resistors on the USB-C `CC1` and `CC2` lines. Without these resistors, USB-C Power Delivery source controllers cannot detect a connected sink device, so the 5V bus line remains turned off (0V).

### The Fix
1. Locate the USB-C connector pads on the bottom layer.
2. Solder a **5.1kΩ (0402 or 0603)** SMD resistor (or through-hole wire) from the `CC1` pin to Ground.
3. Solder a second **5.1kΩ** resistor from the `CC2` pin to Ground.
4. *Result*: Your CYD now powers flawlessly from any modern USB-C PD charger or PC!

---

## ☀️ 2. LDR Light Sensor & Auto-Brightness

### Overview
Most 2.8" CYDs come with an onboard Light Dependent Resistor (photoresistor) wired as a voltage divider into **GPIO 34**.

### Software Auto-Brightness
You can dynamically adjust the screen backlight via PWM based on ambient light:

```cpp
#define TFT_BL 21
#define LDR_PIN 34

void setupAutoBrightness() {
  ledcAttach(TFT_BL, 5000, 8); // 5 kHz PWM, 8-bit resolution (0-255)
}

void loopAutoBrightness() {
  int lightLevel = analogRead(LDR_PIN); // 0 (dark) to 4095 (bright)
  
  // Map ambient light to pleasant backlight brightness (30 to 255)
  int brightness = map(lightLevel, 0, 4095, 30, 255);
  ledcWrite(TFT_BL, constrain(brightness, 30, 255));
  
  delay(100);
}
```

---

## 🔊 3. Audio Amplifier Noise & Buzzing Fix

### The Problem
When powering the CYD from standard USB or running WiFi transmissions, the onboard speaker can emit high-pitched whining, buzzing, or ticking noises.

### Root Cause
1. ESP32 RF bursts cause voltage dips on the 3.3V/5V rail.
2. The onboard single-ended audio amplifier shares the ground plane with high-frequency digital switching circuits.

### Proven Fixes
1. **Bulk Decoupling Capacitor**: Solder a 100µF – 220µF low-ESR electrolytic or ceramic capacitor across the 5V and GND pins near the USB connector.
2. **Backlight PWM Frequency**: Switch backlight PWM frequency to **20 kHz+** (above human hearing). Lower frequencies (e.g. 1 kHz) induce audible inductive coil whine in the trace routing.
3. **Mute Pin Mod**: When not playing sound, write `dacWrite(26, 0);` or disconnect the DAC channel to eliminate idle hiss.

---

## 💾 4. MicroSD Card SPI Bus Optimization

### The Architecture
On standard 2.8" CYD (ESP32-2432S028R):
- **Display & Touch** share the **VSPI** bus (GPIO 12, 13, 14).
- **MicroSD Card** is wired to the **HSPI** bus:
  - `SD_SCK`: GPIO 18
  - `SD_MISO`: GPIO 19
  - `SD_MOSI`: GPIO 23
  - `SD_CS`: GPIO 5

### Preventing SPI Bus Starvation
Because the SD card is on a completely independent SPI peripheral (HSPI), you can achieve maximum read/write throughput without interfering with the 40MHz display refresh rate!

**Proper Arduino Initialization:**
```cpp
#include <SPI.h>
#include <SD.h>

SPIClass sdSPI(HSPI);

void setupSD() {
  sdSPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS
  if (!SD.begin(5, sdSPI, 40000000)) { // Run up to 40MHz SPI
    Serial.println("SD Card initialization failed!");
  } else {
    Serial.println("SD Card initialized at 40MHz!");
  }
}
```

---

## 🔋 5. Running CYD on LiPo Battery Power

### Recommended Hardware
- 3.7V LiPo Battery (1000mAh – 2500mAh)
- TP4056 USB-C Charging Board with Protection
- Low-dropout 3.3V regulator (e.g. AP2112K) or 5V Boost Converter (e.g. MT3608)

### Connection
Connect the 5V output of your boost converter to the **5V** and **GND** pads of the CYD (via the P3 connector).

### Battery Voltage Sensing (ADC Gauge)
To monitor remaining battery percentage, build a 2-resistor voltage divider (two 100kΩ resistors) from the LiPo positive terminal (+3.0V to 4.2V) into **GPIO 35** (available on connector P3):
```cpp
#define BATTERY_PIN 35

float getBatteryVoltage() {
  int raw = analogRead(BATTERY_PIN);
  // 2:1 divider factor: Vbat = Vpin * 2
  float pinVoltage = (raw / 4095.0) * 3.3;
  return pinVoltage * 2.0;
}

int getBatteryPercent() {
  float v = getBatteryVoltage();
  return constrain(map((int)(v * 100), 320, 420, 0, 100), 0, 100);
}
```

---

## 🎛️ 6. Rotary Encoder (EC11) Physical Knob Mod

### The Goal
Add physical tactile menu scrolling, audio volume control, or thermostat adjustments using a standard EC11 rotary encoder with integrated push-button.

### Wiring (to CN1 Connector)
| Rotary Encoder Pin | CYD CN1 Pin | Description |
| :--- | :--- | :--- |
| **GND** | Pin 1 (GND) | Ground |
| **+ (VCC)** | Pin 2 (3.3V) | 3.3V Power |
| **CLK (A)** | Pin 3 (GPIO 22) | Encoder Channel A |
| **DT (B)** | Pin 4 (GPIO 27) | Encoder Channel B |
| **SW (Button)** | P3 Pin 4 (GPIO 35) | Push button input (with pull-up) |

### Code Implementation
```cpp
#define ENCODER_CLK 22
#define ENCODER_DT  27
#define ENCODER_SW  35

volatile int encoderPos = 0;
int lastClk = HIGH;

void IRAM_ATTR readEncoder() {
  int clk = digitalRead(ENCODER_CLK);
  int dt = digitalRead(ENCODER_DT);
  if (clk != lastClk) {
    if (dt != clk) encoderPos++;
    else encoderPos--;
    lastClk = clk;
  }
}

void setupEncoder() {
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), readEncoder, CHANGE);
}
```

---

## 🎵 7. Hi-Fi I2S Audio Mod (MAX98357A DAC Amplifier)

### The Problem
The onboard transistor amplifier on GPIO 26 produces quiet mono audio with slight background hiss during WiFi activity.

### The Upgrade
Wire a $2 **MAX98357A I2S 3W Class-D amplifier** module to external pins:
- **LRC (Word Select)**: GPIO 22 (CN1)
- **BCLK (Bit Clock)**: GPIO 27 (CN1)
- **DIN (Data In)**: GPIO 26 (P3 / onboard trace)
- **VIN & GND**: 5V and GND (P3)
- Connect a 4Ω or 8Ω 3W speaker directly to the MAX98357A screw terminals for rich, room-filling sound on Internet Radio, MP3 playback, and voice assistant responses!

---

## 📺 Community Tutorials
- 🎬 In-depth hardware tear-downs and mods: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ Custom battery enclosures & modular snap cases: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)

