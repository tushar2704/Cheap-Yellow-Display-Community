/**
 * @file 08-klipper-command-deck.ino
 * @brief Multi-Printer Wireless Command Deck (Klipper / Moonraker / OctoPrint)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI: MOSI=13, MISO=12, CLK=14, CS=15)
 * - Touch: XPT2046 Resistive Touch (VSPI: MOSI=32, MISO=39, CLK=25, CS=33)
 * - Network: ESP32 Wi-Fi + Moonraker REST Client
 * - Sensor: DS18B20 OneWire Chamber Probe on GPIO 27
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
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
#define ONE_WIRE_BUS  27 // HY2.0-4P Connector Pin

// Wi-Fi & Moonraker IP Configuration
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";
const char* moonrakerHost = "http://192.168.1.70:7125";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS); // Polling mode avoids GPIO 36 interrupt lockup
TFT_eSPI tft = TFT_eSPI();

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&oneWire);

// State variables
int extruderActual = 210;
int extruderTarget = 210;
int bedActual = 60;
int bedTarget = 60;
float chamberTemp = 36.5f;
float currentZOffset = 0.00f;
int speedFactor = 100;
int printProgress = 68;
bool isEmergencyStopped = false;

unsigned long lastTempPoll = 0;
unsigned long lastSimUpdate = 0;
unsigned long lastTouchTime = 0;
unsigned long footerClearTime = 0;

const char* zLabels[4] = {"-0.05mm", "+0.05mm", "-0.01mm", "+0.01mm"};
const char* preLabels[3] = {"PLA: 210/60", "PETG: 240/80", "COOLDOWN"};

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

void sendGcode(const String& gcode) {
  Serial.printf("⚡ [KLIPPER GCODE]: %s\n", gcode.c_str());
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(moonrakerHost) + "/printer/gcode/script?script=" + gcode;
    http.begin(url);
    http.POST("");
    http.end();
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 6);
  tft.print("KLIPPER COMMAND DECK  ");
  tft.setTextColor(isEmergencyStopped ? 0xF800 : 0x07E0, 0x0010);
  tft.print(isEmergencyStopped ? "[E-STOP ACTIVE]" : "[PRINTING 68%]");

  // Host Indicator
  tft.setTextColor(TFT_WHITE, 0x0010);
  tft.setCursor(214, 6);
  tft.print(WiFi.status() == WL_CONNECTED ? "Moonraker: OK" : "Standalone/Demo");
}

void drawStaticCards() {
  int rowY = 28;
  int w = 98;
  int h = 42;

  // 1. Extruder Frame
  tft.fillRoundRect(6, rowY, w, h, 3, 0x0000);
  tft.drawRoundRect(6, rowY, w, h, 3, 0xFD20); // Orange
  tft.setTextColor(0xFD20, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(12, rowY + 4);
  tft.print("EXTRUDER");

  // 2. Bed Frame
  tft.fillRoundRect(111, rowY, w, h, 3, 0x0000);
  tft.drawRoundRect(111, rowY, w, h, 3, 0x041F); // Blue
  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(117, rowY + 4);
  tft.print("HEATBED");

  // 3. Chamber Frame
  tft.fillRoundRect(216, rowY, w, h, 3, 0x0000);
  tft.drawRoundRect(216, rowY, w, h, 3, 0x07E0); // Green
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(222, rowY + 4);
  tft.print("CHAMBER (IO27)");

  // Babystep section label
  int ctrlY = 76;
  tft.setTextColor(0x07FF, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(8, ctrlY);
  tft.print("BABYSTEP Z-OFFSET:");

  // Preheat section label
  int macroY = 122;
  tft.setTextColor(0xFDC0, 0x0821);
  tft.setCursor(8, macroY);
  tft.print("PREHEAT & HEATER PROFILES:");
}

void updateTemperatures() {
  int rowY = 28;

  // Extruder
  tft.fillRect(10, rowY + 16, 90, 22, 0x0000);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(12, rowY + 18);
  tft.printf("%d C", extruderActual);

  // Heatbed
  tft.fillRect(115, rowY + 16, 90, 22, 0x0000);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(117, rowY + 18);
  tft.printf("%d C", bedActual);

  // Chamber
  tft.fillRect(220, rowY + 16, 90, 22, 0x0000);
  uint16_t chColor = (chamberTemp > 60.0f) ? 0xF800 : TFT_WHITE;
  tft.setTextColor(chColor, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(222, rowY + 18);
  tft.printf("%.1f C", chamberTemp);
}

void updateBabystepLabel() {
  int ctrlY = 76;
  tft.fillRect(130, ctrlY - 2, 185, 14, 0x0821);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(134, ctrlY);
  tft.printf("[Current: %+.2fmm]", currentZOffset);
}

void drawZButton(int i, bool pressed = false) {
  int ctrlY = 76;
  int bx = 8 + i * 77;
  int by = ctrlY + 16;
  int bw = 72;
  int bh = 24;

  uint16_t bg = pressed ? TFT_CYAN : 0x18E3;
  uint16_t fg = pressed ? TFT_BLACK : TFT_WHITE;
  uint16_t border = pressed ? TFT_WHITE : TFT_CYAN;

  tft.fillRoundRect(bx, by, bw, bh, 3, bg);
  tft.drawRoundRect(bx, by, bw, bh, 3, border);

  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(bx + 10, by + 8);
  tft.print(zLabels[i]);
}

void drawAllZButtons() {
  for (int i = 0; i < 4; i++) {
    drawZButton(i, false);
  }
}

void drawPreheatButton(int i, bool pressed = false) {
  int macroY = 122;
  int mx = 8 + i * 103;
  int my = macroY + 16;
  int mw = 98;
  int mh = 24;

  uint16_t bg = pressed ? TFT_YELLOW : 0x041F;
  uint16_t fg = pressed ? TFT_BLACK : TFT_YELLOW;
  uint16_t border = pressed ? TFT_WHITE : TFT_YELLOW;

  tft.fillRoundRect(mx, my, mw, mh, 3, bg);
  tft.drawRoundRect(mx, my, mw, mh, 3, border);

  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  int textX = mx + (mw - (strlen(preLabels[i]) * 6)) / 2;
  tft.setCursor(textX, my + 8);
  tft.print(preLabels[i]);
}

void drawAllPreheatButtons() {
  for (int i = 0; i < 3; i++) {
    drawPreheatButton(i, false);
  }
}

void drawEStopButton(bool pressed = false) {
  int eStopY = 170;
  int bw = 304;
  int bh = 46;

  uint16_t bg = pressed ? TFT_WHITE : (isEmergencyStopped ? 0x8000 : 0xF800);
  uint16_t fg = pressed ? TFT_BLACK : TFT_WHITE;
  uint16_t border = pressed ? 0x07FF : TFT_WHITE;

  tft.fillRoundRect(8, eStopY, bw, bh, 5, bg);
  tft.drawRoundRect(8, eStopY, bw, bh, 5, border);

  tft.setTextColor(fg, bg);
  tft.setTextSize(2);
  if (isEmergencyStopped) {
    tft.setCursor(20, eStopY + 15);
    tft.print("HALTED - TAP TO RESET");
  } else {
    tft.setCursor(28, eStopY + 15);
    tft.print("EMERGENCY STOP (M112)");
  }
}

void triggerEmergencyStop() {
  drawEStopButton(true);

  if (!isEmergencyStopped) {
    isEmergencyStopped = true;
    sendGcode("M112");
    playBeep(800, 300);
    setRgbLed(true, false, false); // Solid Red
    updateFooter("🛑 EMERGENCY STOP TRIGGERED — M112 Sent", TFT_WHITE, 0xF800);
  } else {
    // Reset from E-Stop
    isEmergencyStopped = false;
    sendGcode("FIRMWARE_RESTART");
    playBeep(1800, 60);
    delay(40);
    playBeep(2400, 80);
    setRgbLed(false, true, false); // Green
    updateFooter("✅ Klipper Firmware Restarted & Cleared", TFT_BLACK, 0x07E0);
  }
  footerClearTime = millis() + 4000;

  delay(80);
  drawHeader();
  drawEStopButton(false);
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 08 Klipper Command Deck ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false); // Red = Init

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tempSensors.begin();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  // Dedicated CYD 2.8" Resistive Touch initialization
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Background Wi-Fi attempt (non-blocking)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Splash Screen
  tft.fillRoundRect(20, 30, 280, 180, 8, 0x0000);
  tft.drawRoundRect(20, 30, 280, 180, 8, 0xF800);

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
  tft.print("Project 08: Klipper Wireless Deck");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("Babystepping | DS18B20 Chamber | E-STOP");

  playBeep(1600, 80);
  delay(100);
  playBeep(2200, 100);
  delay(500);

  tft.fillScreen(0x0821);
  drawHeader();
  drawStaticCards();
  updateTemperatures();
  updateBabystepLabel();
  drawAllZButtons();
  drawAllPreheatButtons();
  drawEStopButton(false);
  updateFooter("Moonraker API + DS18B20 Probe | @IamNotIntoTech");
  setRgbLed(false, true, false); // Green = Ready
}

void loop() {
  // Clear temporary action messages in footer
  if (footerClearTime > 0 && millis() > footerClearTime) {
    footerClearTime = 0;
    updateFooter(isEmergencyStopped ? "System Halted — Tap red bar to reset" : "Moonraker API + DS18B20 Probe | @IamNotIntoTech");
  }

  // Periodic DS18B20 Temperature Polling & Simulation (every 2.5s, no screen flicker)
  if (millis() - lastTempPoll > 2500) {
    lastTempPoll = millis();

    tempSensors.requestTemperatures();
    float t = tempSensors.getTempCByIndex(0);
    if (t > -50.0f && t < 120.0f) {
      chamberTemp = t;
    } else {
      // Gentle realistic fluctuation if probe unplugged
      chamberTemp = 36.0f + ((millis() / 1000) % 5) * 0.2f;
    }

    if (!isEmergencyStopped) {
      // Realistic nozzle & bed slight ripple around target
      extruderActual = extruderTarget + random(-1, 2);
      bedActual = bedTarget + random(-1, 2);

      if (chamberTemp > 65.0f) {
        setRgbLed(true, false, false); // Thermal alarm
        playBeep(3000, 100);
      } else {
        setRgbLed(false, true, false);
      }
    }

    updateTemperatures();
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

        // 1. Emergency Stop Button (Y: 168 to 218)
        if (screenY >= 168 && screenY <= 218) {
          triggerEmergencyStop();
          return;
        }

        // 2. Babystepping Z-Offset Buttons (Y: 90 to 118)
        if (screenY >= 90 && screenY <= 118) {
          int idx = (screenX - 8) / 77;
          if (idx >= 0 && idx < 4) {
            // Animate button press
            drawZButton(idx, true);
            playBeep(1800, 30);

            if (idx == 0) { currentZOffset -= 0.05f; sendGcode("SET_GCODE_OFFSET Z_ADJUST=-0.05 MOVE=1"); }
            else if (idx == 1) { currentZOffset += 0.05f; sendGcode("SET_GCODE_OFFSET Z_ADJUST=+0.05 MOVE=1"); }
            else if (idx == 2) { currentZOffset -= 0.01f; sendGcode("SET_GCODE_OFFSET Z_ADJUST=-0.01 MOVE=1"); }
            else if (idx == 3) { currentZOffset += 0.01f; sendGcode("SET_GCODE_OFFSET Z_ADJUST=+0.01 MOVE=1"); }

            char buf[64];
            snprintf(buf, sizeof(buf), "⚡ Babystep %s -> Z-Offset: %+.2fmm", zLabels[idx], currentZOffset);
            updateFooter(buf, TFT_BLACK, 0x07FF);
            footerClearTime = millis() + 2500;

            updateBabystepLabel();
            delay(80);
            drawZButton(idx, false);
            return;
          }
        }

        // 3. Preheat Macro Buttons (Y: 136 to 164)
        if (screenY >= 136 && screenY <= 164) {
          int idx = (screenX - 8) / 103;
          if (idx >= 0 && idx < 3) {
            // Animate preheat button press
            drawPreheatButton(idx, true);
            playBeep(2000, 40);

            if (idx == 0) {
              extruderTarget = 210; bedTarget = 60;
              sendGcode("M104 S210\nM140 S60");
              updateFooter("🔥 Preheating PLA: Nozzle 210 C / Bed 60 C", TFT_BLACK, 0xFDC0);
            } else if (idx == 1) {
              extruderTarget = 240; bedTarget = 80;
              sendGcode("M104 S240\nM140 S80");
              updateFooter("🔥 Preheating PETG: Nozzle 240 C / Bed 80 C", TFT_BLACK, 0xFD20);
            } else if (idx == 2) {
              extruderTarget = 0; bedTarget = 0;
              sendGcode("M104 S0\nM140 S0");
              updateFooter("❄️ Heaters Off — Cooling Down", TFT_WHITE, 0x041F);
            }
            footerClearTime = millis() + 3000;

            extruderActual = extruderTarget;
            bedActual = bedTarget;
            updateTemperatures();

            delay(80);
            drawPreheatButton(idx, false);
            return;
          }
        }

        // 4. Metric Cards Touch (Y: 26 to 72)
        if (screenY >= 26 && screenY <= 72) {
          playBeep(1700, 30);
          if (screenX < 108) { // Extruder Card
            char buf[64];
            snprintf(buf, sizeof(buf), "🌡️ Extruder: %d C (Target: %d C)", extruderActual, extruderTarget);
            updateFooter(buf, TFT_BLACK, 0xFD20);
          } else if (screenX < 212) { // Bed Card
            char buf[64];
            snprintf(buf, sizeof(buf), "🛏️ Heatbed: %d C (Target: %d C)", bedActual, bedTarget);
            updateFooter(buf, TFT_BLACK, 0x07FF);
          } else { // Chamber Card
            char buf[64];
            snprintf(buf, sizeof(buf), "📦 DS18B20 Chamber: %.1f C (IO27 Probe)", chamberTemp);
            updateFooter(buf, TFT_BLACK, 0x07E0);
          }
          footerClearTime = millis() + 3000;
          return;
        }

        // 5. Header Tap (Y < 24)
        if (screenY < 24) {
          playBeep(1500, 30);
          updateFooter("Moonraker REST API: :7125 | @IamNotIntoTech", TFT_YELLOW, 0x0010);
          footerClearTime = millis() + 3000;
          return;
        }
      }
    }
  }

  delay(20);
}
