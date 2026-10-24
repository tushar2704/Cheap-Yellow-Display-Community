# ⚡ Project 04 — CYD 2.8" Zigbee2MQTT & MQTT Touch Commander

> Transform your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a dedicated Zigbee2MQTT & standard MQTT touchscreen command center. Control Zigbee smart switches, smart lights, smart plugs, and relays via standard MQTT payloads (`zigbee2mqtt/[friendly_name]/set`). Features live telemetry readouts for power meters (Watts), mesh ambient temperatures, and Zigbee Link Quality Indicators (LQI). Pairs with the [3D Desktop Wedge Stand](../../../enclosures/cyd-2.8-desktop-stand/) by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 📋 Overview

- **Difficulty**: ⭐⭐⭐⭐ Advanced (Level 4/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch CYD)
- **Protocol**: MQTT / Zigbee2MQTT Standard JSON format
- **Libraries Used**: `TFT_eSPI`, `XPT2046_Touchscreen`, `PubSubClient`, `ArduinoJson`, `WiFi`, `SPI`

---

## 📺 Video Tutorials

- 🎬 **Watch the Zigbee2MQTT Setup & Demo**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Desktop Wedge Stand**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Direct MQTT Broker Integration**: Connects straight to Mosquitto, EMQX, or Home Assistant's embedded MQTT broker over WiFi.
- **4 Interactive Zigbee Device Control Tiles**:
  - 🔘 **Living Switch** (`zigbee2mqtt/living_switch`) — Emerald indicator.
  - 💡 **Kitchen Light** (`zigbee2mqtt/kitchen_light`) — Amber gold toggle.
  - 🔌 **Power Plug** (`zigbee2mqtt/power_plug`) — Cyan active socket state.
  - ⚡ **Garden Relay** (`zigbee2mqtt/garden_relay`) — Magenta high-current relay.
- **Live Mesh Telemetry Card**: Continuous monitoring of overall electrical draw (Watts), room temperature, and Zigbee Link Quality Index (LQI).
- **RGB LED Broker Diagnostics**:
  - 🔵 **Blue Flashing**: Connecting to MQTT broker.
  - 🟢 **Solid Green**: Connected & subscribed to topics.
  - 🟡 **Yellow Flash**: Incoming MQTT payload received.
  - 🔴 **Red**: Broker connection dropped.
- **Audible Piezo Confirmation**: Clean frequencies on touch interactions via GPIO 26 speaker.

---

## 🚀 How to Flash & Configure

### 1. Flash Directly in Your Browser (No IDE Required)
Open the [CYD Community Web Flasher](https://tushar2704.github.io/Cheap-Yellow-Display-Community/) in Chrome, Edge, or Opera, plug in your CYD 2.8" via USB, and click **Install Zigbee2MQTT Commander**.

### 2. Or Build & Flash via PlatformIO:
```bash
cd projects/advanced/04-zigbee-mqtt-commander
pio run -t upload
```

---

## 📜 License
Dual Licensed: GNU AGPLv3 (Code) & CC BY-NC-ND 4.0 (Assets) — see [LICENSE](../../../LICENSE) | Author: [@tushar2704](https://github.com/tushar2704)
