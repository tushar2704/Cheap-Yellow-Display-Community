/**
 * @file 09-atc-flight-radar.ino
 * @brief ATC Rotating Sweep Flight Radar ("What Was That?" Edition)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch (SPI: CLK=25, MISO=39, MOSI=32, CS=33)
 * - Network: ESP32 Wi-Fi + ADS-B Telemetry Stream
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <HTTPClient.h>
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

// Wi-Fi Config
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS); // Polling mode avoids GPIO 36 interrupt lockup
TFT_eSPI tft = TFT_eSPI();

// Radar Scope Geometry
const int RADAR_CX = 90;
const int RADAR_CY = 124;
const int RADAR_R  = 78;

float sweepAngle = 0.0f;
float lastSweepAngle = 0.0f;
int selectedRangeNM = 25; // 10, 25, 50 NM

struct Aircraft {
  const char* callsign;
  const char* model;
  int altitudeFt;
  int speedKts;
  int headingDeg;
  float distanceNM;
  int squawk;
  int x;
  int y;
  bool isAlert;
};

#define MAX_AIRCRAFT 4
Aircraft traffic[MAX_AIRCRAFT] = {
  {"UAL284", "B738", 12500, 360, 45, 6.2f, 1200, 0, 0, false},
  {"DAL119", "A321", 8200,  280, 220, 14.8f, 1200, 0, 0, false},
  {"SWA421", "B737", 3400,  210, 110, 3.8f, 1200, 0, 0, true},
  {"MEDEVAC", "EC35", 1500, 130, 315, 8.4f, 7700, 0, 0, true}
};

int highlightedCraft = 2; // SWA421 nearest
unsigned long lastRadarTick = 0;
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

void drawStaticRadarScope() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("ATC AIRSPACE RADAR  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("IamNotIntoTech");

  // Range Badge (top right)
  tft.fillRect(220, 0, 100, 24, 0x0010);
  tft.setTextColor(TFT_WHITE, 0x0010);
  tft.setCursor(225, 4);
  tft.printf("RANGE: %d NM", selectedRangeNM);

  // Clear Radar Scope Area
  tft.fillRect(0, 24, 180, 200, 0x0000);

  // Concentric Rings
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R, 0x03E0);       // Outer limit
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 2 / 3, 0x0260); // Middle ring
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R / 3, 0x0200);     // Inner ring

  // Crosshairs
  tft.drawFastHLine(RADAR_CX - RADAR_R, RADAR_CY, RADAR_R * 2, 0x0200);
  tft.drawFastVLine(RADAR_CX, RADAR_CY - RADAR_R, RADAR_R * 2, 0x0200);

  // Cardinal markers
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(RADAR_CX - 2, RADAR_CY - RADAR_R + 3); tft.print("N");
  tft.setCursor(RADAR_CX + RADAR_R - 8, RADAR_CY - 3); tft.print("E");
  tft.setCursor(RADAR_CX - 2, RADAR_CY + RADAR_R - 9); tft.print("S");
  tft.setCursor(RADAR_CX - RADAR_R + 2, RADAR_CY - 3); tft.print("W");

  // Vertical Separator
  tft.drawFastVLine(180, 24, 200, 0x2965);

  updateFooter("Proximity ADS-B Sweeper | @IamNotIntoTech");
}

void drawRangeButtons(int pressedIdx = -1) {
  int cardX = 184;
  int btnY = 166;
  int btnW = 42;
  int btnH = 22;
  int gap = 3;
  int ranges[3] = {10, 25, 50};

  for (int i = 0; i < 3; i++) {
    int bx = cardX + i * (btnW + gap);
    bool isSel = (ranges[i] == selectedRangeNM);
    bool isPressed = (pressedIdx == i);

    uint16_t bg = isPressed ? TFT_WHITE : (isSel ? 0x07E0 : 0x18E3);
    uint16_t fg = isPressed ? TFT_BLACK : (isSel ? TFT_BLACK : TFT_WHITE);
    uint16_t border = isSel ? TFT_WHITE : 0x52AA;

    tft.fillRoundRect(bx, btnY, btnW, btnH, 3, bg);
    tft.drawRoundRect(bx, btnY, btnW, btnH, 3, border);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.setCursor(bx + 4, btnY + 7);
    tft.printf("%dNM", ranges[i]);
  }
}

void drawNextTargetButton(bool pressed = false) {
  int cardX = 184;
  int nxtY = 194;
  int cardW = 132;
  int nxtH = 24;

  uint16_t bg = pressed ? TFT_WHITE : 0xFD20;
  uint16_t fg = pressed ? TFT_BLACK : TFT_BLACK;
  uint16_t border = pressed ? 0xF800 : TFT_WHITE;

  tft.fillRoundRect(cardX, nxtY, cardW, nxtH, 4, bg);
  tft.drawRoundRect(cardX, nxtY, cardW, nxtH, 4, border);
  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(cardX + 18, nxtY + 8);
  tft.print(">> NEXT TARGET <<");
}

void drawFlightInfoCard() {
  int cardX = 184;
  int cardY = 28;
  int cardW = 132;

  uint16_t borderCol = (traffic[highlightedCraft].squawk == 7700) ? 0xF800 : 0x041F;
  tft.fillRoundRect(cardX, cardY, cardW, 134, 4, 0x0000);
  tft.drawRoundRect(cardX, cardY, cardW, 134, 4, borderCol);

  const Aircraft& a = traffic[highlightedCraft];

  tft.setTextSize(1);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(cardX + 8, cardY + 6);
  tft.print("TRACKED TARGET");

  // Callsign & Type
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(cardX + 8, cardY + 20);
  tft.print(a.callsign);

  tft.setTextSize(1);
  tft.setTextColor(0xFDC0, 0x0000);
  tft.setCursor(cardX + 8, cardY + 40);
  tft.printf("Type: %s", a.model);

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setCursor(cardX + 8, cardY + 56);
  tft.printf("Alt:   %d ft", a.altitudeFt);

  tft.setCursor(cardX + 8, cardY + 70);
  tft.printf("Speed: %d kts", a.speedKts);

  tft.setCursor(cardX + 8, cardY + 84);
  tft.printf("Dist:  %.1f NM", a.distanceNM);

  tft.setCursor(cardX + 8, cardY + 98);
  tft.printf("Track: %d deg", a.headingDeg);

  // Squawk Pill
  int squawkY = cardY + 112;
  bool isEmerg = (a.squawk == 7700);
  uint16_t sqBg = isEmerg ? 0xF800 : 0x0208;
  tft.fillRoundRect(cardX + 8, squawkY, 116, 16, 3, sqBg);
  tft.drawRoundRect(cardX + 8, squawkY, 116, 16, 3, isEmerg ? TFT_WHITE : 0x07E0);
  tft.setTextColor(TFT_WHITE, sqBg);
  tft.setCursor(cardX + 14, squawkY + 4);
  tft.printf(isEmerg ? "EMERG: SQK 7700" : "SQUAWK: %04d", a.squawk);

  drawRangeButtons();
  drawNextTargetButton();
}

void updateRadarSweep() {
  // Erase previous sweep line
  float rad = sweepAngle * 0.0174533f;
  float oldRad = lastSweepAngle * 0.0174533f;

  int oldX = RADAR_CX + (int)(cos(oldRad) * (RADAR_R - 2));
  int oldY = RADAR_CY + (int)(sin(oldRad) * (RADAR_R - 2));
  tft.drawLine(RADAR_CX, RADAR_CY, oldX, oldY, 0x0000);

  // Redraw faint rings if erased
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R, 0x03E0);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 2 / 3, 0x0260);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R / 3, 0x0200);

  // Draw new sweep line in phosphor green
  int newX = RADAR_CX + (int)(cos(rad) * (RADAR_R - 2));
  int newY = RADAR_CY + (int)(sin(rad) * (RADAR_R - 2));
  tft.drawLine(RADAR_CX, RADAR_CY, newX, newY, 0x07E0);

  // Plot Aircraft Blips
  for (int i = 0; i < MAX_AIRCRAFT; i++) {
    Aircraft& a = traffic[i];
    float distScale = a.distanceNM / (float)selectedRangeNM;
    distScale = constrain(distScale, 0.1f, 1.0f);

    float craftRad = (a.headingDeg - 90) * 0.0174533f;
    int px = RADAR_CX + (int)(cos(craftRad) * (RADAR_R * distScale));
    int py = RADAR_CY + (int)(sin(craftRad) * (RADAR_R * distScale));
    a.x = px;
    a.y = py;

    uint16_t blipCol = (a.squawk == 7700) ? 0xF800 : (i == highlightedCraft ? TFT_WHITE : 0x07E0);
    tft.fillCircle(px, py, (i == highlightedCraft ? 3 : 2), blipCol);

    if (i == highlightedCraft) {
      tft.drawCircle(px, py, 6, 0x07FF);
    }
  }

  lastSweepAngle = sweepAngle;
  sweepAngle += 4.0f;
  if (sweepAngle >= 360.0f) {
    sweepAngle = 0.0f;
    if (traffic[highlightedCraft].distanceNM < 5.0f) {
      playBeep(2400, 30);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 09 ATC Flight Radar ---");
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

  // Initialize touch on dedicated CYD SPI pins
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

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
  tft.print("Project 09: ATC Flight Sweep Radar");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("CRT Sweep | Proximity Alarms | Squawk 7700");

  playBeep(1800, 80);
  delay(100);
  playBeep(2400, 100);
  delay(500);

  drawStaticRadarScope();
  drawFlightInfoCard();
  setRgbLed(false, true, false);
}

void loop() {
  if (millis() - lastRadarTick > 35) {
    lastRadarTick = millis();
    updateRadarSweep();
  }

  // Clear footer notification
  if (footerClearTime > 0 && millis() > footerClearTime) {
    footerClearTime = 0;
    updateFooter("Proximity ADS-B Sweeper | @IamNotIntoTech");
  }

  // Touch handling (calibrated, non-inverted Y)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();

        // Landscape calibration: non-inverted Y
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, screenX, screenY);

        // 1. Range Buttons (X: 184 to 316, Y: 166 to 190)
        if (screenX >= 184 && screenX <= 316 && screenY >= 164 && screenY <= 192) {
          int idx = (screenX - 184) / 45;
          if (idx >= 0 && idx < 3) {
            drawRangeButtons(idx);
            playBeep(2000, 35);

            if (idx == 0) selectedRangeNM = 10;
            else if (idx == 1) selectedRangeNM = 25;
            else if (idx == 2) selectedRangeNM = 50;

            char msg[64];
            snprintf(msg, sizeof(msg), "⚡ Radar Range Set: %d NM", selectedRangeNM);
            updateFooter(msg, TFT_BLACK, 0x07FF);
            footerClearTime = millis() + 2000;

            delay(80);
            drawStaticRadarScope();
            drawFlightInfoCard();
          }
        }
        // 2. Next Target Button (X: 184 to 316, Y: 194 to 222)
        else if (screenX >= 184 && screenX <= 316 && screenY >= 193 && screenY <= 223) {
          drawNextTargetButton(true);
          playBeep(2200, 40);

          highlightedCraft = (highlightedCraft + 1) % MAX_AIRCRAFT;
          char msg[64];
          snprintf(msg, sizeof(msg), "⚡ Target Selected: %s (%s)", traffic[highlightedCraft].callsign, traffic[highlightedCraft].model);
          updateFooter(msg, TFT_BLACK, 0xFDC0);
          footerClearTime = millis() + 2500;

          delay(80);
          drawFlightInfoCard();
        }
        // 3. Tap on Radar Scope to target nearest blip
        else if (screenX < 180 && screenY >= 24 && screenY <= 224) {
          float minD = 99999.0f;
          int nearestIdx = highlightedCraft;
          for (int i = 0; i < MAX_AIRCRAFT; i++) {
            float dx = screenX - traffic[i].x;
            float dy = screenY - traffic[i].y;
            float d = sqrt(dx * dx + dy * dy);
            if (d < minD) {
              minD = d;
              nearestIdx = i;
            }
          }
          if (minD < 30.0f) {
            highlightedCraft = nearestIdx;
            playBeep(2400, 40);
            char msg[64];
            snprintf(msg, sizeof(msg), "⚡ Radar Lock: %s", traffic[highlightedCraft].callsign);
            updateFooter(msg, TFT_BLACK, 0x07E0);
            footerClearTime = millis() + 2500;
            drawFlightInfoCard();
          }
        }
      }
    }
  }

  delay(10);
}
