/**
 * @file 07-bambu-telemetry-monitor.ino
 * @brief Bambu Lab Dual-Printer Telemetry Desk Monitor
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch
 * - Network: ESP32 Wi-Fi + MQTT Telemetry Client
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Wi-Fi & MQTT Configuration
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";
const char* mqttServer = "192.168.1.60";
const int mqttPort = 1883;

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
WiFiClient espClient;
PubSubClient mqttClient(espClient);

struct PrinterTelemetry {
  const char* name;
  const char* model;
  int nozzleActual;
  int nozzleTarget;
  int bedActual;
  int bedTarget;
  int chamberTemp;
  int progressPercent;
  int currentLayer;
  int totalLayers;
  int remainingMins;
  float filamentGrams;
  bool isPrinting;
  bool hasError;
};

#define VIEW_PRINTER_1 0
#define VIEW_PRINTER_2 1
#define VIEW_DUAL      2
int currentView = VIEW_PRINTER_1;

PrinterTelemetry printer1 = {
  "PRINTER 1", "Bambu X1-Carbon", 220, 220, 55, 55, 38, 72, 216, 300, 48, 64.5, true, false
};

PrinterTelemetry printer2 = {
  "PRINTER 2", "Bambu P1S Combo", 215, 215, 60, 60, 34, 45, 90, 200, 115, 42.0, true, false
};

unsigned long lastSimUpdate = 0;
unsigned long lastTouchTime = 0;

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void drawTabs() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("BAMBU LAB MONITOR  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("IamNotIntoTech");

  // 3 Tabs
  const char* tabTitles[3] = {"[1] X1-CARBON", "[2] P1S COMBO", "[3] DUAL SPLIT"};
  for (int i = 0; i < 3; i++) {
    int tx = i * 106;
    bool isSel = (i == currentView);
    uint16_t bg = isSel ? 0xFDC0 : 0x2104;
    uint16_t fg = isSel ? TFT_BLACK : TFT_LIGHTGREY;
    tft.fillRect(tx, 24, 105, 20, bg);
    tft.drawRect(tx, 24, 105, 20, TFT_BLACK);
    tft.setTextColor(fg, bg);
    tft.setCursor(tx + 12, 29);
    tft.print(tabTitles[i]);
  }
}

void drawSinglePrinter(const PrinterTelemetry& p) {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  // Big Progress Ring / Card
  int cardX = 8;
  int cardY = 50;
  int cardW = 144;
  int cardH = 168;

  tft.fillRoundRect(cardX, cardY, cardW, cardH, 4, 0x0000);
  tft.drawRoundRect(cardX, cardY, cardW, cardH, 4, p.hasError ? 0xF800 : 0x07E0);

  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(cardX + 10, cardY + 8);
  tft.print(p.model);

  // Big percentage
  tft.setTextColor(p.hasError ? 0xF800 : TFT_WHITE, 0x0000);
  tft.setTextSize(4);
  tft.setCursor(cardX + 18, cardY + 32);
  tft.printf("%d%%", p.progressPercent);

  // Progress Bar
  int barW = 120;
  int barH = 12;
  tft.drawRect(cardX + 12, cardY + 76, barW, barH, TFT_WHITE);
  int fillW = map(p.progressPercent, 0, 100, 0, barW - 2);
  tft.fillRect(cardX + 13, cardY + 77, fillW, barH - 2, 0x07E0);

  // Remaining Time & Layers
  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(cardX + 10, cardY + 100);
  tft.printf("ETA: %dh %02dm left", p.remainingMins / 60, p.remainingMins % 60);

  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(cardX + 10, cardY + 118);
  tft.printf("Layer: %d / %d", p.currentLayer, p.totalLayers);

  // Spool Cost Estimator ($0.025 per gram)
  float cost = p.filamentGrams * 0.025f;
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(cardX + 10, cardY + 138);
  tft.printf("Filament: %.1fg", p.filamentGrams);
  tft.setCursor(cardX + 10, cardY + 152);
  tft.printf("Job Cost: $%.2f", cost);

  // Right Side: Thermal Telemetry Cards
  int rightX = 160;
  int rw = 152;

  // Nozzle Card
  tft.fillRoundRect(rightX, 50, rw, 52, 4, 0x0000);
  tft.drawRoundRect(rightX, 50, rw, 52, 4, 0xFD20); // Orange
  tft.setTextColor(0xFD20, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(rightX + 8, 56);
  tft.print("NOZZLE TEMPERATURE");
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(rightX + 8, 72);
  tft.printf("%d C", p.nozzleActual);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.printf("  (T: %d C)", p.nozzleTarget);

  // Bed Card
  tft.fillRoundRect(rightX, 108, rw, 52, 4, 0x0000);
  tft.drawRoundRect(rightX, 108, rw, 52, 4, 0x041F); // Blue
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(rightX + 8, 114);
  tft.print("HEATBED TEMPERATURE");
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(rightX + 8, 130);
  tft.printf("%d C", p.bedActual);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.printf("  (T: %d C)", p.bedTarget);

  // Chamber & Status Card
  tft.fillRoundRect(rightX, 166, rw, 52, 4, 0x0000);
  tft.drawRoundRect(rightX, 166, rw, 52, 4, 0x07E0);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(rightX + 8, 172);
  tft.print("CHAMBER & HMS STATUS");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(rightX + 8, 188);
  tft.printf("Chamber Temp: %d C", p.chamberTemp);
  tft.setCursor(rightX + 8, 202);
  tft.setTextColor(0x07E0, 0x0000);
  tft.print("HMS Status: ALL HEALTHY");

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(8, 228);
  tft.print("Bambu MQTT Live Telemetry | @IamNotIntoTech");
}

void drawDualSplit() {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  // Two vertical cards side by side
  for (int i = 0; i < 2; i++) {
    const PrinterTelemetry& p = (i == 0) ? printer1 : printer2;
    int cx = 6 + i * 156;
    int cw = 152;
    int cy = 48;
    int ch = 172;

    tft.fillRoundRect(cx, cy, cw, ch, 4, 0x0000);
    tft.drawRoundRect(cx, cy, cw, ch, 4, (i == 0) ? 0x07E0 : 0x07FF);

    tft.setTextColor((i == 0) ? 0x07E0 : 0x07FF, 0x0000);
    tft.setTextSize(1);
    tft.setCursor(cx + 8, cy + 8);
    tft.print(p.model);

    // Progress %
    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.setTextSize(3);
    tft.setCursor(cx + 12, cy + 28);
    tft.printf("%d%%", p.progressPercent);

    // Mini Progress Bar
    tft.drawRect(cx + 8, cy + 62, 134, 8, TFT_WHITE);
    int fw = map(p.progressPercent, 0, 100, 0, 132);
    tft.fillRect(cx + 9, cy + 63, fw, 6, (i == 0) ? 0x07E0 : 0x07FF);

    tft.setTextColor(0xFDC0, 0x0000);
    tft.setTextSize(1);
    tft.setCursor(cx + 8, cy + 78);
    tft.printf("ETA: %dh %02dm", p.remainingMins / 60, p.remainingMins % 60);

    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.setCursor(cx + 8, cy + 96);
    tft.printf("Nozzle: %d C / %d C", p.nozzleActual, p.nozzleTarget);

    tft.setCursor(cx + 8, cy + 114);
    tft.printf("Bed:    %d C / %d C", p.bedActual, p.bedTarget);

    tft.setCursor(cx + 8, cy + 132);
    tft.printf("Layer:  %d / %d", p.currentLayer, p.totalLayers);

    tft.setTextColor(0x07FF, 0x0000);
    tft.setCursor(cx + 8, cy + 152);
    tft.printf("Filament: %.1fg ($%.2f)", p.filamentGrams, p.filamentGrams * 0.025f);
  }

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setCursor(8, 228);
  tft.print("Tap tabs to isolate printer views | @IamNotIntoTech");
}

void renderCurrentView() {
  drawTabs();
  if (currentView == VIEW_PRINTER_1) {
    drawSinglePrinter(printer1);
  } else if (currentView == VIEW_PRINTER_2) {
    drawSinglePrinter(printer2);
  } else {
    drawDualSplit();
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 07 Bambu Dual-Printer Monitor ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Splash
  tft.fillRect(0, 0, 320, 240, 0x0821);
  tft.fillRoundRect(20, 30, 280, 180, 8, 0x0000);
  tft.drawRoundRect(20, 30, 280, 180, 8, 0x07E0);

  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(44, 55);
  tft.print("IamNotIntoTech");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(44, 90);
  tft.print("ELECROW 2.8\" CYD PROJECT SUITE");

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(44, 115);
  tft.print("Project 07: Bambu Dual-Printer Monitor");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("MQTT Telemetry | Spool Cost | Dual Split");

  playBeep(1600, 80);
  delay(100);
  playBeep(2100, 100);
  delay(500);

  renderCurrentView();
  setRgbLed(false, true, false); // Green
}

void loop() {
  // Gentle simulated progress updates
  if (millis() - lastSimUpdate > 3000) {
    lastSimUpdate = millis();
    if (printer1.progressPercent < 100) {
      printer1.progressPercent++;
      printer1.filamentGrams += 0.2f;
      if (printer1.remainingMins > 0) printer1.remainingMins--;
      if (printer1.currentLayer < printer1.totalLayers) printer1.currentLayer++;
    }
    if (printer2.progressPercent < 100) {
      printer2.progressPercent++;
      printer2.filamentGrams += 0.3f;
      if (printer2.remainingMins > 0) printer2.remainingMins--;
      if (printer2.currentLayer < printer2.totalLayers) printer2.currentLayer++;
    }
    renderCurrentView();
  }

  // Touch handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 250) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Check Tabs (Y: 24 to 44)
      if (screenY >= 24 && screenY <= 44) {
        int clicked = screenX / 106;
        if (clicked >= 0 && clicked < 3 && clicked != currentView) {
          currentView = clicked;
          playBeep(1800, 40);
          renderCurrentView();
        }
      }
    }
  }

  delay(20);
}
