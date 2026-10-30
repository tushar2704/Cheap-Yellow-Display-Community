/**
 * @file 07-bambu-telemetry-monitor.ino
 * @brief Bambu Lab Dual-Printer Telemetry Desk Monitor (Flicker-Free & Calibrated)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI: MOSI=13, MISO=12, CLK=14, CS=15)
 * - Touch: XPT2046 Resistive Touch (VSPI: MOSI=32, MISO=39, CLK=25, CS=33)
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

// Accurate CYD 2.8" Resistive Touch SPI Pinout
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

// Wi-Fi & MQTT Configuration
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";
const char* mqttServer = "192.168.1.60";
const int mqttPort = 1883;

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS); // Polling mode avoids GPIO 36 interrupt lockup
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
unsigned long footerClearTime = 0;

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void updateFooter(const char* message, uint16_t fg = TFT_WHITE, uint16_t bg = 0x18C3) {
  tft.fillRect(0, 224, 320, 16, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print(message);
}

void drawTabs() {
  // Top Header Banner
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 6);
  tft.print("BAMBU LAB MONITOR  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("@IamNotIntoTech");

  // 3 Tabs across screen (Width: 106 each)
  const char* tabTitles[3] = {"[1] X1-CARBON", "[2] P1S COMBO", "[3] DUAL SPLIT"};
  for (int i = 0; i < 3; i++) {
    int tx = i * 107;
    int tw = (i == 2) ? 106 : 106;
    bool isSel = (i == currentView);
    uint16_t bg = isSel ? 0xFDC0 : 0x2104;
    uint16_t fg = isSel ? TFT_BLACK : TFT_LIGHTGREY;
    uint16_t border = isSel ? TFT_WHITE : 0x4208;

    tft.fillRect(tx, 24, tw, 20, bg);
    tft.drawRect(tx, 24, tw, 20, border);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.setCursor(tx + 10, 29);
    tft.print(tabTitles[i]);
  }
}

void drawSinglePrinterStatic(const PrinterTelemetry& p) {
  tft.fillRect(0, 44, 320, 180, 0x0821);

  // Left Card: Print Progress
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

  // Progress Bar Outline
  tft.drawRect(cardX + 12, cardY + 76, 120, 12, TFT_WHITE);

  // Right Side: 3 Thermal & Hardware Cards
  int rightX = 160;
  int rw = 152;

  // 1. Nozzle Card
  tft.fillRoundRect(rightX, 50, rw, 52, 4, 0x0000);
  tft.drawRoundRect(rightX, 50, rw, 52, 4, 0xFD20); // Orange
  tft.setTextColor(0xFD20, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(rightX + 8, 56);
  tft.print("NOZZLE TEMPERATURE");

  // 2. Heatbed Card
  tft.fillRoundRect(rightX, 108, rw, 52, 4, 0x0000);
  tft.drawRoundRect(rightX, 108, rw, 52, 4, 0x041F); // Blue
  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(rightX + 8, 114);
  tft.print("HEATBED TEMPERATURE");

  // 3. Chamber & Status Card
  tft.fillRoundRect(rightX, 166, rw, 52, 4, 0x0000);
  tft.drawRoundRect(rightX, 166, rw, 52, 4, 0x07E0); // Green
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(rightX + 8, 172);
  tft.print("CHAMBER & HMS STATUS");
}

void updateSinglePrinterDynamic(const PrinterTelemetry& p) {
  int cardX = 8;
  int cardY = 50;
  int rightX = 160;

  // Big Percentage
  tft.fillRect(cardX + 16, cardY + 28, 120, 36, 0x0000);
  tft.setTextColor(p.hasError ? 0xF800 : (p.isPrinting ? TFT_WHITE : 0xFDC0), 0x0000);
  tft.setTextSize(4);
  tft.setCursor(cardX + 18, cardY + 32);
  tft.printf("%d%%", p.progressPercent);

  // Progress Bar Fill
  int barW = 120;
  int fillW = map(p.progressPercent, 0, 100, 0, barW - 2);
  fillW = constrain(fillW, 0, barW - 2);
  tft.fillRect(cardX + 13, cardY + 77, barW - 2, 10, 0x0000);
  if (fillW > 0) {
    tft.fillRect(cardX + 13, cardY + 77, fillW, 10, p.isPrinting ? 0x07E0 : 0xFDC0);
  }

  // ETA & Layers
  tft.fillRect(cardX + 10, cardY + 98, 126, 60, 0x0000);
  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(cardX + 10, cardY + 100);
  if (p.isPrinting) {
    tft.printf("ETA: %dh %02dm left", p.remainingMins / 60, p.remainingMins % 60);
  } else {
    tft.print("STATUS: PAUSED");
  }

  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(cardX + 10, cardY + 118);
  tft.printf("Layer: %d / %d", p.currentLayer, p.totalLayers);

  float cost = p.filamentGrams * 0.025f;
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(cardX + 10, cardY + 138);
  tft.printf("Filament: %.1fg", p.filamentGrams);
  tft.setCursor(cardX + 10, cardY + 152);
  tft.printf("Job Cost: $%.2f", cost);

  // Nozzle Actual & Target
  tft.fillRect(rightX + 8, 70, 136, 26, 0x0000);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(rightX + 8, 72);
  tft.printf("%d C", p.nozzleActual);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(rightX + 74, 76);
  tft.printf("(T: %d C)", p.nozzleTarget);

  // Bed Actual & Target
  tft.fillRect(rightX + 8, 128, 136, 26, 0x0000);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(rightX + 8, 130);
  tft.printf("%d C", p.bedActual);
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, 0x0000);
  tft.setCursor(rightX + 74, 134);
  tft.printf("(T: %d C)", p.bedTarget);

  // Chamber Temp & Status
  tft.fillRect(rightX + 8, 186, 136, 26, 0x0000);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(rightX + 8, 188);
  tft.printf("Chamber Temp: %d C", p.chamberTemp);
  tft.setCursor(rightX + 8, 202);
  tft.setTextColor(p.hasError ? 0xF800 : 0x07E0, 0x0000);
  tft.print(p.hasError ? "HMS: ERROR DETECTED" : "HMS: ALL HEALTHY");
}

void drawDualSplitStatic() {
  tft.fillRect(0, 44, 320, 180, 0x0821);

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

    // Mini Progress Bar outline
    tft.drawRect(cx + 8, cy + 62, 134, 8, TFT_WHITE);
  }
}

void updateDualSplitDynamic() {
  for (int i = 0; i < 2; i++) {
    const PrinterTelemetry& p = (i == 0) ? printer1 : printer2;
    int cx = 6 + i * 156;
    int cy = 48;

    // Progress %
    tft.fillRect(cx + 10, cy + 26, 120, 30, 0x0000);
    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.setTextSize(3);
    tft.setCursor(cx + 12, cy + 28);
    tft.printf("%d%%", p.progressPercent);

    // Mini Progress Bar Fill
    int fw = map(p.progressPercent, 0, 100, 0, 132);
    fw = constrain(fw, 0, 132);
    tft.fillRect(cx + 9, cy + 63, 132, 6, 0x0000);
    if (fw > 0) {
      tft.fillRect(cx + 9, cy + 63, fw, 6, (i == 0) ? 0x07E0 : 0x07FF);
    }

    // Telemetry text rows
    tft.fillRect(cx + 8, cy + 76, 136, 90, 0x0000);
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
    tft.printf("Cost: $%.2f (%.1fg)", p.filamentGrams * 0.025f, p.filamentGrams);
  }
}

void renderCurrentView() {
  drawTabs();
  if (currentView == VIEW_PRINTER_1) {
    drawSinglePrinterStatic(printer1);
    updateSinglePrinterDynamic(printer1);
    updateFooter("X1-Carbon Isolated View | Tap cards to interact | @IamNotIntoTech");
  } else if (currentView == VIEW_PRINTER_2) {
    drawSinglePrinterStatic(printer2);
    updateSinglePrinterDynamic(printer2);
    updateFooter("P1S Combo Isolated View | Tap cards to interact | @IamNotIntoTech");
  } else {
    drawDualSplitStatic();
    updateDualSplitDynamic();
    updateFooter("Dual Split Telemetry | Tap either printer | @IamNotIntoTech");
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
  setRgbLed(true, false, false); // Red = Init

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  // Dedicated CYD 2.8" Resistive Touch initialization
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Background Wi-Fi setup (non-blocking)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Splash Screen
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

  tft.fillScreen(0x0821);
  renderCurrentView();
  setRgbLed(false, true, false); // Green = Ready
}

void loop() {
  // Clear temporary action messages in footer
  if (footerClearTime > 0 && millis() > footerClearTime) {
    footerClearTime = 0;
    updateFooter((currentView == VIEW_DUAL) ? "Dual Split View | Tap tabs above | @IamNotIntoTech" : "Single View | Tap cards to interact | @IamNotIntoTech");
  }

  // Smooth, flicker-free telemetry simulation update
  if (millis() - lastSimUpdate > 2500) {
    lastSimUpdate = millis();

    // Advance Printer 1
    if (printer1.isPrinting && printer1.progressPercent < 100) {
      printer1.progressPercent++;
      printer1.filamentGrams += 0.2f;
      if (printer1.remainingMins > 0) printer1.remainingMins--;
      if (printer1.currentLayer < printer1.totalLayers) printer1.currentLayer++;
      printer1.nozzleActual = printer1.nozzleTarget + random(-1, 2);
      printer1.bedActual = printer1.bedTarget + random(-1, 2);
    }

    // Advance Printer 2
    if (printer2.isPrinting && printer2.progressPercent < 100) {
      printer2.progressPercent++;
      printer2.filamentGrams += 0.3f;
      if (printer2.remainingMins > 0) printer2.remainingMins--;
      if (printer2.currentLayer < printer2.totalLayers) printer2.currentLayer++;
      printer2.nozzleActual = printer2.nozzleTarget + random(-1, 2);
      printer2.bedActual = printer2.bedTarget + random(-1, 2);
    }

    // Update only dynamic areas (no full screen flicker)
    if (currentView == VIEW_PRINTER_1) {
      updateSinglePrinterDynamic(printer1);
    } else if (currentView == VIEW_PRINTER_2) {
      updateSinglePrinterDynamic(printer2);
    } else {
      updateDualSplitDynamic();
    }
  }

  // Touch Handling (calibrated, non-inverted Y)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 200) {
        lastTouchTime = millis();

        // Elecrow 2.8" CYD Landscape calibration (non-inverted Y)
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, screenX, screenY);

        // 1. Tab Bar Navigation (Y: 22 to 46)
        if (screenY >= 22 && screenY <= 46) {
          int clickedTab = screenX / 107;
          if (clickedTab >= 0 && clickedTab < 3) {
            playBeep(2000, 40);
            currentView = clickedTab;
            renderCurrentView();
            return;
          }
        }

        // 2. Interactive Touches for Single Printer View
        if (currentView == VIEW_PRINTER_1 || currentView == VIEW_PRINTER_2) {
          PrinterTelemetry& pRef = (currentView == VIEW_PRINTER_1) ? printer1 : printer2;

          // Tap Progress Card (X: 8 to 152, Y: 50 to 218) -> Toggle Pause/Resume
          if (screenX >= 8 && screenX <= 152 && screenY >= 50 && screenY <= 218) {
            pRef.isPrinting = !pRef.isPrinting;
            playBeep(pRef.isPrinting ? 2200 : 1200, 60);
            setRgbLed(pRef.isPrinting ? false : true, true, false);
            char buf[64];
            snprintf(buf, sizeof(buf), "⚡ [%s]: Print Job %s", pRef.model, pRef.isPrinting ? "RESUMED" : "PAUSED");
            updateFooter(buf, TFT_BLACK, pRef.isPrinting ? 0x07E0 : 0xFDC0);
            footerClearTime = millis() + 3000;
            updateSinglePrinterDynamic(pRef);
            return;
          }

          // Tap Nozzle Card (X: 160 to 314, Y: 50 to 102) -> Cycle Nozzle Preheat
          if (screenX >= 160 && screenY >= 50 && screenY <= 102) {
            playBeep(1800, 40);
            if (pRef.nozzleTarget == 220) pRef.nozzleTarget = 250;      // PETG
            else if (pRef.nozzleTarget == 250) pRef.nozzleTarget = 0;   // Cooldown
            else pRef.nozzleTarget = 220;                               // PLA
            pRef.nozzleActual = pRef.nozzleTarget;
            char buf[64];
            snprintf(buf, sizeof(buf), "🔥 Nozzle Target Set: %d C (%s)", pRef.nozzleTarget, pRef.nozzleTarget == 250 ? "PETG" : (pRef.nozzleTarget == 220 ? "PLA" : "Cool"));
            updateFooter(buf, TFT_BLACK, 0xFD20);
            footerClearTime = millis() + 3000;
            updateSinglePrinterDynamic(pRef);
            return;
          }

          // Tap Heatbed Card (X: 160 to 314, Y: 108 to 160) -> Cycle Bed Temp
          if (screenX >= 160 && screenY >= 108 && screenY <= 160) {
            playBeep(1700, 40);
            if (pRef.bedTarget == 55) pRef.bedTarget = 80;       // Engineering plate
            else if (pRef.bedTarget == 80) pRef.bedTarget = 0;  // Cooldown
            else pRef.bedTarget = 55;                            // Textured PEI
            pRef.bedActual = pRef.bedTarget;
            char buf[64];
            snprintf(buf, sizeof(buf), "🛏️ Heatbed Target Set: %d C", pRef.bedTarget);
            updateFooter(buf, TFT_BLACK, 0x07FF);
            footerClearTime = millis() + 3000;
            updateSinglePrinterDynamic(pRef);
            return;
          }

          // Tap HMS Card (X: 160 to 314, Y: 166 to 218) -> Toggle Test Error
          if (screenX >= 160 && screenY >= 166 && screenY <= 218) {
            pRef.hasError = !pRef.hasError;
            playBeep(pRef.hasError ? 600 : 2400, 80);
            setRgbLed(pRef.hasError ? true : false, pRef.hasError ? false : true, false);
            updateFooter(pRef.hasError ? "⚠️ HMS ALERT: Filament Runout Sensor Tripped" : "✅ HMS CHECK: All Subsystems Nominal", TFT_WHITE, pRef.hasError ? 0xF800 : 0x03E0);
            footerClearTime = millis() + 3500;
            updateSinglePrinterDynamic(pRef);
            return;
          }
        }
        // 3. Interactive Touches for Dual Split View
        else if (currentView == VIEW_DUAL) {
          if (screenY >= 48 && screenY <= 220) {
            playBeep(1900, 40);
            if (screenX < 160) {
              currentView = VIEW_PRINTER_1;
            } else {
              currentView = VIEW_PRINTER_2;
            }
            renderCurrentView();
            return;
          }
        }

        // 4. Header Tap
        if (screenY < 22) {
          playBeep(1500, 30);
          updateFooter("MQTT: 192.168.1.60:1883 | SSL Ready | @IamNotIntoTech", TFT_YELLOW, 0x0010);
          footerClearTime = millis() + 3000;
          return;
        }
      }
    }
  }

  delay(20);
}
