/**
 * @file 17-offline-osm-navigator.ino
 * @brief Offline OSM Trail Navigator & Topo Deck - Paris Sector (Eiffel Tower)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI)
 * - Touch: XPT2046 Resistive Touch (SPI: CLK=25, MISO=39, MOSI=32, CS=33)
 * - MicroSD Port: CS=5, SCK=18, MISO=19, MOSI=23
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define STATION_LAT   48.8584
#define STATION_LON   2.2945
#define STATION_ALT   36
#endif

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33

#define SD_CS_PIN     5
#define SD_MOSI_PIN   23
#define SD_MISO_PIN   19
#define SD_SCK_PIN    18

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS); // Polling mode avoids GPIO 36 interrupt lockup
SPIClass sdSPI = SPIClass(HSPI);

bool sdMounted = false;
int zoomLevel = 15;
int mapOffsetX = 0;
int mapOffsetY = 0;
unsigned long lastTouchTime = 0;

float currentLat = STATION_LAT;
float currentLon = STATION_LON;
int currentAltM = STATION_ALT;
float tripDistKm = 3.4;
int headingDeg = 42;

struct Waypoint {
  int x;
  int y;
  const char* label;
};

const int MAX_WAYPOINTS = 8;
Waypoint waypoints[MAX_WAYPOINTS];
int waypointCount = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void initSD() {
  sdSPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  if (SD.begin(SD_CS_PIN, sdSPI, 20000000)) {
    sdMounted = true;
    Serial.println("[STORAGE] MicroSD Card Mounted Successfully!");
    setLED(false, true, false); // Green: SD Mounted
  } else {
    sdMounted = false;
    Serial.println("[STORAGE] MicroSD Card Not Present, utilizing internal vector engine.");
    setLED(true, true, false);  // Yellow: Embedded vector storage
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, tft.color565(15, 25, 30));
  tft.setTextColor(TFT_WHITE, tft.color565(15, 25, 30));
  tft.setTextSize(1);
  tft.drawString("PARIS TOPO NAVIGATOR", 8, 6, 2);

  // Status badges
  char statBuf[32];
  snprintf(statBuf, sizeof(statBuf), "Z:%d  %s  %d° NE", zoomLevel, sdMounted ? "SD:OK" : "VECT", headingDeg);
  tft.setTextColor(sdMounted ? TFT_GREEN : TFT_YELLOW, tft.color565(15, 25, 30));
  tft.drawRightString(statBuf, 312, 6, 2);
}

void drawVectorMap() {
  // Map drawing viewport: X: 0 to 260, Y: 27 to 205
  int vX = 0;
  int vY = 27;
  int vW = 260;
  int vH = 178;

  tft.fillRect(vX, vY, vW, vH, tft.color565(25, 35, 25)); // Earth / park green (Champ de Mars)

  // Topographic elevation contour lines around Paris basin
  for (int r = 25; r < 220; r += 28) {
    int cx = 130 + mapOffsetX;
    int cy = 115 + mapOffsetY;
    tft.drawCircle(cx, cy, r, tft.color565(40, 55, 35));
  }

  // River Seine Waterway Vector Boundary
  int riverStartX = 35 + mapOffsetX;
  for (int y = vY; y < vY + vH; y += 4) {
    int x1 = riverStartX + (int)(22 * sin((y + mapOffsetY) * 0.04));
    tft.fillRect(x1, y, 12, 4, tft.color565(35, 85, 175)); // Seine Blue
  }

  // Walking Trail & Promenade (Pont d'Iena / Champ de Mars)
  int trailPoints[6][2] = {
    { 25, 175 }, { 65, 145 }, { 110, 125 }, { 145, 95 }, { 185, 80 }, { 235, 45 }
  };

  for (int i = 0; i < 5; i++) {
    int px1 = trailPoints[i][0] + mapOffsetX;
    int py1 = trailPoints[i][1] + mapOffsetY;
    int px2 = trailPoints[i+1][0] + mapOffsetX;
    int py2 = trailPoints[i+1][1] + mapOffsetY;
    tft.drawLine(px1, py1, px2, py2, TFT_ORANGE);
    tft.drawLine(px1, py1 + 1, px2, py2 + 1, TFT_ORANGE);
  }

  // User Current Position: Eiffel Tower (Center Beacon)
  int userX = 130 + mapOffsetX;
  int userY = 115 + mapOffsetY;
  tft.fillCircle(userX, userY, 6, TFT_CYAN);
  tft.drawCircle(userX, userY, 10, TFT_WHITE);
  tft.drawCircle(userX, userY, 14, TFT_CYAN);

  // Compass heading direction arrow
  int hdx = (int)(16 * sin(headingDeg * M_PI / 180.0));
  int hdy = (int)(-16 * cos(headingDeg * M_PI / 180.0));
  tft.drawLine(userX, userY, userX + hdx, userY + hdy, TFT_RED);
  tft.drawLine(userX + 1, userY, userX + hdx + 1, userY + hdy, TFT_RED);

  // Dropped Waypoints
  for (int i = 0; i < waypointCount; i++) {
    int wx = waypoints[i].x + mapOffsetX;
    int wy = waypoints[i].y + mapOffsetY;
    if (wx >= vX && wx < (vX + vW) && wy >= vY && wy < (vY + vH)) {
      tft.fillTriangle(wx, wy - 8, wx - 5, wy + 2, wx + 5, wy + 2, TFT_YELLOW);
      tft.setTextColor(TFT_WHITE, tft.color565(25, 35, 25));
      tft.drawString(waypoints[i].label, wx + 6, wy - 4, 1);
    }
  }
}

void drawGloveControls() {
  // Right sidebar Glove Controls (X: 264 to 316)
  int sx = 264;
  int bw = 52;
  int bh = 32;

  // Zoom In (+)
  tft.fillRoundRect(sx, 30, bw, bh, 4, tft.color565(35, 45, 60));
  tft.drawRoundRect(sx, 30, bw, bh, 4, TFT_CYAN);
  tft.setTextColor(TFT_WHITE, tft.color565(35, 45, 60));
  tft.drawCentreString("+", sx + bw/2, 38, 4);

  // Pan Up (▲)
  tft.fillRoundRect(sx, 66, bw, bh, 4, tft.color565(35, 40, 50));
  tft.drawRoundRect(sx, 66, bw, bh, 4, TFT_DARKGREY);
  tft.drawCentreString("▲", sx + bw/2, 74, 2);

  // Pan Down (▼)
  tft.fillRoundRect(sx, 102, bw, bh, 4, tft.color565(35, 40, 50));
  tft.drawRoundRect(sx, 102, bw, bh, 4, TFT_DARKGREY);
  tft.drawCentreString("▼", sx + bw/2, 110, 2);

  // Zoom Out (-)
  tft.fillRoundRect(sx, 138, bw, bh, 4, tft.color565(35, 45, 60));
  tft.drawRoundRect(sx, 138, bw, bh, 4, TFT_CYAN);
  tft.drawCentreString("-", sx + bw/2, 146, 4);

  // Drop Waypoint (WP)
  tft.fillRoundRect(sx, 174, bw, bh - 2, 4, tft.color565(60, 40, 20));
  tft.drawRoundRect(sx, 174, bw, bh - 2, 4, TFT_ORANGE);
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, tft.color565(60, 40, 20));
  tft.drawCentreString("WP", sx + bw/2, 182, 2);

  // Bottom Telemetry Bar (Y: 206 to 240)
  tft.fillRect(0, 206, 320, 34, tft.color565(20, 20, 25));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 20, 25));
  tft.setTextSize(1);
  char telem1[36];
  snprintf(telem1, sizeof(telem1), "LAT: %.4f° N  LON: %.4f° E", currentLat, currentLon);
  tft.drawString(telem1, 8, 210, 1);

  char telem2[36];
  snprintf(telem2, sizeof(telem2), "ALT: %dm MSL  DIST: %.1f km", currentAltM, tripDistKm);
  tft.drawString(telem2, 8, 222, 1);

  tft.setTextColor(TFT_GREEN, tft.color565(20, 20, 25));
  tft.drawRightString("PARIS (JN18eu)", 312, 214, 2);
}

void drawUI() {
  drawHeader();
  drawVectorMap();
  drawGloveControls();
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  if (p.z > 50 || ts.touched()) {
    if (millis() - lastTouchTime > 180) {
      lastTouchTime = millis();
      // Calibrated non-inverted coordinate mapping for 2.8" CYD
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, screenX, screenY);

      // Check Right Sidebar Glove Buttons
      if (screenX >= 260) {
        if (screenY >= 30 && screenY < 65) {
          // Zoom In
          if (zoomLevel < 18) zoomLevel++;
          beep(2400, 40);
          drawUI();
        } else if (screenY >= 66 && screenY < 100) {
          // Pan Up
          mapOffsetY += 16;
          beep(2000, 30);
          drawUI();
        } else if (screenY >= 101 && screenY < 135) {
          // Pan Down
          mapOffsetY -= 16;
          beep(2000, 30);
          drawUI();
        } else if (screenY >= 136 && screenY < 170) {
          // Zoom Out
          if (zoomLevel > 10) zoomLevel--;
          beep(1800, 40);
          drawUI();
        } else if (screenY >= 171 && screenY < 205) {
          // Add Waypoint
          if (waypointCount < MAX_WAYPOINTS) {
            waypoints[waypointCount].x = 130 - mapOffsetX + (random(-30, 30));
            waypoints[waypointCount].y = 115 - mapOffsetY + (random(-30, 30));
            waypoints[waypointCount].label = "WP";
            waypointCount++;
            beep(2800, 80);
            setLED(true, true, false); // Amber flash
            drawUI();
            setLED(false, true, false);
          }
        }
      } else if (screenX < 260 && screenY >= 27 && screenY < 206) {
        // Touch on map pans toward touch point
        mapOffsetX += (130 - screenX) / 3;
        mapOffsetY += (115 - screenY) / 3;
        beep(2100, 20);
        drawUI();
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n=======================================================");
  Serial.println("  17 OFFLINE OSM TRAIL NAVIGATOR - PARIS (CYD 2.8)");
  Serial.println("  Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("  Live Station: Paris, France (JN18eu)");
  Serial.println("=======================================================\n");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLED(false, false, false);
  beep(2200, 70);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  initSD();

  // Seed Paris landmark waypoints
  waypoints[0] = { 130, 115, "EIFFEL" };
  waypoints[1] = { 65, 145, "SEINE" };
  waypoints[2] = { 185, 80, "TROCADERO" };
  waypointCount = 3;

  drawUI();
}

unsigned long lastWalkTick = 0;

void loop() {
  checkTouch();

  // Subtle telemetry distance step every 6 seconds
  if (millis() - lastWalkTick >= 6000) {
    lastWalkTick = millis();
    tripDistKm += 0.02;
    // Refresh only the bottom telemetry bar to maintain crisp responsiveness
    tft.fillRect(0, 206, 220, 34, tft.color565(20, 20, 25));
    tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 20, 25));
    tft.setTextSize(1);
    char telem1[36];
    snprintf(telem1, sizeof(telem1), "LAT: %.4f° N  LON: %.4f° E", currentLat, currentLon);
    tft.drawString(telem1, 8, 210, 1);

    char telem2[36];
    snprintf(telem2, sizeof(telem2), "ALT: %dm MSL  DIST: %.2f km", currentAltM, tripDistKm);
    tft.drawString(telem2, 8, 222, 1);
  }

  delay(15);
}
