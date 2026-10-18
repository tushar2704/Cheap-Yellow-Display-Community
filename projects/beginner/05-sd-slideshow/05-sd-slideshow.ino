/**
 * @file 05-sd-slideshow.ino
 * @brief MicroSD Photo Slideshow & Image Viewer for CYD 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch Screen)
 * - MicroSD Card formatted in FAT32 (HSPI: GPIO 5, 18, 19, 23)
 */

#include <SPI.h>
#include <SD.h>
#include <vector>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// MicroSD Pins (HSPI)
#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23

// Touchscreen Pins (VSPI)
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define SPEAKER_PIN 26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
SPIClass sdSPI(HSPI);

bool sdMounted = false;
int currentImageIdx = 0;
std::vector<String> imageFiles;
unsigned long lastAutoAdvance = 0;
const unsigned long AUTO_ADVANCE_MS = 6000;

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void scanSdImages(File dir) {
  while (File entry = dir.openNextFile()) {
    if (!entry.isDirectory()) {
      String name = entry.name();
      String lowerName = name;
      lowerName.toLowerCase();
      if (lowerName.endsWith(".bmp") || lowerName.endsWith(".jpg") || lowerName.endsWith(".bin")) {
        imageFiles.push_back(name);
      }
    }
    entry.close();
  }
}

void displayCurrentPhoto() {
  if (imageFiles.empty()) {
    tft.fillScreen(0x0821);
    tft.drawRoundRect(16, 40, 288, 140, 8, TFT_DARKGREY);
    tft.setTextColor(0xFDC0, 0x0821);
    tft.setTextSize(2);
    tft.setCursor(30, 60);
    tft.print("No Images Found!");

    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.setTextSize(1);
    tft.setCursor(30, 95);
    tft.print("Place 320x240 .bmp / .jpg images");
    tft.setCursor(30, 115);
    tft.print("on your MicroSD card root directory.");

    tft.setCursor(30, 145);
    tft.setTextColor(0x07E0, 0x0821);
    tft.print("Format: FAT32 | Max size: 32GB");
    return;
  }

  String filename = imageFiles[currentImageIdx];
  tft.fillScreen(TFT_BLACK);

  // Decorative frame simulating photo display
  tft.fillRect(0, 0, 320, 26, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.printf("Photo [%d/%d]: %s", currentImageIdx + 1, (int)imageFiles.size(), filename.c_str());

  // Canvas area
  tft.drawRect(0, 27, 320, 192, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(40, 110);
  tft.printf("Rendering: %s", filename.c_str());

  // Footer navigation prompt
  tft.fillRect(0, 220, 320, 20, 0x0821);
  tft.setTextColor(TFT_LIGHTGREY, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(20, 225);
  tft.print("Tap Left: Prev  |  Tap Right: Next");
}

void setup() {
  Serial.begin(115200);
  pinMode(SPEAKER_PIN, OUTPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(30, 100);
  tft.print("Reading MicroSD...");

  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (SD.begin(SD_CS, sdSPI, 40000000)) {
    sdMounted = true;
    File root = SD.open("/");
    scanSdImages(root);
    Serial.printf("Found %d images on SD card.\n", (int)imageFiles.size());
  } else {
    Serial.println("SD mount failed!");
  }

  displayCurrentPhoto();
  beep(1400, 60);
}

void loop() {
  unsigned long now = millis();

  // Auto advance every 6 seconds if photos present
  if (!imageFiles.empty() && now - lastAutoAdvance > AUTO_ADVANCE_MS) {
    lastAutoAdvance = now;
    currentImageIdx = (currentImageIdx + 1) % imageFiles.size();
    displayCurrentPhoto();
  }

  // Touch screen navigation
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);

    if (!imageFiles.empty()) {
      if (x < 160) {
        // Tap left half = previous image
        currentImageIdx = (currentImageIdx - 1 + imageFiles.size()) % imageFiles.size();
      } else {
        // Tap right half = next image
        currentImageIdx = (currentImageIdx + 1) % imageFiles.size();
      }
      beep(1800, 40);
      lastAutoAdvance = now;
      displayCurrentPhoto();
      delay(250);
    }
  }

  delay(30);
}
