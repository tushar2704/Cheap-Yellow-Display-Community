# 🚀 Elecrow 2.8" CYD — 50 Touch Projects Directory

> **Official Project Suite & Directory**  
> **Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Hardware Target**: Elecrow CrowPanel 2.8" / ESP32-2432S028R (ILI9341 320x240 TFT + XPT2046 Resistive Touch)  
> **Repository Directory**: `elecrow_projects/`  

---

## 📌 1. Hardware Architecture & Pinout Standard

All 50 projects in this collection are specifically configured, calibrated, and optimized for the **Elecrow 2.8" CYD (ESP32-2432S028R / CrowPanel 2.8")**:

| Component | Controller / Chip | GPIO Pin / Interface | Notes & Characteristics |
|---|---|---|---|
| **MCU** | ESP32-WROOM-32E | Dual-Core Xtensa LX6 @ 240MHz | 520 KB SRAM, 4MB Flash |
| **Display Panel** | ILI9341 (320x240 QVGA) | SPI (MOSI: IO13, MISO: IO12, SCK: IO14, CS: IO15, DC: IO2) | High refresh rate via DMA |
| **Backlight** | Backlight Transistor | GPIO 21 (PWM dimmable) | Active HIGH |
| **Touch Controller** | XPT2046 Resistive Touch | SPI (MOSI: IO32, MISO: IO39, SCK: IO25, CS: IO33, IRQ: IO36) | Precision stylus & glove friendly |
| **RGB LED** | Onboard Tricolor LED | Red: IO4, Green: IO16, Blue: IO17 | Active-LOW logic |
| **Photoresistor (LDR)**| Ambient Light Sensor | GPIO 34 (ADC1_CH6) | Auto-brightness & night detection |
| **Audio Output** | Onboard Amp + SPK header | GPIO 26 (DAC2 / Tone / PWM) | 1.25mm 2-pin JST connector |
| **MicroSD Card** | TF Slot (SPI Mode) | CS: IO5, MOSI: IO23, MISO: IO19, SCK: IO18 | Up to 32GB FAT32 |
| **I2C Expansion** | HY2.0-4P (Crowtail) | SDA: IO27, SCL: IO22, 3.3V, GND | Plug-and-play sensor bus |
| **UART / GPIO Port** | HY2.0-4P (Crowtail) | TX: IO1 (or IO22), RX: IO3 (or IO27) | GPS, Serial Bridge, IR transceiver |
| **Analog In / Extra** | 2-Pin Expansion Header | GPIO 35 (Input Only) | ADC probe / external sensor |

---

## 📁 2. Production Directory Structure

Every project in this suite is self-contained with its own verified code, configuration, documentation, and test reports:

```
elecrow_projects/
├── {NN}-{project-slug}/
│   ├── README.md               # Project overview, features, and wiring
│   ├── platformio.ini          # Standalone PlatformIO config for ESP32-2432S028R
│   ├── {NN}-{project-slug}.ino # Full production Arduino / C++ source code
│   └── TEST_REPORT.md          # Compilation metrics and hardware validation
```

---

## 📑 3. The 50 Projects Catalog

### Cluster 1: Offline IT, SysAdmin, Homelab & Developer Appliances (Projects 01–08)

| # | Project Name | Directory | Status | Description |
|---|---|---|---|---|
| **01** | **DevOps Command Vault** | [`01-devops-command-vault`](./01-devops-command-vault) | ✅ Complete | Interactive Docker CLI builder, Git rescue cheat sheet, Linux signals lookup, and DNS reference with direct USB serial output. |
| **02** | **Chmod & VLSM Architect** | [`02-chmod-vlsm-architect`](./02-chmod-vlsm-architect) | ✅ Complete | Visual CIDR /24 to /30 subnet splitter with IP calculation and interactive 9-box octal permission matrix. |
| **03** | **Homelab WoL Touch Deck** | [`03-homelab-wol-deck`](./03-homelab-wol-deck) | ✅ Complete | 102-byte UDP Magic Packet broadcaster with 4-digit security PIN lock and multi-node server lifecycle tracking. |
| **04** | **USB PC Telemetry & Macro Deck** | [`04-usb-pc-telemetry-deck`](./04-usb-pc-telemetry-deck) | ✅ Complete | Live PC CPU/GPU/VRAM bar gauges and 6-tile touch macro controller via CH340 serial bridge. |
| **05** | **Local AI Desk Terminal** | [`05-local-ai-desk-terminal`](./05-local-ai-desk-terminal) | ✅ Complete | Local-first private Ollama LLM client streaming tokens directly to an on-screen terminal window. |
| **06** | **Ad-Blocking DNS Sinkhole** | [`06-dns-sinkhole-monitor`](./06-dns-sinkhole-monitor) | ✅ Complete | Standalone UDP port 53 DNS sinkhole server with 5-minute temporary snooze and live traffic metrics. |
| **07** | **Bambu Dual-Printer Monitor** | [`07-bambu-telemetry-monitor`](./07-bambu-telemetry-monitor) | ✅ Complete | MQTT 3D printer fleet monitor with live nozzle/bed temperatures, layer progress, and spool cost calculation. |
| **08** | **Klipper Command Deck** | [`08-klipper-command-deck`](./08-klipper-command-deck) | ✅ Complete | Moonraker REST control panel with babystepping Z-offset, preheat macros, E-stop, and DS18B20 chamber sensor. |

---

### Cluster 2: Live Aviation, Space, Ham Radio & RF Telemetry (Projects 09–18)

| # | Project Name | Directory | Status | Description |
|---|---|---|---|---|
| **09** | **ATC Rotating Sweep Flight Radar** | [`09-atc-flight-radar`](./09-atc-flight-radar) | ✅ Complete | Dual-core radar with CRT sweep display, altitude filters, and emergency squawk proximity audio chirps. |
| **10** | **Sky-Spotter Flight Deck** | [`10-sky-spotter-station`](./10-sky-spotter-station) | ✅ Complete | Airline departures, arrivals, and spotter elevation/compass line calculation for overhead aircraft. |
| **11** | **Orbital Rocket & ISS Tracker** | [`11-iss-rocket-tracker`](./11-iss-rocket-tracker) | ✅ Complete | Local SGP4 orbital propagator calculating visible ISS flyovers and real-time rocket launch countdowns. |
| **12** | **Pilot's Desk METAR/TAF Decoder** | [`12-metar-weather-decoder`](./12-metar-weather-decoder) | ✅ Complete | Plain-English aviation weather decoder with dynamic crosswind component calculations for favorite runways. |
| **13** | **HamClock Mini Dashboard** | [`13-hamclock-mini`](./13-hamclock-mini) | ✅ Complete | Solar Flux Index (SFI), geomagnetic K-index gauge, and POTA/SOTA live spotter list with Maidenhead calculations. |
| **14** | **Minimalist Vector Flight Tracker** | [`14-vector-flight-tracker`](./14-vector-flight-tracker) | ✅ Complete | Ultra-high contrast distant-readability card displaying nearest overhead flight number, altitude, and path. |
| **15** | **GPS Stratum Triple Clock** | [`15-gps-stratum-clock`](./15-gps-stratum-clock) | ✅ Complete | 100% offline atomic clock using UART GPS receiver showing Local, UTC/Zulu, and satellite constellation SNR. |
| **16** | **Live Coastal Tide Chart** | [`16-coastal-tide-chart`](./16-coastal-tide-chart) | ✅ Complete | Continuous harmonic sine-wave tidal curve with golden hour sunrise/sunset markers and lunar illumination. |
| **17** | **Offline OSM Trail Navigator** | [`17-offline-osm-navigator`](./17-offline-osm-navigator) | ✅ Complete | Glove-friendly resistive touch map viewer rendering offline 256x256 OSM tiles from MicroSD with GPX tracking. |
| **18** | **ESP-NOW Marine Instrument Repeater** | [`18-espnow-marine-repeater`](./18-espnow-marine-repeater) | ✅ Complete | Router-free wireless receiver listening to outdoor battery sensor nodes for wind speed, polar rose, and barograph. |

---

### Cluster 3: Cybersecurity Auditing, RF Diagnostics & Bench Instruments (Projects 19–26)

| # | Project Name | Directory | Status | Description |
|---|---|---|---|---|
| **19** | **Defensive Wi-Fi Channel Auditor** | [`19-wifi-channel-auditor`](./19-wifi-channel-auditor) | ✅ Complete | 2.4GHz spectrum analyzer visualizing 802.11 channels 1-14 overlap curves with channel recommendation. |
| **20** | **Privacy Shield BLE Tracker Detector** | [`20-privacy-shield-detector`](./20-privacy-shield-detector) | ✅ Complete | Passive BLE advertisement analyzer identifying Apple FindMy, Tile, and SmartTag devices by proximity RSSI. |
| **21** | **Retro CRT Phosphor Oscilloscope** | [`21-crt-phosphor-scope`](./21-crt-phosphor-scope) | ✅ Complete | Analog ADC DMA waveform visualizer with simulated phosphor decay and touch-adjustable PWM generator. |
| **22** | **CYD Hardware Diagnostic Suite** | Interactive hardware bench testing LDR curve, RGB LEDs, speaker sweeps, MicroSD benchmarks, and touch calibration. |
| **23** | **Multi-Firmware SD Bootloader** | Graphical touch launcher booting pre-compiled `.bin` files from MicroSD via OTA partition swapping. |
| **24** | **Pocket RF & IR Multi-Tool** | Infrared transceiver cloner and raw pulse analyzer with dynamic Wi-Fi QR credential generation. |
| **25** | **Wi-Fi Deauth Defense Sentinel** | Promiscuous 802.11 management frame monitor alerting on camera deauthentication floods and attacks. |
| **26** | **Wearable Conference Badge** | Battery-powered touch badge with dynamic vCard QR codes, animated avatars, and ambient RF activity meter. |

---

### Cluster 4: Offline Knowledge, Media, Smart Home & Desk Productivity (Projects 27–38)

| # | Project Name | Description |
|---|---|---|
| **27** | **Offline Survival Encyclopedia** | MicroSD SQLite3 offline reader for emergency first-aid procedures, knots, and water purification guides. |
| **28** | **Spotify Album Art Media Remote** | Live album art decoding, ambient dominant color extractor, and resistive swipe playback gestures. |
| **29** | **Formula 1 Pit Wall Clock** | Session countdown clocks, local timezone converter, 2D vector circuit layouts, and constructor standings. |
| **30** | **Catppuccin Weather Station** | Zero-API-key Open-Meteo weather station with 7-day forecasts and automatic LDR night-mode dimming. |
| **31** | **CO2 Indoor Air Quality Monitor** | Sensirion SCD40/SCD41 photoacoustic CO2 sensor interface with 24h ppm trend and ventilation alert. |
| **32** | **Smart Home Touch Switch Plate** | openHASP touchscreen control plate for lighting sliders, HVAC thermostat dial, and master scene toggles. |
| **33** | **Streaming Internet Lo-Fi Radio** | MP3/AAC worldwide web radio streamer with animated 16-band real-time FFT spectrum visualizer. |
| **34** | **Local Thermal Imaging Camera** | MLX90640 32x24 IR array interface with bilinear interpolation rendering 320x240 false-color heatmaps. |
| **35** | **Solar & Battery Flow Dashboard** | Animated energy flow nodes (Solar -> Inverter -> Battery -> Home) over Modbus/TCP or MQTT. |
| **36** | **Stylus Note Messenger** | High-precision resistive touch sketchpad exporting handwritten doodles and notes via webhook. |
| **37** | **Stylus RPN Calculator** | HP-16C programmer's bitwise calc and HP-12C financial calc emulator with 32-bit register bit flipping. |
| **38** | **Sim-Racing Telemetry Dash** | 60Hz UDP telemetry parser displaying gear indicator, RPM shift light bar, and Discord mute button. |

---

### Cluster 5: Retro Gaming, Generative Art, Virtual Pets & OS Environments (Projects 39–50)

| # | Project Name | Description |
|---|---|---|
| **39** | **5-in-1 Pocket Arcade Deluxe** | Suite of 5 touch arcade games using zero-flicker dirty-rectangle rendering and persistent high scores. |
| **40** | **Playable No-PSRAM FreeDOOM** | Bare-metal 3D Doom port running in 520KB SRAM with translucent touch controls and Bluetooth gamepad support. |
| **41** | **Minimax AI Gomoku & Snake** | Tactical 5-in-a-Row game with an Alpha-Beta pruning AI engine showing live evaluation heuristics. |
| **42** | **Tamagotchi Pomodoro Pet** | Desk companion whose happiness and health are tied to 25-minute Pomodoro study/work sessions. |
| **43** | **Procedural Emotive Robot Eyes** | 60 FPS Bezier procedural animated eyes reacting to touch gestures and room ambient light levels. |
| **44** | **ArdOS Retro Window Manager** | Retro desktop GUI with draggable overlapping windows, taskbar, Notepad, Calculator, and Minesweeper. |
| **45** | **PyDOS Handheld Computer** | MicroPython command-line PC with full on-screen stylus QWERTY virtual keyboard. |
| **46** | **Boids Artificial Life Toy** | Autonomous flocking simulation with touch predator repulsors and real-time rotating 3D wireframe hypercube. |
| **47** | **Neon Vector Spirograph** | LovyanGFX rotational symmetry mandala engine with glowing RGB565 brush strokes and SD BMP export. |
| **48** | **Modular Sensor Starter Kit** | Swipeable multi-page dashboard automatically plotting live data for plug-and-play Crowtail sensors. |
| **49** | **Smart 3D Filament Scale** | HX711 load cell interface with tare presets for major filament brands and remaining meter estimation. |
| **50** | **Wet-Finger Kitchen Timer** | 4-zone simultaneous kitchen timer with meat probe temperature alarms and recipe step checklist. |

---

## 📺 Channel Attribution

All projects in this collection are developed for and featured on:
* **YouTube**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
* **Author**: Tushar Aggarwal ([@tushar2704](https://github.com/tushar2704))
