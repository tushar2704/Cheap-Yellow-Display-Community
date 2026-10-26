# ⚡ Cheap Yellow Display (CYD) Circuit Schematics & Hardware Guide

This technical guide breaks down the circuit topology, power management, amplifier stages, and sensor dividers of the **ESP32-2432S028** hardware platform.

---

## 1. Power Supply & Voltage Regulation

```
USB 5V (VBUS) ──┬──► [Schottky Diode SS14 / B5819WS] ──► +5V Rail ──► [CN1/P3 Pin 2]
                │
                └──► [AMS1117-3.3 / ME6211 LDO Regulator] 
                            │
                            ├──► 3.3V Rail (ESP32 VDD, Flash, ILI9341, CST816)
                            └──► Decoupling Caps (10uF Tantalum + 100nF Ceramic)
```

- **LDO Input Limits**: 4.75V to 6.0V max (safe operation at standard 5V USB).
- **Current Capability**:
  - The stock 3.3V LDO delivers up to **800mA** (AMS1117) or **500mA** (ME6211).
  - ESP32 WiFi TX peak bursts draw ~240mA; display backlight draws ~65mA.
  - Adding heavy external sensors to CN1/P3 should not exceed 150mA on the 3.3V rail.

---

## 2. Onboard Audio Stage (FM8002A Class-AB Amplifier)

```
ESP32 GPIO 26 ──► [1uF DC-Blocking Cap] ──► [10kΩ Resistor] ──► [FM8002A Amp Pin 4 (IN-)]
                                                                       │
                                                       Speaker Header ◄┴──► [FM8002A VO1/VO2]
```

- **Amplifier IC**: FM8002A / 8002B mono bridge-tied load (BTL) audio power amp.
- **Output Power**: Up to **2.0W into a 4Ω load** or **1.5W into 8Ω** at 5V.
- **Speaker Header**: 2-pin 1.25mm JST connector labeled `SPEAKER`.
- **Operating Note**: GPIO 26 is also connected to DAC Channel 2, allowing true 8-bit analog audio output as well as PWM square-wave beeps.

---

## 3. LCD Backlight Switching Circuit

```
ESP32 GPIO 21 ──► [1kΩ Base Resistor] ──► [NPN Transistor S8050 / J3Y]
                                                   │
                                     LED Cathode ◄─┘ (Collector)
                                     LED Anode   ◄─── +3.3V Rail (Emitter to GND)
```

- **Backlight Control**: Active-HIGH. Driving GPIO 21 HIGH turns the backlight ON.
- **PWM Dimming**: Using `ledcSetup(0, 5000, 8)` allows smooth 0–255 brightness dimming.

---

## 4. Ambient Light Sensor Divider (LDR)

```
+3.3V Rail ──► [GL5528 Photoresistor (LDR)] ──┬──► ESP32 GPIO 34 (ADC1_CH6)
                                              │
                                       [10kΩ Resistor]
                                              │
                                             GND
```

- **Sensor Type**: GL5528 light-dependent resistor.
- **Light Response**:
  - Bright light: LDR resistance drops to ~2–5kΩ → GPIO 34 voltage rises toward 2.5V (ADC ~3200–4095).
  - Dark room: LDR resistance rises to ~500kΩ–1MΩ → GPIO 34 voltage drops toward 0.1V (ADC ~50–300).
- **Safety Note**: GPIO 34 is an **input-only** pin with no internal pull-up/pull-down resistors.
