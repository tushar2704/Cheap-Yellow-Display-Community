# 📋 MASTER INSTRUCTIONS — Cheap Yellow Display Community

> **This file is for AI assistants, collaborators, and any automated tooling.**
> It defines the rules, conventions, and context needed to contribute to this repository.

---

## 🔐 Identity & Commit Rules

**ALL commits, pull requests, and authored content MUST use:**

```
Git Username: tushar2704
Git Email:    tushar.inseec@gmail.com
Full Name:    Tushar Aggarwal
GitHub:       https://github.com/tushar2704
```

**Configure git before any commit:**
```bash
git config user.name "tushar2704"
git config user.email "tushar.inseec@gmail.com"
```

**Never commit with any other identity.**

---

## 📺 YouTube Channels (ALWAYS Credit Both)

| Channel Handle | Channel Name | Focus |
|---------------|-------------|-------|
| `@IamNotIntoTech` | IamNotIntoTech | Tech projects with humor, ESP32 tutorials, CYD builds |
| `@Makerrdesigns` | Makerrdesigns | 3D design, 3D printing, enclosure designs for CYD |

**Links to include in every project README:**
```markdown
📺 **Tutorial**: [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
🖨️ **Enclosure**: [Watch on Makerrdesigns](https://youtube.com/@Makerrdesigns)
```

---

## 🏗️ Repository Context

### What This Repo Is
- The **#1 community hub** for ALL Cheap Yellow Display (CYD) variants
- Covers every screen size: 2.4", 2.8", 3.2", 3.5", 4.0", 5.0", 7.0"
- Covers every ESP32 variant: ESP32-WROOM, ESP32-S3, ESP32-C3
- Contains: projects, documentation, 3D enclosures, pin maps, and a web flasher

### What This Repo Is NOT
- NOT a fork or copy of any other repository
- NOT limited to a single CYD size (we cover ALL)
- NOT affiliated with or crediting any other community project

---

## 📁 File & Folder Conventions

### Project Structure
Every project MUST follow this exact structure:
```
projects/{difficulty}/{NN}-{project-slug}/
├── README.md            # REQUIRED: Description, screenshots, wiring, video link
├── {project-slug}.ino   # REQUIRED: Main Arduino sketch
├── platformio.ini       # REQUIRED: PlatformIO configuration
├── src/                 # Optional: Additional source files
│   └── main.cpp         # PlatformIO main (if different from .ino)
├── include/             # Optional: Header files
├── lib/                 # Optional: Local libraries
├── data/                # Optional: SPIFFS/LittleFS data
├── assets/              # Optional: Screenshots, demo GIFs
│   ├── screenshot.png
│   └── demo.gif
└── enclosure/           # Optional: 3D print files
    ├── case.stl
    └── case.step
```

### Difficulty Levels
| Folder | Level | Description |
|--------|-------|-------------|
| `beginner/` | ⭐–⭐⭐ | No external components, basic code |
| `intermediate/` | ⭐⭐⭐ | WiFi/API/MQTT, moderate complexity |
| `advanced/` | ⭐⭐⭐⭐+ | Complex systems, multi-component |

### Naming Conventions
- **Folders**: lowercase with hyphens (`weather-station`, not `WeatherStation`)
- **Arduino files**: lowercase with hyphens matching folder name
- **Documentation**: UPPERCASE with underscores (`SETUP.md`, `PINOUT.md`)
- **Images**: lowercase, descriptive (`cyd-2.8-pinout.png`)

---

## 📝 Project README Template

Every project README.md MUST contain these sections:

```markdown
# Project Name

> One-line description of what this project does.

![Screenshot or demo GIF](assets/screenshot.png)

## 📋 Overview
- What it does
- Which CYD variants it supports
- Difficulty rating (⭐ to ⭐⭐⭐⭐⭐)

## 🔧 Requirements
### Hardware
- List of required hardware

### Software / Libraries
- List of required libraries with links

## 📺 Video Tutorial
- 🎬 [Watch on IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 🖨️ [Enclosure build on Makerrdesigns](https://youtube.com/@Makerrdesigns)

## ⚡ Wiring / Pin Connections
Table of pin connections (if external components needed)

## 🚀 Installation
Step-by-step installation instructions for both Arduino IDE and PlatformIO

## 📸 Screenshots
Additional screenshots or GIFs

## 📜 License
MIT License — see [LICENSE](../../LICENSE)
```

---

## 🔍 SEO Rules

### README.md (Main)
- H1 MUST contain "Cheap Yellow Display"
- Use "CYD" and "Cheap Yellow Display" naturally throughout
- Include model numbers (ESP32-2432S028R etc.) for long-tail SEO
- Every section heading should contain a relevant keyword
- Include "ESP32", "touch screen", "TFT display" as secondary keywords

### Project READMEs
- Title should include "Cheap Yellow Display" or "CYD"
- Include "ESP32" and the specific CYD model in description
- Use descriptive alt text on all images

### GitHub Repository Settings
Apply these topics/tags:
```
cheap-yellow-display, cyd, esp32, esp32-s3, esp32-display,
touch-screen, tft-display, arduino, platformio, iot,
smart-home, esp32-projects, sunton-display, ili9341,
home-assistant, maker
```

---

## 💰 Monetization Context

### Pricing Tiers
| Type | Price | Distribution |
|------|-------|-------------|
| Beginner projects | Free | GitHub |
| Intermediate projects | $5 each | Gumroad |
| Advanced projects | $8–$10 each | Gumroad |
| Enclosure packs | $10 | Gumroad |
| Project bundles | $15–$45 | Gumroad |

### What "Sellable" Means
- Free projects: Full source code on GitHub, no restrictions
- Paid projects: Basic version on GitHub (free), premium version with extra features/enclosures/video on Gumroad
- NEVER gate basic functionality — the free version must be useful

---

## 🏷️ CYD Variant Quick Reference

When writing code that supports multiple CYD variants, use these defines:

```cpp
// Define your CYD variant before including headers
// Uncomment ONE of the following:

// #define CYD_24_RESISTIVE    // ESP32-2432S024 (2.4" Resistive)
// #define CYD_28_RESISTIVE    // ESP32-2432S028R (2.8" Resistive) — MOST COMMON
// #define CYD_28_CAPACITIVE   // ESP32-2432S028C (2.8" Capacitive)
// #define CYD_32_RESISTIVE    // ESP32-3248S032 (3.2" Resistive)
// #define CYD_35_RESISTIVE    // ESP32-3248S035R (3.5" Resistive)
// #define CYD_35_CAPACITIVE   // ESP32-3248S035C (3.5" Capacitive)
// #define CYD_40_CAPACITIVE   // ESP32-4048S040 (4.0" Capacitive, ESP32-S3)
// #define CYD_50_CAPACITIVE   // ESP32-8048S050 (5.0" Capacitive, ESP32-S3)
// #define CYD_70_CAPACITIVE   // ESP32-8048S070 (7.0" Capacitive, ESP32-S3)
```

---

## ✅ Quality Checklist

Before committing any project:

- [ ] Code compiles without errors
- [ ] README follows the template above
- [ ] Both YouTube channels are credited
- [ ] Screenshots/GIFs are included in `assets/`
- [ ] PlatformIO config is provided
- [ ] Arduino .ino file is provided
- [ ] License header or reference is included
- [ ] SEO keywords are present in README headings
- [ ] Git author is `tushar2704` / `tushar.inseec@gmail.com`

---

## 🔄 Commit Message Convention

```
type(scope): description

Examples:
feat(projects): add weather station project
docs(setup): update Arduino IDE installation steps
fix(hello-cyd): correct touch calibration for 3.5" variant
chore(ci): add GitHub Actions compile workflow
style(readme): improve SEO headings and keyword density
```

Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`

---

*Last updated: 2024 | Maintainer: tushar2704*
