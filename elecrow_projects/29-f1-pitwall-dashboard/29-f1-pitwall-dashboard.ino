/**
 * 🏎️ Project 29: Formula 1 Pit Wall Clock & Telemetry
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Grand Prix weekend countdown clock ticking down to lights out.
 * - 2D Vector circuit layout renderer with DRS zones and track telemetry.
 * - Live World Championship constructor & driver standings.
 * - Pit wall environmental station: track temp, air temp, and rain probability.
 * - Hardware active-low RGB LED reflecting FIA flag status (Green, Yellow VSC, Red Flag).
 * - Five-red-lights start sequence audio simulation on buzzer.
 * - Calibrated non-inverted resistive touch controls.
 */

#include <Arduino.h>
#include <SPI.h>
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

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);

int currentScreen = 0; // 0 = Circuit & Countdown, 1 = Standings, 2 = Weather & Status
int currentCircuit = 0;
unsigned long lastSecTick = 0;
unsigned long lastTouchTime = 0;

struct GrandPrixInfo {
  const char* gpName;
  const char* circuitName;
  const char* country;
  float lapLengthKm;
  int laps;
  int countdownDays;
  int countdownHours;
  int countdownMins;
  int countdownSecs;
};

const int TOTAL_CIRCUITS = 4;
GrandPrixInfo circuits[TOTAL_CIRCUITS] = {
  { "MONACO GRAND PRIX",    "Circuit de Monaco",   "Monaco",         3.337, 78, 2, 14, 32, 18 },
  { "BRITISH GRAND PRIX",   "Silverstone Circuit", "Great Britain",  5.891, 52, 6,  8, 45,  0 },
  { "BELGIAN GRAND PRIX",   "Spa-Francorchamps",   "Belgium",        7.004, 44, 12, 18, 10, 42 },
  { "JAPANESE GRAND PRIX",  "Suzuka Circuit",      "Japan",          5.807, 53, 19,  4, 20, 15 }
};

struct DriverStanding {
  int pos;
  const char* name;
  const char* team;
  int points;
  uint16_t teamColor;
};

DriverStanding drivers[5] = {
  { 1, "M. Verstappen", "Red Bull Racing", 255, TFT_NAVY },
  { 2, "L. Norris",     "McLaren F1",      208, TFT_ORANGE },
  { 3, "C. Leclerc",    "Scuderia Ferrari", 192, TFT_RED },
  { 4, "O. Piastri",    "McLaren F1",      164, TFT_ORANGE },
  { 5, "C. Sainz",      "Scuderia Ferrari", 154, TFT_RED }
};

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void playLightsOutSequence() {
  for (int i = 0; i < 5; i++) {
    setLED(true, false, false); // Red light
    tone(BUZZER_PIN, 1200, 150);
    delay(1000);
  }
  delay(1200); // Random delay
  setLED(false, true, false);  // Green light
  tone(BUZZER_PIN, 2400, 400); // LIGHTS OUT AND AWAY WE GO!
  delay(500);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, tft.color565(180, 20, 20));
  tft.setTextColor(TFT_WHITE, tft.color565(180, 20, 20));
  tft.setTextSize(1);
  tft.drawString("FORMULA 1 PIT WALL", 8, 6, 2);

  tft.setTextColor(TFT_WHITE, tft.color565(180, 20, 20));
  tft.drawRightString("TRACK: GREEN ●", 312, 6, 2);
}

void drawVectorCircuitTrack(int cx, int cy) {
  // Draw 2D Vector Circuit Outline
  int pts[9][2] = {
    { cx - 45, cy + 25 }, { cx + 25, cy + 25 }, { cx + 45, cy + 10 },
    { cx + 40, cy - 20 }, { cx + 15, cy - 35 }, { cx - 25, cy - 35 },
    { cx - 45, cy - 20 }, { cx - 35, cy - 5 },  { cx - 45, cy + 25 }
  };

  for (int i = 0; i < 8; i++) {
    tft.drawLine(pts[i][0], pts[i][1], pts[i+1][0], pts[i+1][1], TFT_WHITE);
    tft.drawLine(pts[i][0], pts[i][1] + 1, pts[i+1][0], pts[i+1][1] + 1, TFT_WHITE);
  }

  // Start / Finish line in yellow
  tft.drawFastVLine(cx - 45, cy + 18, 14, TFT_YELLOW);

  // DRS Zone marker in Green
  tft.drawLine(pts[0][0], pts[0][1] - 2, pts[1][0], pts[1][1] - 2, TFT_GREEN);
}

void drawCircuitScreen() {
  GrandPrixInfo& gp = circuits[currentCircuit];

  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // GP Title & Circuit Name
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString(gp.gpName, 12, 32);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString(gp.circuitName, 14, 52, 2);

  // Vector Track Map Box (X: 14 to 124, Y: 72 to 148)
  tft.fillRoundRect(12, 72, 114, 80, 4, tft.color565(15, 20, 28));
  tft.drawRoundRect(12, 72, 114, 80, 4, TFT_DARKGREY);
  drawVectorCircuitTrack(69, 112);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(15, 20, 28));
  tft.drawCentreString("2D TRACK MAP", 69, 138, 1);

  // T-Minus Countdown Clock on Right (X: 134 to 308)
  tft.fillRoundRect(134, 72, 176, 80, 4, tft.color565(20, 25, 35));
  tft.drawRoundRect(134, 72, 176, 80, 4, TFT_RED);

  tft.setTextColor(TFT_RED, tft.color565(20, 25, 35));
  tft.setTextSize(1);
  tft.drawString("LIGHTS OUT COUNTDOWN", 142, 78, 1);

  char countBuf[24];
  snprintf(countBuf, sizeof(countBuf), "%02d:%02d:%02d", gp.countdownHours, gp.countdownMins, gp.countdownSecs);
  tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
  tft.setTextSize(3);
  tft.drawString(countBuf, 146, 94);

  char daysBuf[24];
  snprintf(daysBuf, sizeof(daysBuf), "%d DAYS REMAINING", gp.countdownDays);
  tft.setTextColor(TFT_YELLOW, tft.color565(20, 25, 35));
  tft.setTextSize(1);
  tft.drawString(daysBuf, 146, 126, 2);

  // Track Specs Bar (Y: 156 to 200)
  tft.fillRoundRect(12, 156, 298, 42, 4, tft.color565(15, 18, 22));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(15, 18, 22));
  char spec1[32];
  snprintf(spec1, sizeof(spec1), "LAP: %.3f km  |  LAPS: %d", gp.lapLengthKm, gp.laps);
  tft.drawString(spec1, 18, 162, 1);

  char spec2[48];
  snprintf(spec2, sizeof(spec2), "TOTAL DISTANCE: %.1f km  |  DRS ZONES: 1", gp.lapLengthKm * gp.laps);
  tft.drawString(spec2, 18, 178, 1);
}

void drawStandingsScreen() {
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // Table Header
  tft.fillRoundRect(8, 30, 304, 20, 2, tft.color565(25, 30, 40));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 40));
  tft.drawString("POS", 14, 34, 1);
  tft.drawString("DRIVER", 48, 34, 1);
  tft.drawString("CONSTRUCTOR", 160, 34, 1);
  tft.drawRightString("PTS", 304, 34, 1);

  for (int i = 0; i < 5; i++) {
    int y = 54 + i * 28;
    uint16_t rowBg = (i % 2 == 0) ? tft.color565(15, 18, 24) : TFT_BLACK;
    tft.fillRect(8, y, 304, 26, rowBg);

    // Position
    tft.setTextColor(TFT_YELLOW, rowBg);
    tft.setTextSize(1);
    tft.drawNumber(drivers[i].pos, 16, y + 4, 2);

    // Driver name
    tft.setTextColor(TFT_WHITE, rowBg);
    tft.drawString(drivers[i].name, 48, y + 4, 2);

    // Team name
    tft.setTextColor(drivers[i].teamColor, rowBg);
    tft.drawString(drivers[i].team, 160, y + 6, 1);

    // Points
    tft.setTextColor(TFT_GREEN, rowBg);
    tft.drawRightString(String(drivers[i].points).c_str(), 304, y + 4, 2);
  }
}

void drawWeatherScreen() {
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // Weather Telemetry Grid
  // Box 1: Track Temp
  tft.fillRoundRect(12, 34, 142, 60, 4, tft.color565(35, 20, 15));
  tft.drawRoundRect(12, 34, 142, 60, 4, TFT_ORANGE);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(35, 20, 15));
  tft.drawString("TRACK TEMP", 20, 40, 1);
  tft.setTextColor(TFT_ORANGE, tft.color565(35, 20, 15));
  tft.setTextSize(3);
  tft.drawString("42.8°C", 20, 54);

  // Box 2: Air Temp
  tft.fillRoundRect(164, 34, 144, 60, 4, tft.color565(15, 25, 35));
  tft.drawRoundRect(164, 34, 144, 60, 4, TFT_CYAN);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(15, 25, 35));
  tft.setTextSize(1);
  tft.drawString("AIR TEMP", 172, 40, 1);
  tft.setTextColor(TFT_CYAN, tft.color565(15, 25, 35));
  tft.setTextSize(3);
  tft.drawString("24.5°C", 172, 54);

  // Box 3: Rain Probability
  tft.fillRoundRect(12, 102, 142, 60, 4, tft.color565(15, 20, 35));
  tft.drawRoundRect(12, 102, 142, 60, 4, TFT_BLUE);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(15, 20, 35));
  tft.setTextSize(1);
  tft.drawString("RAIN RISK", 20, 108, 1);
  tft.setTextColor(TFT_WHITE, tft.color565(15, 20, 35));
  tft.setTextSize(3);
  tft.drawString("10%", 20, 122);

  // Box 4: Wind Vector
  tft.fillRoundRect(164, 102, 144, 60, 4, tft.color565(20, 25, 30));
  tft.drawRoundRect(164, 102, 144, 60, 4, TFT_GREEN);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(20, 25, 30));
  tft.setTextSize(1);
  tft.drawString("WIND SPEED", 172, 108, 1);
  tft.setTextColor(TFT_GREEN, tft.color565(20, 25, 30));
  tft.setTextSize(3);
  tft.drawString("14 km/h", 172, 122);

  // Start Sequence Test Button
  tft.fillRoundRect(12, 168, 296, 32, 4, tft.color565(50, 20, 20));
  tft.drawRoundRect(12, 168, 296, 32, 4, TFT_RED);
  tft.setTextColor(TFT_WHITE, tft.color565(50, 20, 20));
  tft.setTextSize(1);
  tft.drawCentreString("TEST 5-LIGHT START SEQUENCE", 160, 176, 2);
}

void drawBottomTabs() {
  const char* tabs[3] = { "RACE CLOCK", "STANDINGS", "TELEMETRY" };
  int bw = 98;

  for (int i = 0; i < 3; i++) {
    int bx = 8 + i * 104;
    uint16_t bg = (i == currentScreen) ? TFT_DARKCYAN : tft.color565(30, 35, 45);
    uint16_t fg = (i == currentScreen) ? TFT_WHITE : TFT_LIGHTGREY;

    tft.fillRoundRect(bx, 206, bw, 30, 4, bg);
    tft.drawRoundRect(bx, 206, bw, 30, 4, (i == currentScreen) ? TFT_CYAN : TFT_DARKGREY);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.drawCentreString(tabs[i], bx + bw/2, 214, 2);
  }
}

void drawUI() {
  drawHeader();
  if (currentScreen == 0) drawCircuitScreen();
  else if (currentScreen == 1) drawStandingsScreen();
  else drawWeatherScreen();
  drawBottomTabs();
}

void checkTouch() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        // Calibrated non-inverted coordinate mapping for 2.8" CYD
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        if (screenY >= 200) {
          int tabIdx = (screenX - 8) / 104;
          if (tabIdx >= 0 && tabIdx < 3 && tabIdx != currentScreen) {
            currentScreen = tabIdx;
            beep(2000, 40);
            drawUI();
          }
        } else if (currentScreen == 0 && screenY >= 32 && screenY < 70) {
          // Tap title to cycle circuits
          currentCircuit = (currentCircuit + 1) % TOTAL_CIRCUITS;
          beep(2200, 40);
          drawCircuitScreen();
        } else if (currentScreen == 2 && screenY >= 165 && screenY < 202) {
          // Start lights sequence
          playLightsOutSequence();
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Formula 1 Pit Wall Clock Starting for @IamNotIntoTech...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLED(false, true, false); // Green: Track Clear
  beep(2200, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawUI();
}

void loop() {
  checkTouch();

  // 1-second Countdown decrement
  if (millis() - lastSecTick >= 1000) {
    lastSecTick = millis();
    GrandPrixInfo& gp = circuits[currentCircuit];

    if (gp.countdownSecs > 0) {
      gp.countdownSecs--;
    } else {
      gp.countdownSecs = 59;
      if (gp.countdownMins > 0) {
        gp.countdownMins--;
      } else {
        gp.countdownMins = 59;
        if (gp.countdownHours > 0) gp.countdownHours--;
      }
    }

    if (currentScreen == 0) {
      char countBuf[24];
      snprintf(countBuf, sizeof(countBuf), "%02d:%02d:%02d", gp.countdownHours, gp.countdownMins, gp.countdownSecs);
      tft.fillRect(146, 94, 150, 24, tft.color565(20, 25, 35));
      tft.setTextColor(TFT_WHITE, tft.color565(20, 25, 35));
      tft.setTextSize(3);
      tft.drawString(countBuf, 146, 94);
    }
  }

  delay(15);
}
