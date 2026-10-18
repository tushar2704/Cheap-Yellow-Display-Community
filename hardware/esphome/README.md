# 🏡 ESPHome for Cheap Yellow Display (CYD)

> Turn your Cheap Yellow Display into a native Home Assistant touch panel in minutes using official ESPHome firmware.

---

## ⚡ Features Out of the Box

- 🖥️ **ILI9341 320x240 Display**: Zero-flicker rendering with Google Fonts integration.
- 👆 **XPT2046 Resistive Touchscreen**: Fully calibrated coordinate transforms (`swap_xy: true`, `mirror_y: true`).
- 💡 **Backlight Brightness Entity**: Monochromatic dimmable light slider directly inside Home Assistant.
- 🌈 **RGB Status LED**: Native Home Assistant RGB light entity with active-low inversion handled.
- ☀️ **Ambient Light Sensor**: 0–100% room illumination sensor entity.

---

## 🚀 How to Flash with ESPHome Dashboard

1. Open your Home Assistant **ESPHome Dashboard**.
2. Click **+ NEW DEVICE** and name it `cyd-28-smart-panel`.
3. Choose **ESP32**.
4. Replace the default YAML with the contents of [`cyd-2.8-display.yaml`](cyd-2.8-display.yaml).
5. Click **Install ➔ Plug into this computer** (or download the compiled `.bin` and flash via our [Web Flasher](../../flasher/)).
6. Once online, Home Assistant will automatically discover the CYD as a new device!
