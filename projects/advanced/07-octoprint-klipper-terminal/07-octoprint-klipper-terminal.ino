/**
 * @file 07-octoprint-klipper-terminal.ino
 * @brief OctoPrint & Klipper/Moonraker 3D Printer Touch Terminal for Cheap Yellow Display 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch CYD)
 * - Onboard RGB LED (GPIO 4, 16, 17)
 * - Onboard Speaker (GPIO 26)
 * - Onboard LDR Sensor (GPIO 34)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Enter your WiFi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// 3D Printer Host (OctoPrint / Moonraker)
const char* PRINTER_HOST  = "http://192.168.1.150";
const char* API_KEY       = "YOUR_OCTOPRINT_OR_MOONRAKER_KEY";

// Hardware Pins (ESP32-2432S028R)
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26
#define LDR_PIN 34
#define TFT_BL 21

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Telemetry State
struct PrinterStatus {
  String stateStr;      // "Printing", "Paused", "Operational"
  String jobFile;
  float progressPct;
  float nozzleActual;
  float nozzleTarget;
  float bedActual;
  float bedTarget;
  int printTimeSec;
  int printTimeLeftSec;
  bool chamberLight;
};

PrinterStatus printer = {
  "Printing",
  "CYD-Stand-V2.gcode",
  64.5,
  215.2,
  215.0,
  60.1,
  60.0,
  3420,
  1880,
  true
};

unsigned long lastPollTime = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void updateLedState() {
  if (printer.stateStr == "Printing") {
    setRgb(false, true, true); // Cyan while printing
  } else if (printer.stateStr == "Paused") {
    setRgb(true, true, false); // Yellow while paused
  } else if (printer.stateStr == "Operational") {
    setRgb(false, true, false); // Green standby
  } else {
    setRgb(true, false, false); // Red error
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 32, 0x18C3);
  tft.setTextColor(0xFDC0, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(10, 11);
  tft.print("🖨️ Klipper / OctoPrint Terminal");

  // State Badge
  uint16_t badgeBg = (printer.stateStr == "Printing") ? 0x07E0 : 0xFDC0;
  tft.setTextColor(TFT_BLACK, badgeBg);
  tft.setCursor(225, 11);
  tft.printf(" %s ", printer.stateStr.c_str());
}

void drawThermalCard() {
  // Left Box: Hotend & Bed Temps
  tft.fillRoundRect(10, 38, 145, 96, 6, 0x1082);
  tft.drawRoundRect(10, 38, 145, 96, 6, 0x31A6);

  // Nozzle
  tft.setTextColor(0xFD20, 0x1082); // Orange
  tft.setTextSize(1);
  tft.setCursor(18, 46);
  tft.print("NOZZLE HOTEND");
  tft.setTextSize(2);
  tft.setCursor(18, 58);
  tft.printf("%.0f C", printer.nozzleActual);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, 0x1082);
  tft.printf(" / %.0f", printer.nozzleTarget);

  // Bed
  tft.setTextColor(0x07FF, 0x1082); // Cyan
  tft.setCursor(18, 86);
  tft.print("HEATED BED");
  tft.setTextSize(2);
  tft.setCursor(18, 98);
  tft.printf("%.0f C", printer.bedActual);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, 0x1082);
  tft.printf(" / %.0f", printer.bedTarget);
}

void drawJobProgressCard() {
  // Right Box: Job Details & Progress
  tft.fillRoundRect(165, 38, 145, 96, 6, 0x1082);
  tft.drawRoundRect(165, 38, 145, 96, 6, 0x31A6);

  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setTextSize(1);
  tft.setCursor(173, 46);
  tft.print("ACTIVE PRINT");

  tft.setTextColor(0x07E0, 0x1082);
  tft.setCursor(173, 58);
  String f = printer.jobFile;
  if (f.length() > 14) f = f.substring(0, 14);
  tft.print(f);

  // Big Percentage
  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(2);
  tft.setCursor(173, 76);
  tft.printf("%.1f%%", printer.progressPct);

  // Time Left
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, 0x1082);
  tft.setCursor(173, 100);
  int minLeft = printer.printTimeLeftSec / 60;
  tft.printf("Left: %dm %ds", minLeft, printer.printTimeLeftSec % 60);

  // Progress Bar under cards (10, 140, 300, 12)
  tft.fillRoundRect(10, 140, 300, 12, 4, 0x2124);
  int barW = map((int)printer.progressPct, 0, 100, 0, 300);
  tft.fillRoundRect(10, 140, barW, 12, 4, 0x07E0);
}

void drawControlButtons() {
  // Button 1: PAUSE / RESUME (10, 160, 94, 52)
  uint16_t b1Col = (printer.stateStr == "Paused") ? 0x07E0 : 0xFDC0;
  tft.fillRoundRect(10, 160, 94, 52, 6, b1Col);
  tft.drawRoundRect(10, 160, 94, 52, 6, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, b1Col);
  tft.setTextSize(1);
  tft.setCursor(24, 182);
  tft.print(printer.stateStr == "Paused" ? "▶ RESUME" : "⏸ PAUSE");

  // Button 2: CANCEL / STOP (114, 160, 94, 52)
  tft.fillRoundRect(114, 160, 94, 52, 6, 0xF800);
  tft.drawRoundRect(114, 160, 94, 52, 6, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, 0xF800);
  tft.setCursor(132, 182);
  tft.print("⏹ CANCEL");

  // Button 3: LIGHT / FAN (218, 160, 92, 52)
  uint16_t b3Col = printer.chamberLight ? 0x07FF : 0x2104;
  tft.fillRoundRect(218, 160, 92, 52, 6, b3Col);
  tft.drawRoundRect(218, 160, 92, 52, 6, TFT_WHITE);
  tft.setTextColor(printer.chamberLight ? TFT_BLACK : TFT_WHITE, b3Col);
  tft.setCursor(232, 182);
  tft.print(printer.chamberLight ? "💡 LIGHT ON" : "💡 LIGHT OFF");
}

void drawFooter() {
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
}

void drawFullUI() {
  tft.fillScreen(0x0000);
  drawHeader();
  drawThermalCard();
  drawJobProgressCard();
  drawControlButtons();
  drawFooter();
  updateLedState();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0000);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Splash
  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.print("Connecting 3D Printer...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 15) {
    delay(200);
    retry++;
  }

  drawFullUI();
  beep(1900, 60);
}

void loop() {
  // Handle Touch Screen
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Button 1: PAUSE / RESUME (10, 160, 94, 52)
    if (x >= 10 && x <= 104 && y >= 160 && y <= 212) {
      beep(1800, 35);
      if (printer.stateStr == "Printing") {
        printer.stateStr = "Paused";
      } else {
        printer.stateStr = "Printing";
      }
      drawHeader();
      drawControlButtons();
      updateLedState();
      delay(200);
    }
    // Button 2: CANCEL (114, 160, 94, 52)
    else if (x >= 114 && x <= 208 && y >= 160 && y <= 212) {
      beep(1200, 70);
      printer.stateStr = "Operational";
      printer.progressPct = 0.0;
      drawHeader();
      drawJobProgressCard();
      drawControlButtons();
      updateLedState();
      delay(200);
    }
    // Button 3: LIGHT (218, 160, 92, 52)
    else if (x >= 218 && x <= 310 && y >= 160 && y <= 212) {
      beep(2200, 30);
      printer.chamberLight = !printer.chamberLight;
      drawControlButtons();
      delay(200);
    }
    // Tap Thermal Card -> re-heat bump
    else if (x >= 10 && x <= 155 && y >= 38 && y <= 134) {
      beep(2000, 30);
      printer.nozzleActual = 215.0 + (random(-5, 6) / 10.0);
      printer.bedActual    = 60.0 + (random(-3, 4) / 10.0);
      drawThermalCard();
      delay(200);
    }
  }

  // Periodic simulated print progress progression
  if (millis() - lastPollTime > 3000) {
    lastPollTime = millis();
    if (printer.stateStr == "Printing") {
      printer.progressPct += 0.3;
      if (printer.progressPct > 100.0) printer.progressPct = 100.0;
      if (printer.printTimeLeftSec > 5) printer.printTimeLeftSec -= 3;
      printer.nozzleActual = 215.0 + (random(-8, 9) / 10.0);
      printer.bedActual    = 60.0 + (random(-4, 5) / 10.0);

      drawThermalCard();
      drawJobProgressCard();
    }
  }

  delay(25);
}
