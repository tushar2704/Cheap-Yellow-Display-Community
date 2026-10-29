# 🚀 Elecrow 2.8" CYD — 50 Original Touch Projects Master Plan

> **Official Master Plan & Project Blueprint**  
> **Brand & YouTube Channel**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech) (`@IamNotIntoTech`)  
> **Target Hardware**: Elecrow CrowPanel 2.8" / ESP32-2432S028R (ILI9341 320x240 TFT + XPT2046 Resistive Touch)  
> **Repository Directory**: `elecrow_projects/`  
> **Status**: 📋 Planning Phase (Architecture, Project Specs & Implementation Roadmap)

---

## 📌 1. Hardware Architecture & Pinout Standard

All 50 projects in this series are tailored and calibrated specifically for the **Elecrow 2.8" CYD (ESP32-2432S028R / CrowPanel 2.8")**:

| Component | Controller / Chip | GPIO Pin / Interface | Notes & Characteristics |
|---|---|---|---|
| **MCU** | ESP32-WROOM-32E | Dual-Core Xtensa LX6 @ 240MHz | 520 KB SRAM, 4MB Flash (No PSRAM) |
| **Display Panel** | ILI9341 (320x240 QVGA) | SPI (MOSI: IO13, MISO: IO12, SCK: IO14, CS: IO15, DC: IO2) | High refresh rate via DMA |
| **Backlight** | Backlight Transistor | GPIO 21 (PWM dimmable) | Inverted PWM brightness control |
| **Touch Controller** | XPT2046 Resistive Touch | SPI (MOSI: IO32, MISO: IO39, SCK: IO25, CS: IO33, IRQ: IO36) | Glove & stylus friendly |
| **RGB LED** | Onboard Tricolor LED | Red: IO4, Green: IO16, Blue: IO17 | Active-LOW logic |
| **Photoresistor (LDR)**| Ambient Light Sensor | GPIO 34 (ADC1_CH6) | Auto-brightness & night detection |
| **Audio Output** | Onboard Amp + SPK header | GPIO 26 (DAC2 / Tone / PWM) | 1.25mm 2-pin JST connector |
| **MicroSD Card** | TF Slot (SPI Mode) | CS: IO5, MOSI: IO23, MISO: IO19, SCK: IO18 | Up to 32GB FAT32 |
| **I2C Expansion** | HY2.0-4P (Crowtail) | SDA: IO27, SCL: IO22, 3.3V, GND | Plug-and-play sensor bus |
| **UART / GPIO Port** | HY2.0-4P (Crowtail) | TX: IO1 (or IO22), RX: IO3 (or IO27) | GPS, Serial Bridge, IR transceiver |
| **Analog In / Extra** | 2-Pin Expansion Header | GPIO 35 (Input Only) | ADC probe / external sensor |

---

## 📁 2. Directory & Coding Conventions

Each project will live inside `elecrow_projects/{NN}-{project-slug}/` following this standardized production structure:

```
elecrow_projects/
├── 01-devops-command-vault/
│   ├── README.md               # IamNotIntoTech branding, wiring, demo & YT links
│   ├── platformio.ini          # Standalone PlatformIO config for ESP32-2432S028R
│   ├── 01-devops-command-vault.ino
│   ├── src/                    # Modular C++ logic & UI controllers
│   ├── include/                # Hardware pinouts & assets
│   ├── assets/                 # UI screenshots & promo graphics
│   └── enclosure/              # 3D print STL specs & mount designs
```

### Universal Firmware Standards:
- **Zero-Cloud & Privacy First**: Where possible, operate 100% offline or on local LAN.
- **SRAM-Conservative Engine**: Tailored for 520KB SRAM (no PSRAM) using chunked HTTP streaming, dirty-rectangle rendering, and MicroSD asset paging.
- **Consistent Branding**: Boot splash and title headers display `IamNotIntoTech` branding.
- **Calibrated Touch**: Built-in 5-point calibration persisted to ESP32 Non-Volatile Storage (NVS).

---

## 📑 3. The 50 Projects Blueprint

---

### Cluster 1: Offline IT, SysAdmin, Homelab & Developer Appliances (Projects 01–08)

#### 01. DevOps Command Vault (Docker, Git & Exit Codes)
* **Open-Source Base**: [lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit](https://github.com/lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit)
* **2.8" Porting Twist**: Implements a complete interactive offline Docker run/compose CLI builder, Git disaster rescue cheat sheet (detached HEAD, cherry-pick recovery), POSIX Linux signal lookup (SIGTERM/SIGKILL/SIGHUP), and DNS record reference table with responsive resistive category filters.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "The $15 Cheat Code Screen That Saved My Coding Job"
  * *Hook*: Stop switching tabs and googling cryptic Git flags or Docker port syntaxes. A dedicated mechanical-keypad desk screen gives you instant command generation with one touch.
* **Hardware & Pins**: ILI9341 Display, XPT2046 Touch, LDR auto-dim (IO34).
* **3D Enclosure Concept**: *Keycap-Angled Desk Wedge* — 35° ergonomic tilt angle matching standard mechanical keyboards with a stylus pen holster.

---

#### 02. Chmod & VLSM Visual Network Architect
* **Open-Source Base**: [lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit (V2)](https://github.com/lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit)
* **2.8" Porting Twist**: Redesigned for 320x240 QVGA with a visual subnet block divider. Shows how a `/24` network breaks down into `/26`, `/27`, `/28` subnets with usable host ranges, broadcast IP calculation, and an interactive 9-box `rwxr-xr-x` octal permission calculator.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Stop Using Sketchy Websites to Calculate Subnets"
  * *Hook*: Calculating CIDR blocks and permissions in your head during an outage is stressful. This offline pocket tool guarantees zero telemetry and zero latency.
* **Hardware & Pins**: ILI9341, XPT2046, RGB LED (IO4/16/17 flashes green on valid IP math).
* **3D Enclosure Concept**: *Server-Rack Magnetic Puck* — Rear neodymium magnet slots to snap directly onto server racks and network switches.

---

#### 03. Standalone Homelab Wake-on-LAN (WoL) Touch Deck
* **Open-Source Base**: [crowpanel WoL community base](https://github.com/topics/crowpanel)
* **2.8" Porting Twist**: UDP Magic Packet transmitter combined with background ICMP ping polling. Server tiles dynamically glow Green (Online), Amber (Booting), or Red (Offline). Features a 4-digit security PIN lock screen on the resistive touch keypad.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Built a Physical 'Turn On My PC' Button That Cuts My Power Bill"
  * *Hook*: Keep your 500W power-hungry workstation or server powered down until you tap this dedicated bedside/desk panel.
* **Hardware & Pins**: Wi-Fi 802.11b/g/n, UDP socket, ICMP ping, Speaker buzzer (IO26) for confirmation beeps.
* **3D Enclosure Concept**: *1U Mini-Rack Ear Mount* — Mounts flush into a 10" or 19" homelab mini-rack faceplate.

---

#### 04. USB Serial PC Telemetry & Macro Deck
* **Open-Source Base**: [lepczynski-cloud/ESP32-PC-Control-Deck](https://github.com/lepczynski-cloud/ESP32-PC-Control-Deck)
* **2.8" Porting Twist**: Bypasses the CH340 hardware USB-HID limit using a lightweight Python background serial bridge. Renders live CPU %, GPU %, VRAM usage, and temperatures alongside 6 custom macro tiles (mute mic, launch terminal, run build scripts, AI quick prompt).
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Why Pay $150 for a Stream Deck When This $15 Screen Exists?"
  * *Hook*: Build a customizable streaming and coding macro pad with live hardware gauges for 1/10th the price of commercial units.
* **Hardware & Pins**: CH340 USB-UART, ILI9341 display, RGB alert LED on CPU throttle.
* **3D Enclosure Concept**: *Under-Monitor Clip Pod* — Clamps directly to the bottom bezel of any desktop monitor.

---

#### 05. Local-First Private AI Desk Terminal (Ollama Client)
* **Open-Source Base**: [lepczynski-cloud/crowpanel_AI_assistant](https://github.com/lepczynski-cloud/crowpanel_AI_assistant)
* **2.8" Porting Twist**: Fully adapted for no-PSRAM ESP32-WROOM-32E with a chunked HTTP JSON stream reader. Connects to your local LAN Ollama server (`http://homelab.local:11434`) with one-touch prompt tiles: "Explain Clipboard", "Fix Regex", "Draft Reply", and "Terminal Rescue".
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "A Private AI Assistant on Your Desk With Zero Monthly Fees"
  * *Hook*: Total privacy, running local Llama 3 / Mistral LLMs straight to a physical retro terminal on your desk with zero internet required.
* **Hardware & Pins**: Wi-Fi client, HTTP chunked stream parser, ILI9341 smooth text canvas.
* **3D Enclosure Concept**: *Retro 1980s CRT Terminal Shell* — Miniature vintage IBM/Macintosh housing with ventilation slots.

---

#### 06. ESP32 Ad-Blocking DNS Sinkhole & Traffic Monitor
* **Open-Source Base**: [ESP32 DNS Sinkhole community](https://github.com/topics/esp32-s3)
* **2.8" Porting Twist**: MicroSD-based blocklist storage with an on-device local DNS resolver. Features a giant touchable "Pause Blocking for 5 Minutes" snooze button (for broken affiliate/shopping links) and a real-time blocked domain graph.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "This $15 Screen Blocks Every Ad on My Home Wi-Fi (No Raspberry Pi)"
  * *Hook*: Pi-hole hardware without buying a $50 single-board computer! A low-power, zero-noise screen that purges telemetry and ads across all home phones and TVs.
* **Hardware & Pins**: Wi-Fi AP/Client, MicroSD (IO5/18/19/23) for 50,000+ domain lists, RGB LED indicator.
* **3D Enclosure Concept**: *Router-Top Antenna Saddle* — Clips onto router ventilation grilles, powered by the router's USB port.

---

#### 07. Bambu Lab Dual-Printer Telemetry Desk Monitor
* **Open-Source Base**: [Keralots/BambuHelper](https://github.com/Keralots/BambuHelper)
* **2.8" Porting Twist**: Native MQTT subscriber over local Wi-Fi for Bambu Lab X1/P1/A1 printers. Calculates real-time filament spool cost consumed and grams remaining, with onboard RGB LED warning (flashing red on print errors/spaghetti alerts, solid green on print finish).
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Watch Your 3D Printer From Across the House Without Opening an App"
  * *Hook*: No app lag, no cloud delays. A live heads-up desk display showing print progress, nozzle temperatures, and layer height.
* **Hardware & Pins**: MQTT over TLS/Plain, JSON parser, RGB LED (IO4/16/17), Piezo buzzer (IO26) for completion chime.
* **3D Enclosure Concept**: *Mini AMS Spool Stand* — Styled like a scaled-down Bambu AMS filament spool with internal cable management.

---

#### 08. Multi-Printer Wireless Command Deck (Klipper / OctoPrint)
* **Open-Source Base**: [suchmememanyskill/CYD-Klipper](https://github.com/suchmememanyskill/CYD-Klipper) & [xperiments-in/xtouch](https://github.com/xperiments-in/xtouch)
* **2.8" Porting Twist**: Connects to Moonraker and OctoPrint REST APIs. Features emergency stop, babystepping Z-offset adjustments, bed temperature PID graphs, and external DS18B20 chamber temperature probe integration via the HY2.0-4P port (IO27/IO22).
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "The $15 Touchscreen Upgrade Every 3D Printer Owner Needs"
  * *Hook*: Ditch old 12864 click-wheel LCDs! Add a full-color responsive touchscreen with emergency macros and chamber safety alarms.
* **Hardware & Pins**: Wi-Fi, Moonraker WebSockets, DS18B20 OneWire sensor on IO27, ILI9341.
* **3D Enclosure Concept**: *2020 V-Slot Extrusion Snap Bracket* — Direct mount to Ender 3 / Voron / Prusa frame extrusions.

---

### Cluster 2: Live Aviation, Space, Ham Radio & RF Telemetry (Projects 09–18)

#### 09. ATC Rotating Sweep Flight Radar ("What Was That?" Edition)
* **Open-Source Base**: [Eiswolf-BG/eiswolfs-flightradar-CYD](https://github.com/Eiswolf-BG/eiswolfs-flightradar-CYD)
* **2.8" Porting Twist**: Dual-core architecture (Core 0 handles ADSB REST API parsing, Core 1 handles 30 FPS CRT sweep radar rendering). Features proximity threshold alerts with buzzer chirps whenever military aircraft, helicopters, or Squawk 7700 emergencies cross your local airspace.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Ever Hear a Loud Plane Overhead? This $15 Screen Tells You What It Is"
  * *Hook*: Whenever a jet roars over your roof, look at your desk: instant altitude, model, origin, destination, and squawk code.
* **Hardware & Pins**: Wi-Fi, ADSB API, Piezo speaker (IO26) for radar sweep audio, ILI9341 display.
* **3D Enclosure Concept**: *Air Traffic Control Tower Console* — Angled anti-glare hood designed to simulate an airport radar terminal.

---

#### 10. Sky-Spotter Flight Deck with Airline Visuals
* **Open-Source Base**: [theqkash/esp32flight](https://github.com/theqkash/esp32flight)
* **2.8" Porting Twist**: Optimized for 320x240 with MicroSD storage for 500+ compressed airline logos. Calculates the exact "Spotter Line" (compass azimuth and elevation angle) so you can point your eyes or binoculars at the exact spot in the sky.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Built a Live Airport Flight Board & Radar for $15 (No Subscriptions)"
  * *Hook*: Live flight departures, arrivals, and overhead sightings with colorful airline branding and zero paid API keys.
* **Hardware & Pins**: MicroSD (IO5) for airline asset cache, Wi-Fi HTTPS client, XPT2046 touch.
* **3D Enclosure Concept**: *Airplane Window Bezel Frame* — Miniature commercial airliner cabin window with sliding sunshade.

---

#### 11. Orbital Rocket Launch & ISS Space Weather Tracker
* **Open-Source Base**: [JamesDavid/Overhead](https://jamesdavid.github.io/Overhead/)
* **2.8" Porting Twist**: Local SGP4 orbital propagator running in integer math (no PSRAM required). Calculates next visible backyard flyovers of the International Space Station and Starlink clusters. Strobe-pulses the RGB LED 5 minutes before horizon rise.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "This Tiny Gadget Glows Whenever the Space Station Flies Over My House"
  * *Hook*: Never miss an ISS pass or a SpaceX launch countdown again with this physical NASA-themed mission clock.
* **Hardware & Pins**: HTTPS Launch Library 2 API, SGP4 orbital math, RGB LED (IO4/16/17).
* **3D Enclosure Concept**: *Apollo DSKY Flight Computer Box* — Styled after the iconic Apollo Guidance Computer keyboard and numeric display.

---

#### 12. Pilot's Desk METAR & TAF Aviation Weather Decoder
* **Open-Source Base**: [OttoMeister/ESP32-2432S028R](https://github.com/OttoMeister/ESP32-2432S028R)
* **2.8" Porting Twist**: Parses cryptic raw NOAA METAR/TAF strings into clear flight categories (VFR Green, MVFR Blue, IFR Red, LIFR Magenta). Generates a dynamic touch-calculated runway crosswind and headwind component gauge for 6 favorite airports.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Why Pilots Use Secret Weather Codes (And How This $15 Screen Decodes Them)"
  * *Hook*: Understand what the aviation weather letters actually mean, with wind barbs, visibility limits, and runway vector calculations.
* **Hardware & Pins**: Aviation Weather API, XPT2046 touch, ILI9341 display.
* **3D Enclosure Concept**: *Cockpit Instrument Gauge Bezel* — Square aviation instrument bezel with corner hex-head bolts.

---

#### 13. HamClock Mini: DX Cluster, FT8 & Solar Greyline Dashboard
* **Open-Source Base**: [Matt-Housley/cyd-dashboard](https://github.com/Matt-Housley/cyd-dashboard) & [hf7a/ESP32-ham-combo](https://github.com/hf7a/ESP32-ham-combo)
* **2.8" Porting Twist**: Live Solar Flux Index (SFI), planetary K-index geomagnetic gauge, POTA (Parks on the Air) / SOTA spotter feed, and tap-to-calculate Maidenhead grid square distance and beam heading.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "How Space Weather Controls Radio Waves Around the Earth"
  * *Hook*: A dedicated solar-geomagnetic telemetry screen showing ham radio propagation windows and global greyline maps.
* **Hardware & Pins**: Wi-Fi, NOAA SWPC JSON feed, POTA API, resistive touch list scroller.
* **3D Enclosure Concept**: *Rugged Field-Go-Box Clip* — MOLLE-compatible tactical clip with impact bumpers for portable radio field bags.

---

#### 14. Minimalist Vector Flight Wall Tracker (adsb.fi Edition)
* **Open-Source Base**: [nicholaswilde/cyd-flight-radar](https://github.com/nicholaswilde/cyd-flight-radar) & [AnthonySturdy/micro-radar](https://github.com/AnthonySturdy/micro-radar)
* **2.8" Porting Twist**: High-contrast "Single Nearest Aircraft Card" mode. Giant, crisp typography displaying flight number, altitude, knots, and flight path progress bar, readable from 10 feet away across a living room or office.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "The Cleanest Desk Gadget I Own Costs Less Than Lunch"
  * *Hook*: Minimalist Swiss design meets live ADS-B aerospace telemetry without clutter.
* **Hardware & Pins**: Wi-Fi, adsb.fi open API, LDR auto-dimming for nighttime sleep.
* **3D Enclosure Concept**: *Picture-Frame Wall Puck* — Ultra-thin flush wall mount with hidden recessed USB-C cable routing.

---

#### 15. GPS-Disciplined Stratum-Style Triple Timezone & Grid Clock
* **Open-Source Base**: [CoolmdXi/CYD-Gps-Triple-clock](https://github.com/CoolmdXi/CYD-Gps-Triple-clock)
* **2.8" Porting Twist**: Interfaces an external NEO-6M / NEO-M8N GPS module via the HY2.0-4P UART port (IO27/IO22). Operates 100% offline with zero Wi-Fi, displaying Local, UTC/Zulu, GPS atomic time, precision fix status, and satellite SNR constellation bars.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "A Clock That Never Needs Wi-Fi or Setting—Ever"
  * *Hook*: Connect to satellites in space to build an atomic-accurate desk clock that works during power and internet outages.
* **Hardware & Pins**: UART GPS Receiver (HY2.0-4P), NMEA sentence parser, ILI9341 display.
* **3D Enclosure Concept**: *Dual-Bay Slanted GPS Pod* — Angled desk stand with an integrated skyward window for the ceramic GPS patch antenna.

---

#### 16. Live NOAA Coastal Tide, Swell & Moon Phase Chart
* **Open-Source Base**: [NOAA Tide Chart community](https://github.com/topics/xpt2046)
* **2.8" Porting Twist**: Continuous harmonic sine-wave curve showing tidal height (feet/meters) centered on "NOW", with golden hour sunrise/sunset vertical markers, moon illumination percentage, and quick switching between 5 favorite coastal stations.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Built a Live Ocean Tide & Beach Forecast Screen for My Desk"
  * *Hook*: Perfect for surfers, boaters, and beach lovers: glance at water level trends, tidal flow direction, and sunset timing.
* **Hardware & Pins**: NOAA CO-OPS REST API, XPT2046 touch selector, ILI9341 smooth curve plotter.
* **3D Enclosure Concept**: *Nautical Brass Porthole Case* — Circular vintage marine porthole frame transforming the rectangular screen into an ocean gauge.

---

#### 17. Offline OpenStreetMap Trail & Moto Turn-by-Turn Navigator
* **Open-Source Base**: [ESP32 GPS Navigator community](https://github.com/topics/esp32-s3)
* **2.8" Porting Twist**: Renders pre-downloaded 256x256 OSM map tiles from MicroSD. The resistive XPT2046 touch panel functions flawlessly through thick motorcycle or winter gloves (unlike capacitive phone screens). Features GPX track logging and digital compass bearing.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Why My $15 DIY GPS Beats a Smartphone When Wearing Gloves"
  * *Hook*: Capacitive phone screens fail with gloves and rain. This resistive offline GPS navigator works anywhere with zero cell service.
* **Hardware & Pins**: UART GPS module, MicroSD tile cache, resistive glove-ready touch.
* **3D Enclosure Concept**: *Handlebar RAM-Mount Cradle* — Vibration-dampened motorcycle handlebar clamp with built-in sun/rain visor.

---

#### 18. ESP-NOW Wireless Marine & Wind Instrument Repeater
* **Open-Source Base**: [crowpanel ESP-NOW telemetry](https://github.com/topics/crowpanel)
* **2.8" Porting Twist**: Zero-router wireless receiver using ESP-NOW packets from outdoor solar-powered sensor nodes. Displays wind speed and direction on an analog polar rose, 24-hour barometric pressure trend barograph, and bilge water alarm.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Long-Range Wireless Sensors With Zero Wi-Fi Required"
  * *Hook*: Broadcast sensor telemetry up to 250 meters across yards, garages, or boats with ultra-low power and zero network infrastructure.
* **Hardware & Pins**: ESP-NOW protocol, speaker alarm (IO26), RGB status LED.
* **3D Enclosure Concept**: *Marine Bulkhead Instrument Pod* — Rubber gasket-ready surface mount with front stylus clip.

---

### Cluster 3: Cybersecurity Auditing, RF Diagnostics & Bench Instruments (Projects 19–26)

#### 19. Defensive Home Wi-Fi Channel & Rogue AP Auditor
* **Open-Source Base**: [lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit (V2 Wireless)](https://github.com/lepczynski-cloud/crowpanel-Admin-Pocket-Toolkit)
* **2.8" Porting Twist**: Real-time 2.4GHz spectrum analyzer displaying 802.11 channel 1–14 overlap bell curves with dBm signal strength. Automatically calculates and recommends the least congested Wi-Fi channel, and alerts on duplicate rogue AP SSIDs.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Why Your Home Wi-Fi Is Slow (This $15 Screen Shows the Hidden Problem)"
  * *Hook*: See invisible Wi-Fi interference from neighbors and discover the exact channel settings to double your wireless speed.
* **Hardware & Pins**: Promiscuous Wi-Fi scanning, ILI9341 bell curve graph, XPT2046 channel selector.
* **3D Enclosure Concept**: *Handheld Tricorder Shell* — Sci-Fi handheld diagnostic scanner body with top antenna cutout.

---

#### 20. AirTag, Skimmer & Hidden BLE Tracker Detector ("Privacy Shield")
* **Open-Source Base**: [Passive BLE Badge community](https://github.com/topics/xpt2046)
* **2.8" Porting Twist**: Passive NimBLE advertisement sniffer. Identifies and categorizes Apple FindMy AirTags, Tile beacons, Samsung SmartTags, and suspicious HC-05 serial modules. Ranks devices by proximity RSSI and alerts on persistent trackers moving with you over time.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "How to Spot Hidden Bluetooth Trackers & Hotel Devices With a $15 Screen"
  * *Hook*: Detect unwanted tracking devices and rogue Bluetooth bugs in hotel rooms, rental cars, or luggage.
* **Hardware & Pins**: BLE promiscuous scanner, NVS MAC history table, buzzer alarm (IO26).
* **3D Enclosure Concept**: *Pocket Keychain FOB Slider* — Pocket-sized shell with a slide-to-protect acrylic lens cover.

---

#### 21. Retro CRT Phosphor Bench Oscilloscope & PWM Generator
* **Open-Source Base**: [kosmitta/esp32-024-crt-scope](https://github.com/kosmitta/esp32-024-crt-scope)
* **2.8" Porting Twist**: Reads analog waveforms through GPIO 35 via ADC DMA. Simulates authentic green phosphor CRT trace decay persistence. Simultaneously outputs a touch-adjustable test square-wave PWM signal (1Hz to 100kHz) on GPIO 22.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Turned a $15 Screen Into a Retro Sci-Fi Oscilloscope"
  * *Hook*: Troubleshoot audio circuits, sensors, and PWM signals with an ultra-cool vintage CRT aesthetic.
* **Hardware & Pins**: GPIO35 ADC1 input, GPIO22 PWM generator output, DMA display refresh.
* **3D Enclosure Concept**: *Mini Tektronix Lab Scope* — Classic laboratory oscilloscope cabinet with integrated BNC-to-JST probe jack.

---

#### 22. Universal CYD Hardware Variant Identifier & Pin Tester
* **Open-Source Base**: [HenrysCat/ESP32-2432S028-CYD-Diagnostic-Tool](https://github.com/HenrysCat/ESP32-2432S028-CYD-Diagnostic-Tool)
* **2.8" Porting Twist**: Interactive peripheral diagnostic bench: tests LDR curve (IO34), cycles active-low RGB LEDs, sweeps speaker frequencies (IO26), benchmarks MicroSD read/write speed, verifies HY2.0 ports, and exports calibrated 5-point touch data to NVS.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Bought a Cheap ESP32 Screen? Run This First to See If It's Fake or Broken"
  * *Hook*: Test every trace, sensor, and capacitor on newly delivered CYD boards to catch manufacturing defects before soldering.
* **Hardware & Pins**: All peripherals (LDR IO34, RGB IO4/16/17, SPK IO26, SD IO5, Touch).
* **3D Enclosure Concept**: *Test-Jig Snap Dock* — Open-chassis bench tray allowing full access to rear test points and headers.

---

#### 23. Multi-Firmware SD-Card Bootloader & App Switcher
* **Open-Source Base**: [bmorcelli/Launcher](https://github.com/bmorcelli/Launcher)
* **2.8" Porting Twist**: Custom graphical touch UI launcher. Lets you store dozens of compiled `.bin` firmware files on a FAT32 MicroSD card and switch between Admin Toolkit, Flight Radar, Retro Games, and Oscilloscope on the fly via OTA partition flashing without a computer.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Turn One $15 Screen Into 20 Different Gadgets With an SD Card"
  * *Hook*: Why dedicate a board to one project? Boot any firmware like a Nintendo Game Boy cartridge directly from an SD card.
* **Hardware & Pins**: ESP32 OTA flash partitioning, MicroSD (IO5), XPT2046 touch selector.
* **3D Enclosure Concept**: *Cartridge-Slot Game Dock* — Housing with a spring-assisted MicroSD cartridge slot and eject lever.

---

#### 24. Pocket RF & Infrared Universal Diagnostic Multi-Tool
* **Open-Source Base**: [BruceDevices/firmware](https://github.com/BruceDevices/firmware) & [Mark-MDO47/UniRemote](https://github.com/Mark-MDO47/UniRemote)
* **2.8" Porting Twist**: Pairs an external 940nm IR transmitter/receiver on the HY2.0-4P port. Clones and visualizes TV/AC remote raw pulse waveforms, offers on-screen touch remotes, and displays dynamic Wi-Fi connection QR codes.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Replace 10 Lost TV and AC Remotes With One $15 Touchscreen"
  * *Hook*: Read raw infrared signals, decode protocols, and build a master universal touch remote for your living room or workshop.
* **Hardware & Pins**: IR Receiver & Emitter on HY2.0-4P, QR Code engine, ILI9341 display.
* **3D Enclosure Concept**: *Ergonomic TV Blaster Grip* — Contoured handheld remote body with front twin IR-LED transparent lens windows.

---

#### 25. Packet-Per-Second Wi-Fi Deauth Detector & Security Alarm
* **Open-Source Base**: [Fr4nkFletcher/ESP32-Marauder-Cheap-Yellow-Display](https://github.com/Fr4nkFletcher/ESP32-Marauder-Cheap-Yellow-Display) & [Spooks4576/Ghost_ESP](https://github.com/Spooks4576/Ghost_ESP)
* **2.8" Porting Twist**: A pure defensive blue-team watchdog. Monitors 802.11 management frames in promiscuous mode for deauthentication/disassociation floods (used by jammers to take down Wi-Fi security cameras). Flashes the screen red, triggers the buzzer, and sends a Discord webhook alert.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "How Wi-Fi Jammers Disable Security Cameras (And How to Detect It)"
  * *Hook*: How criminals knock wireless cameras offline, and how this $15 sentinel catches them in the act.
* **Hardware & Pins**: Promiscuous 802.11 sniffer, Speaker alarm (IO26), RGB strobe (IO4/16/17).
* **3D Enclosure Concept**: *Wall-Mounted Security Sentinel* — Sleek security-keypad wall unit with an RGB frosted diffuser lens.

---

#### 26. Wearable Tech Conference Badge & Dynamic vCard QR Generator
* **Open-Source Base**: [Cybersecurity Badge community](https://github.com/topics/xpt2046)
* **2.8" Porting Twist**: Battery-powered portable lanyard badge (powered via 3.7V LiPo on the battery JST connector). Cycles between animated name tags, social media QR codes (vCard, LinkedIn, YouTube), and a live ambient 2.4GHz RF activity meter.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "The Ultimate Digital Business Card That Never Needs an App"
  * *Hook*: Turn heads at maker faires and tech conferences with an interactive touch badge that beams your contacts straight to phones.
* **Hardware & Pins**: LiPo battery management, QR code generator, LDR brightness auto-adjustment.
* **3D Enclosure Concept**: *Lanyard Badge Armor* — Slim frame with lanyard eyelets and a rear cavity for a 1000mAh flat LiPo cell.

---

### Cluster 4: Offline Knowledge, Media, Smart Home & Desk Productivity (Projects 27–38)

#### 27. "Don't Panic" Offline Survival & First-Aid Encyclopedia
* **Open-Source Base**: [GlitchedPanda/DontPanic](https://github.com/GlitchedPanda/DontPanic)
* **2.8" Porting Twist**: High-speed SQLite3 database running off the MicroSD card. Contains an indexed offline library: emergency first-aid protocols, survival knot guides, water purification ratios, Morse code tables, and plant safety. Uses a retro amber Hitchhiker's Guide vector UI.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Put an Entire Survival Encyclopedia on a $15 Screen That Needs No Internet"
  * *Hook*: When cell towers go down, this pocket guide keeps critical life-saving manuals, maps, and guides at your fingertips.
* **Hardware & Pins**: MicroSD SQLite3 engine, XPT2046 resistive touch search keyboard.
* **3D Enclosure Concept**: *"Don't Panic" Rugged Clamshell* — Protective book-style case with embossed lettering and stylus clasp.

---

#### 28. DIY Spotify "Car Thing" & Album Art Desk Controller
* **Open-Source Base**: [witnessmenow/Spotify-Diy-Thing](https://github.com/witnessmenow/Spotify-Diy-Thing) & [epixcz/esp32-cyd-home-assistant-media-remote](https://github.com/epixcz/esp32-cyd-home-assistant-media-remote)
* **2.8" Porting Twist**: Live JPEG album cover decoding over Wi-Fi, dynamic color background matching (extracts dominant color from album art), and smooth touch swipe gestures: swipe left/right for tracks, tap to pause, drag bottom scrubber for volume.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Spotify Killed the 'Car Thing'—So I Built a Better One for $15"
  * *Hook*: Spotify bricked thousands of official Car Things; this open-source build gives you live album art and media controls forever.
* **Hardware & Pins**: Wi-Fi WebSockets / Spotify Web API, TJpg_Decoder, XPT2046 swipe gestures.
* **3D Enclosure Concept**: *Car-Thing Minimalist Knob Bezel* — Angled desk cradle with matte-black finish and a tactile styling dial.

---

#### 29. Formula 1 & Motorsport Race Weekend Countdown & Pit Wall
* **Open-Source Base**: [witnessmenow/F1-Arduino-Notifications](https://github.com/witnessmenow/F1-Arduino-Notifications)
* **2.8" Porting Twist**: Pulls free OpenF1 and Ergast motorsport data. Features session countdown clocks (FP1, FP2, Quali, Sprint, Grand Prix) automatically adjusted to your local timezone, 2D vector track layouts, and real-time constructor standings.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Never Miss an F1 Start Time Again With This $15 Pit-Wall Screen"
  * *Hook*: Never calculate race times across foreign timezones again. A miniature Formula 1 telemetry dashboard for your gaming desk.
* **Hardware & Pins**: Wi-Fi HTTPS client, MicroSD track graphics, ILI9341 display.
* **3D Enclosure Concept**: *Pirelli Tire Stand* — Circular soft TPU racing tire stand that holds the display in its center hub.

---

#### 30. Catppuccin-Themed Open-Meteo Weather & Air Quality Station
* **Open-Source Base**: [Catppuccin Weather community](https://github.com/topics/esp32-2432s028) & [cjgb/esp32-2432s028-meteo](https://github.com/cjgb/esp32-2432s028-meteo)
* **2.8" Porting Twist**: Utilizes the 100% free Open-Meteo endpoint (zero API keys or account registration required). Styled in the gorgeous Catppuccin Mocha pastel color theme. Includes 7-day hourly forecasts, UV index, and LDR-based auto night dimming.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "The Prettiest $15 Weather Station (Zero Accounts or API Keys Needed)"
  * *Hook*: No credit cards, no expiring OpenWeatherMap keys. Instant plug-and-play modern weather dashboard.
* **Hardware & Pins**: Wi-Fi, Open-Meteo JSON, LDR sensor (IO34), PWM backlight control (IO21).
* **3D Enclosure Concept**: *Mid-Century Modern Braun Stand* — Dieter Rams minimalist white desk stand with a clean 15° backward tilt.

---

#### 31. CO2 & Indoor Air Quality "Brain Fog" Monitor (SCD40/SCD41)
* **Open-Source Base**: [Sensirion SCD40 community](https://github.com/topics/esp32-2432s028r)
* **2.8" Porting Twist**: Connects a Sensirion SCD40/SCD41 true NDIR photoacoustic CO2 sensor via the I2C HY2.0-4P connector. Live PPM gauge with 24-hour bar trend. Flashes yellow above 1,000 ppm and red above 1,500 ppm with an audible "Open a Window!" alert.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Why You Feel Tired at Your Desk by 2 PM (And How This Screen Fixes It)"
  * *Hook*: High CO2 levels in sealed rooms ruin productivity. Build a true optical CO2 monitor to know exactly when to ventilate.
* **Hardware & Pins**: I2C port (IO27/IO22), Sensirion SCD4x driver, Buzzer alarm (IO26).
* **3D Enclosure Concept**: *Ventilated Honeycomb Tower* — Enclosure with an isolated rear thermal chimney preventing screen backlight heat from skewing temperature readings.

---

#### 32. No-YAML Home Assistant Wall Switch Plate (openHASP Touch)
* **Open-Source Base**: [HASwitchPlate/openHASP](https://github.com/HASwitchPlate/openHASP)
* **2.8" Porting Twist**: Ready-to-flash openHASP firmware calibrated for the 2.8" ILI9341 display. Pre-configured JSONL pages for room lighting brightness sliders, HVAC climate dial arcs, media playback, and a master "Goodnight All Off" macro button over MQTT.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Replaced My Boring Light Switches With $15 Smart Touchscreens"
  * *Hook*: Turn a standard wall plate into a slick interactive Home Assistant touchscreen control center.
* **Hardware & Pins**: openHASP engine, MQTT auto-discovery, XPT2046 touch, Backlight sleep timer.
* **3D Enclosure Concept**: *Flush Wall-Gang Decora Plate* — Snaps cleanly over standard North American / European wall light switch boxes with 5V DC step-down.

---

#### 33. Touch-Controlled Streaming Internet Radio & Lo-Fi Beats Player
* **Open-Source Base**: [Carlo47/CYD_InternetRadio](https://github.com/Carlo47/CYD_InternetRadio)
* **2.8" Porting Twist**: Stream MP3/AAC stations worldwide using `ESP32-audioI2S`. Outputs sound through the onboard amplifier connector (or external I2S DAC). Displays station logos and an animated 16-band real-time FFT audio spectrum visualizer.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "A Dedicated 'Lo-Fi Hip Hop & World Radio' Box for Your Desk"
  * *Hook*: Free music streaming with gorgeous live visualizer bars on your desk without opening browser tabs or apps.
* **Hardware & Pins**: Wi-Fi audio stream buffer, SPK amplifier (IO26), FFT math, ILI9341.
* **3D Enclosure Concept**: *Vintage Wood-Grain Transistor Radio* — Retro 1960s tabletop radio case with speaker grille styling.

---

#### 34. Local-First Thermal Imaging Camera & Sous-Vide PID Monitor
* **Open-Source Base**: [MLX90640 CYD community](https://github.com/topics/crowpanel)
* **2.8" Porting Twist**: Connects an MLX90640 32x24 far-infrared thermal sensor over I2C. Runs bilinear interpolation on Core 1 to render a silky 320x240 Ironbow/Rainbow thermal heatmap with touch-draggable spot temperature crosshairs and min/max auto-tracking.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Built a DIY Thermal Camera to Find Heat Leaks & Check PCB Hotspots"
  * *Hook*: Commercial FLIR thermal cameras cost $300+. Build your own portable thermal inspection camera for a fraction of that price.
* **Hardware & Pins**: MLX90640 I2C (IO27/IO22), Bilinear interpolation pipeline, ILI9341 palette engine.
* **3D Enclosure Concept**: *Pistol-Grip Thermal Imager* — Ergonomic handheld scanner grip with forward sensor alignment bezel.

---

#### 35. Solar & Off-Grid Battery Energy Flow Dashboard
* **Open-Source Base**: [Th3r4c3r/ESP32-Victron-VRM-Display](https://github.com/Th3r4c3r/ESP32-Victron-VRM-Display)
* **2.8" Porting Twist**: Animated energy flow diagram (Solar Panels ➔ MPPT Inverter ➔ Battery Storage ➔ Household Load ➔ Grid). Reads data from Victron Modbus/TCP, Home Assistant Energy entities, or a local INA226 current sensor.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "See Exactly How Much Power Your House Uses in Real Time"
  * *Hook*: Watch solar generation and household power draw animate live on your kitchen counter or campervan dashboard.
* **Hardware & Pins**: Wi-Fi Modbus/MQTT client, Animated node canvas, RGB battery state LED.
* **3D Enclosure Concept**: *Mini Utility Meter Box* — Industrial gray electrical enclosure miniature with faux conduit fittings.

---

#### 36. Stylus Cloud Sketchpad ("TinyCanvas" Fridge Note Messenger)
* **Open-Source Base**: [EJA-creative/tinycanvas-cyd](https://github.com/EJA-creative/tinycanvas-cyd)
* **2.8" Porting Twist**: Takes advantage of high-precision XPT2046 resistive stylus input. Draw notes, doodles, and to-do lists, select ink colors and brush sizes, and tap "Send" to automatically export and upload the note as a PNG image to Telegram, Discord, or Home Assistant.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "The Digital Sticky Note That Sends Handwritten Doodles Straight to Your Phone"
  * *Hook*: A modern fridge whiteboard that syncs notes and grocery lists straight to family phones when you tap send.
* **Hardware & Pins**: XPT2046 resistive stylus precision driver, Wi-Fi HTTPS bot client, MicroSD canvas buffer.
* **3D Enclosure Concept**: *Fridge-Magnet Notepad Frame* — Whiteboard-styled frame with rear rare-earth magnets and top stylus clip.

---

#### 37. Stylus RPN Scientific, Programmer's Hex & Financial Calculator
* **Open-Source Base**: [CoolmdXi/CYD-2USB-Simple-Calculator](https://github.com/CoolmdXi/CYD-2USB-Simple-Calculator)
* **2.8" Porting Twist**: Complete emulation mode switcher: HP-16C Programmer's mode (HEX/DEC/OCT/BIN bitwise ops, 32-bit register bit-flipping grid), HP-12C Financial mode (compound interest, NPV, loan schedules), and standard Reverse Polish Notation (RPN) stack.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Why Engineers Still Obsess Over 1980s Calculators (So I Built One for $15)"
  * *Hook*: Vintage HP scientific calculators sell for $300 on eBay. Here is how to build your own touchscreen RPN calculation powerhouse.
* **Hardware & Pins**: XPT2046 stylus keypad matrix, High-precision math engine, ILI9341 display.
* **3D Enclosure Concept**: *HP Voyager Beveled Horizontal Tray* — Classic brushed black-and-gold beveled calculator housing.

---

#### 38. Sim-Racing Telemetry & Discord Mic Control Dash
* **Open-Source Base**: [Sim-Racing UDP community](https://github.com/Vibed-Coder/ESP32-WeatherDisplay)
* **2.8" Porting Twist**: 60Hz UDP telemetry listener for Forza, Assetto Corsa, F1 24, and BeamNG.drive. Displays massive central gear indicator, RPM shift-light bar across the top of the display, speed, tire temps, and a dedicated touch button to mute your Discord mic.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Add a Real Race Car Dashboard to Your Gaming PC for $15"
  * *Hook*: Elevate your sim-racing setup with a physical shift-light telemetry dash that mounts directly to your steering wheel base.
* **Hardware & Pins**: High-speed UDP packet parser, ILI9341 DMA rendering, RGB LED shift-redline indicator.
* **3D Enclosure Concept**: *Steering Wheel Wheelbase Clamp* — Slender clamp mounting flush above Logitech, Thrustmaster, or Fanatec wheel hubs.

---

### Cluster 5: Retro Gaming, Generative Art, Virtual Pets & OS Environments (Projects 39–50)

#### 39. CrowPanel 5-in-1 Pocket Arcade Deluxe (Dirty-Rectangle Engine)
* **Open-Source Base**: [lepczynski-cloud/crowpanel-pocket-arcade](https://github.com/lepczynski-cloud/crowpanel-pocket-arcade)
* **2.8" Porting Twist**: Expands the original 3-game suite to 5 full games: Reflex Beacon, Shift Vault, Circuit Worm, Touch Brick Breaker, and Memory Matrix. Features zero-flicker dirty-rectangle rendering, persistent NVS high scores, and audio buzzer sound effects.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Turned a $15 Microcontroller Into a Pocket Touchscreen Game Console"
  * *Hook*: How to squeeze 5 fast, flicker-free touchscreen arcade games onto a single ESP32 without an operating system.
* **Hardware & Pins**: ILI9341 dirty-rect graphics, XPT2046 touch controls, Speaker audio (IO26), NVS high scores.
* **3D Enclosure Concept**: *Mini Arcade Cabinet* — Upright tabletop coin-op arcade shell with a backlit marquee slot.

---

#### 40. Playable FreeDOOM on a No-PSRAM 2.8" Touchscreen
* **Open-Source Base**: [freedoom.github.io / CYD Doom Port](https://freedoom.github.io/)
* **2.8" Porting Twist**: Highly optimized bare-metal port of the classic Doom engine running strictly in 520KB SRAM and 4MB Flash without requiring PSRAM. Translucent on-screen touch thumb controls (left movement zone, right aim/fire zone) plus optional Bluetooth gamepad support.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Can a $15 Smart Home Screen Actually Run DOOM?"
  * *Hook*: The ultimate hacker milestone: running real, playable 3D DOOM on a budget smart touchscreen microcontroller.
* **Hardware & Pins**: ESP32 dual-core rendering, ILI9341 direct frame write, Bluetooth classic gamepad / touch.
* **3D Enclosure Concept**: *Boomer-Shooter Handheld Grip* — Dual-handed controller grip with ergonomic rear palm swells.

---

#### 41. Unbeatable Minimax AI Gomoku (5-in-a-Row) & Arcade Snake
* **Open-Source Base**: [NguyenQuocPhi2006/ESP32-GAME_Snake-and-Caro](https://github.com/NguyenQuocPhi2006/ESP32-GAME_Snake-and-Caro_ESP32-2432S028_ESP32-3248S035_ESP32-S3-Display)
* **2.8" Porting Twist**: Implements an Alpha-Beta pruning Minimax AI engine running on Core 0 with live evaluation score graphs ("AI Confidence") shown in real time as the ESP32 calculates its next move. Includes a 2-player pass-and-play timer and Arcade Snake mode.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Programmed a $15 Chip to Beat Me at Board Games (And Now I Can't Win)"
  * *Hook*: Watch the microcontroller think in real time with an on-screen AI depth heuristics bar while playing tactical Gomoku.
* **Hardware & Pins**: Dual-core processing (Core 0 AI, Core 1 UI), XPT2046 precision stylus grid.
* **3D Enclosure Concept**: *Tabletop Game Board Tray* — Flat desk case with dual side stylus slots for two-player pass-and-play.

---

#### 42. Tamagotchi-Style Desk Companion & Pomodoro Focus Pet
* **Open-Source Base**: [CYD Tamagotchi community](https://github.com/topics/esp32-2432s028r)
* **2.8" Porting Twist**: Connects an animated virtual pet's health, hunger, and happiness directly to a 25-minute Pomodoro study/work timer. Completing focus sprints rewards your pet with treats and playful animations; abandoning tasks makes it grumpy.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "This $15 Desk Pet Dies If I Procrastinate on My Work"
  * *Hook*: The ultimate anti-procrastination productivity hack: a cute physical virtual pet that thrives when you focus and suffers when you Slack off.
* **Hardware & Pins**: Sprite animation engine, NVS life-state persistence, Speaker chimes (IO26), Touch petting.
* **3D Enclosure Concept**: *Giant 1990s Nostalgic Egg Shell* — Oversized classic Tamagotchi egg silhouette with a desktop stand.

---

#### 43. Procedural Emotive Robot Eyes & Desk Companion
* **Open-Source Base**: [TFT Emotive Eyes community](https://github.com/topics/xpt2046)
* **2.8" Porting Twist**: 60 FPS smooth Bezier procedural animated eyes with full touch responsiveness. Tapping or poking an eye causes it to squint, blink, or get irritated; stroking the screen makes it happy. Ambient light sensor (IO34) dynamically dilates and contracts pupils.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "I Gave My Desk Setup a Living Pair of Eyes for $15"
  * *Hook*: Turn a small touchscreen into a lifelike expressive desk companion that reacts to your touch and room lighting.
* **Hardware & Pins**: Bezier curve math, LDR light sensor (IO34), XPT2046 gesture tracking, Speaker expressions.
* **3D Enclosure Concept**: *Articulated Pixar-Style Robot Head* — Ball-jointed desktop neck stand allowing the eye unit to swivel and tilt.

---

#### 44. ArdOS Pocket Window Manager & Desktop GUI Environment
* **Open-Source Base**: [oguzgundogdu/ArdOS](https://github.com/oguzgundogdu/ArdOS)
* **2.8" Porting Twist**: Skinned with a nostalgic Windows 95 / Classic Mac System 7 retro desktop aesthetic. Features draggable overlapping windows, a working Start Menu, touch Notepad, Calculator, Minesweeper, and real-time System RAM / Task Manager monitors.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Running a Tiny 'Windows 95' Style Desktop on a $15 Chip"
  * *Hook*: Experience the nostalgia of early computing with a real multitasking window manager running on a tiny microcontroller.
* **Hardware & Pins**: C++ windowing manager, XPT2046 stylus cursor, MicroSD file storage, ILI9341 display.
* **3D Enclosure Concept**: *Miniature 1995 Desktop PC Tower & CRT* — Tiny beige computer monitor resting on top of a desktop PC chassis.

---

#### 45. PyDOS Retro Command-Line PC with Virtual Touch Keyboard
* **Open-Source Base**: [RetiredWizard/PyDOS_virtkeyboard](https://github.com/RetiredWizard/PyDOS_virtkeyboard)
* **2.8" Porting Twist**: MicroPython / CircuitPython PyDOS command line environment paired with an on-screen XPT2046 resistive QWERTY virtual keyboard. Write, edit, and execute Python scripts and retro `.BAS` BASIC programs directly on the display without any PC attached.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "A Pocket 1980s DOS Computer You Can Program With a Stylus"
  * *Hook*: Type DOS commands, write retro BASIC programs, and manage files on a self-contained pocket computer.
* **Hardware & Pins**: MicroPython interpreter, MicroSD virtual floppy disk, Stylus virtual keyboard.
* **3D Enclosure Concept**: *Pocket Cyberdeck Clamshell* — Rugged pocket folding clamshell with faux mechanical keyboard palm rests.

---

#### 46. Boids Artificial Life & 3D Wireframe Physics Desk Toy
* **Open-Source Base**: [Creative Coding Boids community](https://github.com/topics/esp32-2432s028r)
* **2.8" Porting Twist**: Craig Reynolds artificial life flocking simulation. Touching the screen creates dynamic predator repulsion waves or food vortex attractors. Double-tapping transforms the screen into a real-time touch-rotatable 3D wireframe hypercube/tesseract.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "The Most Satisfying $15 Desk Toy for When You're Stuck on Zoom Calls"
  * *Hook*: Hypnotic flocking math and interactive 3D matrix transformations that turn your desk screen into visual eye candy.
* **Hardware & Pins**: 3D matrix transformation pipeline, Float math optimization, Touch attractor physics.
* **3D Enclosure Concept**: *Minimalist Floating Acrylic Plinth* — Clear stand creating the illusion that the screen is levitating on the desk.

---

#### 47. NVS Touch Calibration Lab & Neon Vector Spirograph
* **Open-Source Base**: [xyz-rainbow/esp32-s3-expansion-tft-touch](https://github.com/xyz-rainbow/esp32-s3-expansion-tft-touch)
* **2.8" Porting Twist**: High-speed LovyanGFX rotational symmetry mandala and spirograph generator. Stylus strokes mirror across 4, 8, or 16 axes in vivid RGB565 neon glowing gradients. Saves artwork as 24-bit `.BMP` files to the MicroSD card. Includes an interactive touch precision calibration tool.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Anyone Can Draw Hypnotic Neon Art on This $15 Screen"
  * *Hook*: Turn anyone into a digital artist with real-time radial symmetry drawing and instant SD card image export.
* **Hardware & Pins**: LovyanGFX vector renderer, MicroSD BMP encoder, NVS calibration matrix.
* **3D Enclosure Concept**: *Mini Artist's Studio Easel* — Miniature wooden-filament A-frame easel holding the screen like an art canvas.

---

#### 48. Modular Multi-Page Smart Home & Sensor Starter Kit
* **Open-Source Base**: [Carlo47/CYD_MultipageMenu](https://github.com/Carlo47/CYD_MultipageMenu) & [Carlo47/CYD_Gui](https://github.com/Carlo47/CYD_Gui)
* **2.8" Porting Twist**: A modular plug-and-play beginner workbench. Automatically scans the I2C and GPIO ports (IO27/IO22) and renders live interactive gauges and graphs for connected Crowtail sensors (temperature, humidity, light, relays, distance).
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "How to Build Your First Touchscreen Gadget in 10 Minutes (Zero Soldering)"
  * *Hook*: The ultimate beginner guide to building custom touchscreen sensor stations with zero breadboards or soldering.
* **Hardware & Pins**: I2C auto-detection, Multi-page swipe gesture engine, Crowtail modular ports.
* **3D Enclosure Concept**: *Benchtop Lab Wedge with Sensor Rails* — Slanted console with side snap-rails to lock in two Crowtail sensors.

---

#### 49. Interactive 3D Printer Filament Drying & Spool Inventory Scale
* **Open-Source Base**: [BOlaerts/ESP32-2432s028](https://github.com/BOlaerts/ESP32-2432s028)
* **2.8" Porting Twist**: Interfaces an external HX711 load cell and SHT30 temperature/humidity sensor via the expansion pins. Features tare presets for major spool brands (Bambu Lab, Sunlu, Polymaker, eSun), calculates remaining meters of filament, and sounds moisture warnings.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Never Run Out of 3D Printer Filament Mid-Print Again"
  * *Hook*: Know exactly how many grams and meters of filament you have left before starting a 24-hour 3D print.
* **Hardware & Pins**: HX711 Load Cell on IO27/IO22, SHT30 I2C, Tare NVS presets, Buzzer alarm.
* **3D Enclosure Concept**: *Spool Roller Base with Scale* — Filament spool bearing rollers resting on load cells directly behind the display.

---

#### 50. No-Cloud Kitchen Timer, Meat Probe & Recipe Step Matrix
* **Open-Source Base**: [crowpanel Kitchen Touch community](https://github.com/topics/crowpanel)
* **2.8" Porting Twist**: 4-zone simultaneous kitchen timer with independent chimes. Built specifically for resistive touch so it can be tapped with wet, soapy, or flour-dusted fingers without false touches. Features MAX6675/NTC meat probe temp alerts and an SD-card recipe checklist.
* **IamNotIntoTech YouTube Hook**:
  * *Title*: "Why Phones Are Terrible in the Kitchen (So I Built a Wet-Finger-Proof Screen)"
  * *Hook*: Wet or dirty hands ruin smartphone screens. This tough kitchen counter timer and recipe prompter never misbehaves.
* **Hardware & Pins**: Resistive wet-finger touch, MAX6675 SPI probe, High-volume buzzer (IO26), MicroSD recipe reader.
* **3D Enclosure Concept**: *Magnetic Under-Cabinet Flip Mount* — Folds up under a kitchen cabinet when stored, or sticks to the stove hood with magnets.

---

## 📅 4. Implementation Batches & Roadmap

| Batch | Projects | Theme / Focus Area | Primary Libraries & Stack |
|---|---|---|---|
| **Batch 1** | Projects 01 – 08 | Offline SysAdmin & Homelab Appliances | `TFT_eSPI`, `LovyanGFX`, `PubSubClient`, `WiFi` |
| **Batch 2** | Projects 09 – 18 | Live Aviation, Space & RF Telemetry | `ArduinoJson`, `ESP-NOW`, `TinyGPSPlus`, `SGP4` |
| **Batch 3** | Projects 19 – 26 | RF Security, Diagnostics & Instruments | `ESP32-ADC-DMA`, `NimBLE`, `WiFi Promiscuous`, `OTA` |
| **Batch 4** | Projects 27 – 38 | Offline Knowledge, Smart Home & Media | `openHASP`, `TJpg_Decoder`, `ESP32-audioI2S`, `sqlite3` |
| **Batch 5** | Projects 39 – 50 | Retro Gaming, Generative Art & Life Tools| `LovyanGFX`, `MicroPython`, `Boids-Physics`, `HX711` |

---

## 📺 5. IamNotIntoTech Branding Guidelines

Every project generated in subsequent phases will adhere to these YouTube production standards:

1. **Header Attribution**:
   ```markdown
   📺 **Created for**: [IamNotIntoTech](https://youtube.com/@IamNotIntoTech)  
   🛠️ **Platform**: Elecrow CrowPanel 2.8" (ESP32-2432S028R)  
   💡 **Original Concept**: Inspired by open-source community innovation with custom feature engineering.
   ```
2. **On-Screen Splash Screen**:
   - Every `.ino` / C++ sketch boots with an authentic animated `IamNotIntoTech` intro sequence on the ILI9341 screen.
3. **Stand-Alone Portability**:
   - Every project folder contains its own self-contained `platformio.ini` with verified pin definitions so viewers can clone and flash with a single click.
