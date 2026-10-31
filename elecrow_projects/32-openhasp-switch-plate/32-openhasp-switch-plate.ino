/**
 * 💡 Project 32: Smart Home Touch Switch Plate
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Smart Home control console (Home Assistant / MQTT ready).
 * - Page 1: Multi-zone lighting toggles (Main Ceiling, Desk Lamp, Ambient Strip) & 0-100% dimming touch slider.
 * - Page 2: HVAC climate thermostat with current/target temperature adjusting and heat/cool/auto/off mode selection.
 * - Page 3: 4-button quick scene launcher (Focus Work, Movie Night, Relax Chill, All Off).
 * - Hardware active-low RGB LED synchronizing ambient scene mood colors.
 * - Calibrated non-inverted resistive touch controls with acoustic buzzer clicks.
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Theme Colors (16-bit 565)
#define COLOR_BG         0x0842 // Charcoal
#define COLOR_NAV        0x10A3
#define COLOR_CARD       0x18E5
#define COLOR_CARD_BRD   0x2967
#define COLOR_ACTIVE     0x04DF // Vibrant Blue
#define COLOR_ACCENT     0xFD20 // Warm Amber
#define COLOR_GREEN      0x27C4
#define COLOR_TEXT       0xFFFF
#define COLOR_MUTED      0x9CF3

// System States
int currentPage = 0; // 0 = Lights, 1 = Climate, 2 = Scenes

// Light States
bool light1 = true;  // Main Ceiling
bool light2 = false; // Desk Lamp
bool light3 = true;  // Ambient Strip
int brightness = 75; // 0 to 100%

// Climate States
float currentTemp = 21.2;
float targetTemp = 22.0;
enum HvacMode { HVAC_HEAT, HVAC_COOL, HVAC_AUTO, HVAC_OFF };
HvacMode hvacMode = HVAC_HEAT;

// Scenes
int activeScene = 0; // 0: None, 1: Focus, 2: Movie, 3: Relax, 4: All Off

unsigned long lastTouchTime = 0;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void updateSceneRGB() {
  switch (activeScene) {
    case 1: setRGB(false, false, true); break; // Focus: Blue
    case 2: setRGB(true, false, true); break;  // Movie: Magenta
    case 3: setRGB(false, true, false); break; // Relax: Green
    default: setRGB(false, false, false); break;
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, COLOR_NAV);
  tft.drawFastHLine(0, 26, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_ACCENT, COLOR_NAV);
  tft.drawString("IamNotIntoTech", 8, 6, 2);

  tft.setTextColor(COLOR_TEXT, COLOR_NAV);
  tft.drawCentreString("SMART SWITCH PLATE", 180, 6, 2);

  // Status Indicator
  tft.fillCircle(305, 13, 4, COLOR_GREEN);
}

void drawBottomNav() {
  tft.fillRect(0, 206, 320, 34, COLOR_NAV);
  tft.drawFastHLine(0, 206, 320, COLOR_CARD_BRD);

  const char* tabs[3] = { "LIGHTS", "CLIMATE", "SCENES" };
  for (int i = 0; i < 3; i++) {
    int x = i * 106 + 3;
    bool active = (currentPage == i);
    uint16_t bg = active ? COLOR_CARD : COLOR_NAV;
    uint16_t fg = active ? COLOR_ACCENT : COLOR_MUTED;

    tft.fillRoundRect(x, 210, 100, 26, 4, bg);
    if (active) {
      tft.drawRoundRect(x, 210, 100, 26, 4, COLOR_ACCENT);
      tft.fillRect(x + 20, 233, 60, 2, COLOR_ACCENT);
    }
    tft.setTextColor(fg, bg);
    tft.drawCentreString(tabs[i], x + 50, 215, 2);
  }
}

void drawLightsPage() {
  tft.fillRect(0, 27, 320, 178, COLOR_BG);

  const char* names[3] = { "Main Ceiling", "Desk Lamp", "Ambient Strip" };
  bool states[3] = { light1, light2, light3 };

  for (int i = 0; i < 3; i++) {
    int x = 10 + i * 102;
    int y = 34;
    int w = 96;
    int h = 82;

    uint16_t cardBg = states[i] ? tft.color565(30, 45, 65) : COLOR_CARD;
    uint16_t cardBrd = states[i] ? COLOR_ACTIVE : COLOR_CARD_BRD;

    tft.fillRoundRect(x, y, w, h, 6, cardBg);
    tft.drawRoundRect(x, y, w, h, 6, cardBrd);

    // State Badge
    tft.fillCircle(x + w - 16, y + 16, 6, states[i] ? COLOR_ACCENT : tft.color565(60, 60, 60));

    tft.setTextColor(COLOR_TEXT, cardBg);
    tft.drawString(names[i], x + 8, y + 36, 1);

    tft.setTextColor(states[i] ? COLOR_ACCENT : COLOR_MUTED, cardBg);
    tft.drawString(states[i] ? "STATE: ON" : "STATE: OFF", x + 8, y + 54, 2);
  }

  // Brightness Slider Card
  tft.fillRoundRect(10, 124, 300, 72, 6, COLOR_CARD);
  tft.drawRoundRect(10, 124, 300, 72, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawString("MASTER BRIGHTNESS", 24, 134, 1);

  char bBuf[16];
  snprintf(bBuf, sizeof(bBuf), "%d%%", brightness);
  tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
  tft.drawRightString(bBuf, 296, 134, 1);

  // Slider Track
  int trackX = 24;
  int trackY = 162;
  int trackW = 272;
  int trackH = 12;

  tft.fillRoundRect(trackX, trackY, trackW, trackH, 6, tft.color565(30, 35, 45));
  int fillW = map(brightness, 0, 100, 0, trackW);
  tft.fillRoundRect(trackX, trackY, fillW, trackH, 6, COLOR_ACTIVE);

  // Slider Handle
  tft.fillCircle(trackX + fillW, trackY + trackH / 2, 8, COLOR_TEXT);
  tft.drawCircle(trackX + fillW, trackY + trackH / 2, 8, COLOR_ACTIVE);
}

void drawClimatePage() {
  tft.fillRect(0, 27, 320, 178, COLOR_BG);

  // Current Temp Card
  tft.fillRoundRect(10, 34, 145, 96, 6, COLOR_CARD);
  tft.drawRoundRect(10, 34, 145, 96, 6, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("CURRENT ROOM", 20, 42, 1);

  char cBuf[16];
  snprintf(cBuf, sizeof(cBuf), "%.1f", currentTemp);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawString(cBuf, 20, 56, 6);
  tft.drawString("C", 116, 62, 2);

  // Target Temp Card with Buttons
  tft.fillRoundRect(165, 34, 145, 96, 6, COLOR_CARD);
  tft.drawRoundRect(165, 34, 145, 96, 6, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("TARGET TEMP", 175, 42, 1);

  char tBuf[16];
  snprintf(tBuf, sizeof(tBuf), "%.1f C", targetTemp);
  tft.setTextColor(COLOR_ACCENT, COLOR_CARD);
  tft.drawCentreString(tBuf, 237, 56, 4);

  // Minus Button
  tft.fillRoundRect(175, 92, 55, 30, 4, tft.color565(40, 45, 55));
  tft.drawRoundRect(175, 92, 55, 30, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, tft.color565(40, 45, 55));
  tft.drawCentreString("-", 202, 94, 4);

  // Plus Button
  tft.fillRoundRect(245, 92, 55, 30, 4, tft.color565(40, 45, 55));
  tft.drawRoundRect(245, 92, 55, 30, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, tft.color565(40, 45, 55));
  tft.drawCentreString("+", 272, 94, 4);

  // Mode Selectors
  const char* modes[4] = { "HEAT", "COOL", "AUTO", "OFF" };
  for (int i = 0; i < 4; i++) {
    int x = 10 + i * 77;
    int y = 142;
    int w = 70;
    int h = 48;

    bool sel = (hvacMode == (HvacMode)i);
    uint16_t mBg = sel ? COLOR_ACTIVE : COLOR_CARD;
    uint16_t mBrd = sel ? COLOR_TEXT : COLOR_CARD_BRD;

    tft.fillRoundRect(x, y, w, h, 6, mBg);
    tft.drawRoundRect(x, y, w, h, 6, mBrd);
    tft.setTextColor(sel ? COLOR_TEXT : COLOR_MUTED, mBg);
    tft.drawCentreString(modes[i], x + 35, y + 16, 2);
  }
}

void drawScenesPage() {
  tft.fillRect(0, 27, 320, 178, COLOR_BG);

  const char* sceneNames[4] = { "Focus Work", "Movie Night", "Relax / Chill", "All Off" };
  const char* sceneSubs[4] = { "Cool White 100%", "Dim Warm 15%", "Ambient Green 50%", "Kill All Lights" };
  uint16_t sceneColors[4] = { 0x04DF, 0xF81F, 0x07E0, 0xF800 };

  for (int i = 0; i < 4; i++) {
    int col = i % 2;
    int row = i / 2;
    int x = 10 + col * 154;
    int y = 36 + row * 78;
    int w = 146;
    int h = 70;

    bool active = (activeScene == i + 1);
    uint16_t sBg = active ? tft.color565(30, 40, 60) : COLOR_CARD;
    uint16_t sBrd = active ? sceneColors[i] : COLOR_CARD_BRD;

    tft.fillRoundRect(x, y, w, h, 6, sBg);
    tft.drawRoundRect(x, y, w, h, 6, sBrd);

    // Accent strip
    tft.fillRect(x + 4, y + 4, 4, h - 8, sceneColors[i]);

    tft.setTextColor(COLOR_TEXT, sBg);
    tft.drawString(sceneNames[i], x + 16, y + 14, 2);

    tft.setTextColor(COLOR_MUTED, sBg);
    tft.drawString(sceneSubs[i], x + 16, y + 36, 1);
  }
}

void renderCurrentPage() {
  if (currentPage == 0) drawLightsPage();
  else if (currentPage == 1) drawClimatePage();
  else if (currentPage == 2) drawScenesPage();
  drawBottomNav();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  // Initialize Touch (VSPI)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawHeader();
  renderCurrentPage();

  playBeep(2200, 60);
  Serial.println(F("[SYSTEM] Smart Home Switch Plate initialized."));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Navigation Tab Bar (screenY >= 205)
      if (screenY >= 205) {
        if (millis() - lastTouchTime > 180) {
          lastTouchTime = millis();
          int tapped = screenX / 106;
          if (tapped >= 0 && tapped <= 2 && tapped != currentPage) {
            currentPage = tapped;
            playBeep(1800, 30);
            renderCurrentPage();
          }
        }
      }
      // Page 0: Lights
      else if (currentPage == 0) {
        if (screenY >= 34 && screenY <= 116) {
          if (millis() - lastTouchTime > 180) {
            lastTouchTime = millis();
            int lIdx = (screenX - 10) / 102;
            if (lIdx == 0) { light1 = !light1; playBeep(2000, 30); drawLightsPage(); }
            else if (lIdx == 1) { light2 = !light2; playBeep(2000, 30); drawLightsPage(); }
            else if (lIdx == 2) { light3 = !light3; playBeep(2000, 30); drawLightsPage(); }
          }
        }
        // Slider Dragging
        else if (screenY >= 145 && screenY <= 185 && screenX >= 24 && screenX <= 296) {
          brightness = map(screenX, 24, 296, 0, 100);
          brightness = constrain(brightness, 0, 100);
          drawLightsPage();
          delay(20);
        }
      }
      // Page 1: Climate
      else if (currentPage == 1) {
        if (screenY >= 92 && screenY <= 124) {
          if (millis() - lastTouchTime > 140) {
            lastTouchTime = millis();
            if (screenX >= 175 && screenX <= 230) {
              targetTemp = max(16.0f, targetTemp - 0.5f);
              playBeep(1600, 25);
              drawClimatePage();
            } else if (screenX >= 245 && screenX <= 300) {
              targetTemp = min(30.0f, targetTemp + 0.5f);
              playBeep(2200, 25);
              drawClimatePage();
            }
          }
        }
        else if (screenY >= 142 && screenY <= 190) {
          if (millis() - lastTouchTime > 180) {
            lastTouchTime = millis();
            int mIdx = (screenX - 10) / 77;
            if (mIdx >= 0 && mIdx <= 3) {
              hvacMode = (HvacMode)mIdx;
              playBeep(2000, 30);
              drawClimatePage();
            }
          }
        }
      }
      // Page 2: Scenes
      else if (currentPage == 2) {
        if (millis() - lastTouchTime > 180) {
          lastTouchTime = millis();
          int col = (screenX >= 164) ? 1 : 0;
          int row = (screenY >= 114) ? 1 : 0;
          int sIdx = row * 2 + col + 1;
          activeScene = sIdx;
          playBeep(2400, 50);
          updateSceneRGB();

          if (activeScene == 4) { // All off
            light1 = light2 = light3 = false;
            brightness = 0;
          } else if (activeScene == 1) { // Focus
            light1 = light2 = true;
            brightness = 100;
          }

          drawScenesPage();
        }
      }
    }
  }

  delay(20);
}

