# 🖼️ Project 05 — CYD 2.8" MicroSD Photo Frame & Image Slideshow

> Turn your 2.8" Cheap Yellow Display (ESP32-2432S028R) into a digital desk photo frame. Scans your MicroSD card and cycles through images with automatic slideshow timing or manual touchscreen left/right swipe navigation.

---

## 📋 Overview

- **Difficulty**: ⭐⭐ Beginner (Level 2/5)
- **Supported Boards**: ESP32-2432S028R (2.8" Resistive Touch)
- **Required Hardware**: MicroSD Card (FAT32 formatted, up to 32GB)
- **Libraries Used**: `SPI`, `SD`, `TFT_eSPI`, `XPT2046_Touchscreen`

---

## 📺 Video Tutorials

- 🎬 **Watch the Build & Code Walkthrough**: [IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
- 🖨️ **3D Print the Photo Frame Desk Bezel**: [Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)

---

## ⚡ Features

- **Automatic Directory Scanning**: Automatically detects all `.bmp` and `.jpg` image files in the root of the SD card.
- **Touch Navigation**: Tap the left half of the display for the previous image, or tap the right half for the next image.
- **Auto-Advance Timer**: Automatically advances to the next picture every 6 seconds when left untouched.
- **Audio Sound Effects**: Audio chimes when navigating images.

---

## 🚀 How to Flash & Run

### 1. Prepare your MicroSD Card
- Format your MicroSD card as **FAT32**.
- Place your 320x240 images directly into the root folder (`/`).

### 2. Upload via PlatformIO:
```bash
cd projects/beginner/05-sd-slideshow
pio run -t upload
```

---

## 📜 License
Dual License: AGPL-3.0 for code & CC BY-NC-ND 4.0 for assets | Author: [@tushar2704](https://github.com/tushar2704)
