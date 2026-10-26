# 🏠 ESPHome Ready-to-Flash Packages for CYD 2.8"

Plug-and-play **ESPHome YAML configurations** designed specifically for the Cheap Yellow Display (`ESP32-2432S028R`), enabling native, direct integration into **Home Assistant** without writing C++ code.

---

## 📦 Available Configurations

| File | Application | Description |
| :--- | :--- | :--- |
| **[`cyd-2.8-display.yaml`](cyd-2.8-display.yaml)** | Base Hardware Template | Full hardware initialization: ILI9341 display, XPT2046 touch, PWM backlight, RGB LED, and LDR light sensor. |
| **[`cyd-thermostat-hvac.yaml`](cyd-thermostat-hvac.yaml)** | Smart HVAC Thermostat | Full-screen thermostat controller with target temperature setpoint up/down touch buttons, current room temperature, and auto mode status. |
| **[`cyd-media-player-companion.yaml`](cyd-media-player-companion.yaml)** | Media Player Desktop Remote | Home Assistant media player remote with song title, artist, progress bar, and touch controls (Prev, Play/Pause, Next). |
| **[`cyd-ble-proxy-environment.yaml`](cyd-ble-proxy-environment.yaml)** | Active BLE Tracker Proxy | Turns the CYD into an active Bluetooth proxy relaying room iBeacons, RuuviTags, and wearables to Home Assistant while reporting WiFi RSSI and ambient lux. |

---

## ⚡ How to Flash via ESPHome Dashboard

1. Open your **Home Assistant** instance and navigate to **ESPHome Dashboard**.
2. Click **+ New Device**, name your node, and choose **ESP32**.
3. Copy the contents of any YAML configuration above into your ESPHome editor.
4. Ensure your `secrets.yaml` contains:
   ```yaml
   wifi_ssid: "Your_WiFi_SSID"
   wifi_password: "Your_WiFi_Password"
   api_encryption_key: "Generated_Key_Here"
   ```
5. Connect your CYD via USB to your computer and click **Install → Manual Download / Web Serial**.
6. Your CYD will boot up and automatically appear under **Home Assistant Integrations**!
