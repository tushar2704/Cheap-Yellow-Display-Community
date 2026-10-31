# 1 "C:\\Users\\tusha\\AppData\\Local\\Temp\\tmpn_wt4q9g"
#include <Arduino.h>
# 1 "D:/Cheap-Yellow-Display-Community/elecrow_projects/09-atc-flight-radar/09-atc-flight-radar.ino"
# 17 "D:/Cheap-Yellow-Display-Community/elecrow_projects/09-atc-flight-radar/09-atc-flight-radar.ino"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>


#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33


#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define LDR_PIN 34
#define SPEAKER_PIN 26


#define TS_MINX 240
#define TS_MAXX 3800
#define TS_MINY 240
#define TS_MAXY 3800


const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS);
TFT_eSPI tft = TFT_eSPI();


const int RADAR_CX = 90;
const int RADAR_CY = 124;
const int RADAR_R = 78;

float sweepAngle = 0.0f;
float lastSweepAngle = 0.0f;
int selectedRangeNM = 25;

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
  {"DAL119", "A321", 8200, 280, 220, 14.8f, 1200, 0, 0, false},
  {"SWA421", "B737", 3400, 210, 110, 3.8f, 1200, 0, 0, true},
  {"MEDEVAC", "EC35", 1500, 130, 315, 8.4f, 7700, 0, 0, true}
};

int highlightedCraft = 2;
unsigned long lastRadarTick = 0;
unsigned long lastTouchTime = 0;
void setRgbLed(bool r, bool g, bool b);
void playBeep(int freq, int durationMs);
void drawStaticRadarScope();
void drawFlightInfoCard();
void updateRadarSweep();
void setup();
void loop();
#line 85 "D:/Cheap-Yellow-Display-Community/elecrow_projects/09-atc-flight-radar/09-atc-flight-radar.ino"
void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void drawStaticRadarScope() {

  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("ATC AIRSPACE RADAR  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("IamNotIntoTech");


  tft.setTextColor(TFT_WHITE, 0x0010);
  tft.setCursor(225, 4);
  tft.printf("RANGE: %d NM", selectedRangeNM);


  tft.fillRect(0, 24, 180, 200, 0x0000);


  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R, 0x03E0);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 2 / 3, 0x0260);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R / 3, 0x0200);


  tft.drawFastHLine(RADAR_CX - RADAR_R, RADAR_CY, RADAR_R * 2, 0x0200);
  tft.drawFastVLine(RADAR_CX, RADAR_CY - RADAR_R, RADAR_R * 2, 0x0200);


  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(RADAR_CX - 2, RADAR_CY - RADAR_R + 3); tft.print("N");
  tft.setCursor(RADAR_CX + RADAR_R - 8, RADAR_CY - 3); tft.print("E");
  tft.setCursor(RADAR_CX - 2, RADAR_CY + RADAR_R - 9); tft.print("S");
  tft.setCursor(RADAR_CX - RADAR_R + 2, RADAR_CY - 3); tft.print("W");


  tft.drawFastVLine(180, 24, 200, 0x2965);


  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print("Proximity ADS-B Sweeper | @IamNotIntoTech");
}

void drawFlightInfoCard() {
  int cardX = 184;
  int cardY = 28;
  int cardW = 132;

  const Aircraft& a = traffic[highlightedCraft];


  if (a.squawk == 7700) {
    setRgbLed(true, false, false);
    tft.drawRoundRect(cardX, cardY, cardW, 134, 4, 0xF800);
  } else {
    setRgbLed(false, true, false);
    tft.drawRoundRect(cardX, cardY, cardW, 134, 4, 0x041F);
  }


  tft.fillRoundRect(cardX + 1, cardY + 1, cardW - 2, 132, 3, 0x0000);

  tft.setTextSize(1);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(cardX + 8, cardY + 6);
  tft.print("TRACKED TARGET");


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
  tft.printf("Hdg:   %03d deg", a.headingDeg);

  tft.setCursor(cardX + 8, cardY + 114);
  if (a.squawk == 7700) {
    tft.setTextColor(0xF800, 0x0000);
    tft.print("SQUAWK: 7700 EMERG!");
  } else {
    tft.setTextColor(0x07E0, 0x0000);
    tft.printf("Squawk: %d OK", a.squawk);
  }


  int btnY = 166;
  int btnW = 41;
  const int ranges[3] = {10, 25, 50};

  for (int i = 0; i < 3; i++) {
    int bx = cardX + i * (btnW + 4);
    bool isSel = (ranges[i] == selectedRangeNM);
    tft.fillRoundRect(bx, btnY, btnW, 24, 3, isSel ? 0x07E0 : 0x18E3);
    tft.drawRoundRect(bx, btnY, btnW, 24, 3, TFT_WHITE);

    tft.setTextColor(isSel ? TFT_BLACK : TFT_WHITE, isSel ? 0x07E0 : 0x18E3);
    tft.setTextSize(1);
    tft.setCursor(bx + 4, btnY + 8);
    tft.printf("%dNM", ranges[i]);
  }


  int nxtY = 194;
  tft.fillRoundRect(cardX, nxtY, cardW, 24, 3, 0xFD20);
  tft.drawRoundRect(cardX, nxtY, cardW, 24, 3, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, 0xFD20);
  tft.setTextSize(1);
  tft.setCursor(cardX + 18, nxtY + 8);
  tft.print(">> NEXT TARGET <<");
}

void updateRadarSweep() {

  float rad = sweepAngle * 0.0174533f;
  float oldRad = lastSweepAngle * 0.0174533f;

  int oldX = RADAR_CX + (int)(cos(oldRad) * (RADAR_R - 2));
  int oldY = RADAR_CY + (int)(sin(oldRad) * (RADAR_R - 2));
  tft.drawLine(RADAR_CX, RADAR_CY, oldX, oldY, 0x0000);


  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R, 0x03E0);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 2 / 3, 0x0260);
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R / 3, 0x0200);
  tft.drawFastHLine(RADAR_CX - RADAR_R, RADAR_CY, RADAR_R * 2, 0x0200);
  tft.drawFastVLine(RADAR_CX, RADAR_CY - RADAR_R, RADAR_R * 2, 0x0200);


  int newX = RADAR_CX + (int)(cos(rad) * (RADAR_R - 2));
  int newY = RADAR_CY + (int)(sin(rad) * (RADAR_R - 2));
  tft.drawLine(RADAR_CX, RADAR_CY, newX, newY, 0x07E0);


  for (int i = 0; i < MAX_AIRCRAFT; i++) {
    Aircraft& a = traffic[i];
    float distScale = a.distanceNM / (float)selectedRangeNM;
    distScale = constrain(distScale, 0.08f, 1.0f);

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

    for (int i = 0; i < MAX_AIRCRAFT; i++) {
      traffic[i].distanceNM += (i % 2 == 0 ? -0.1f : 0.1f);
      if (traffic[i].distanceNM < 1.0f) traffic[i].distanceNM = 12.0f;
      if (traffic[i].distanceNM > (float)selectedRangeNM) traffic[i].distanceNM = 2.5f;
    }

    if (traffic[highlightedCraft].distanceNM < 5.0f) {
      playBeep(2400, 30);
    }

    drawFlightInfoCard();
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


  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


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
  tft.setTextSize(1);
  tft.setCursor(44, 140);
  tft.print("CRT Sweep | Proximity Alarms | Squawk 7700");

  playBeep(1800, 80);
  delay(100);
  playBeep(2400, 100);
  delay(600);

  drawStaticRadarScope();
  drawFlightInfoCard();
  setRgbLed(false, true, false);
}

void loop() {

  if (millis() - lastRadarTick > 35) {
    lastRadarTick = millis();
    updateRadarSweep();
  }


  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 220) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      Serial.printf("[TOUCH] Raw(%d, %d) -> Screen(%d, %d)\n", p.x, p.y, screenX, screenY);


      if (screenX >= 0 && screenX <= 180 && screenY >= 24 && screenY <= 224) {
        int closestIdx = -1;
        float minDistSq = 999999.0f;
        for (int i = 0; i < MAX_AIRCRAFT; i++) {
          float dx = screenX - traffic[i].x;
          float dy = screenY - traffic[i].y;
          float d2 = dx * dx + dy * dy;
          if (d2 < minDistSq) {
            minDistSq = d2;
            closestIdx = i;
          }
        }
        if (closestIdx >= 0 && minDistSq < 1600) {
          highlightedCraft = closestIdx;
          playBeep(2400, 40);
          drawFlightInfoCard();
        }
      }

      else if (screenX >= 184 && screenX <= 316 && screenY >= 166 && screenY <= 190) {
        int idx = (screenX - 184) / 45;
        if (idx == 0) selectedRangeNM = 10;
        else if (idx == 1) selectedRangeNM = 25;
        else if (idx >= 2) selectedRangeNM = 50;

        playBeep(2000, 35);
        drawStaticRadarScope();
        drawFlightInfoCard();
      }

      else if (screenX >= 184 && screenX <= 316 && screenY >= 194 && screenY <= 222) {
        highlightedCraft = (highlightedCraft + 1) % MAX_AIRCRAFT;
        playBeep(2200, 35);
        drawFlightInfoCard();
      }
    }
  }

  delay(5);
}