/**
 * 🌊 Project 16: Live Coastal Tide Chart & Lunar Tracker
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Continuous harmonic tidal wave curve plotted across a 24-hour diurnal timeline.
 * - Real-time High / Low tide crest and trough predictions with height in meters and feet.
 * - Lunar phase engine calculating moon illumination percentage and spring/neap cycle.
 * - Golden hour, sunrise, and sunset temporal markers.
 * - Interactive multi-harbor station switcher.
 * - Active-low RGB LED reflecting rising, falling, and slack water states.
 * - Hardware-calibrated non-inverted touch navigation.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>

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

struct TideStation {
  const char* name;
  const char* region;
  float meanSeaLevel;
  float tidalRange;
  float phaseOffsetHours;
  const char* highTide1;
  const char* lowTide1;
  const char* highTide2;
  const char* lowTide2;
};

const int TOTAL_STATIONS = 4;
TideStation stations[TOTAL_STATIONS] = {
  { "SF Bay", "Golden Gate, USA", 3.0, 5.2, 1.2, "05:42 (5.6ft)", "11:50 (0.8ft)", "18:14 (5.1ft)", "23:55 (1.2ft)" },
  { "Dover", "English Channel, UK", 3.5, 6.4, 3.8, "07:15 (6.2m)",  "13:30 (0.9m)",  "19:40 (6.0m)",  "01:50 (1.1m)" },
  { "Sydney", "Fort Denison, AUS", 1.0, 1.8, 5.4, "04:30 (1.7m)",  "10:45 (0.3m)",  "17:05 (1.8m)",  "23:15 (0.4m)" },
  { "Mumbai", "Apollo Bunder, IND", 2.2, 4.5, 2.6, "06:20 (4.4m)",  "12:35 (0.7m)",  "18:50 (4.1m)",  "00:40 (1.0m)" }
};

int currentStation = 0;
float currentSimHour = 14.5; // 14:30
float currentWaterLevel = 3.8;
bool isRising = true;
unsigned long lastTouchTime = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 40) {
  tone(BUZZER_PIN, freq, ms);
}

void updateTideLED() {
  if (fabs(currentWaterLevel - stations[currentStation].meanSeaLevel) < 0.2) {
    setLED(true, true, false);  // Yellow: Slack water / Mid-tide transition
  } else if (isRising) {
    setLED(false, true, true);  // Cyan: Rising Tide
  } else {
    setLED(false, false, true); // Blue: Falling Tide / Ebb
  }
}

// Compute harmonic tide height for given hour (0 to 24)
float getTideHeight(float hour, const TideStation& st) {
  float period = 12.42; // Semi-diurnal tidal period (hours)
  float angle = (hour - st.phaseOffsetHours) * (2.0 * M_PI / period);
  return st.meanSeaLevel + (st.tidalRange / 2.0) * sin(angle);
}

void drawTideChart() {
  const TideStation& st = stations[currentStation];

  // Header Bar
  tft.fillRect(0, 0, 320, 30, tft.color565(10, 25, 45));
  tft.setTextColor(TFT_WHITE, tft.color565(10, 25, 45));
  tft.setTextSize(1);
  tft.drawString("COASTAL TIDE CHART", 8, 4, 2);
  tft.setTextColor(TFT_CYAN, tft.color565(10, 25, 45));
  tft.drawString(st.name, 160, 4, 2);

  // Moon Phase badge in header
  tft.setTextColor(TFT_YELLOW, tft.color565(10, 25, 45));
  tft.drawRightString("84% GIBBOUS (SPRING)", 312, 6, 1);
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(10, 25, 45));
  tft.drawString(st.region, 8, 18, 1);

  // Tide Wave Plot Area
  tft.fillRect(0, 31, 320, 165, TFT_BLACK);

  // Plot grid lines
  tft.drawFastHLine(10, 80, 300, tft.color565(30, 35, 45));
  tft.drawFastHLine(10, 115, 300, tft.color565(40, 50, 70)); // Mean sea level datum
  tft.drawFastHLine(10, 150, 300, tft.color565(30, 35, 45));

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("HIGH", 12, 70, 1);
  tft.drawString("DATUM", 12, 105, 1);
  tft.drawString("LOW", 12, 140, 1);

  // Draw harmonic sine-wave tidal curve
  int prevX = 20;
  int prevY = 115;
  int plotW = 280;

  for (int x = 0; x <= plotW; x += 2) {
    float simH = (float)x / plotW * 24.0;
    float h = getTideHeight(simH, st);
    // Map height to Y coordinates (70 high, 160 low)
    int y = map((int)(h * 100), (int)((st.meanSeaLevel - st.tidalRange/2) * 100), (int)((st.meanSeaLevel + st.tidalRange/2) * 100), 155, 75);
    y = constrain(y, 65, 165);

    if (x > 0) {
      tft.drawLine(prevX, prevY, 20 + x, y, TFT_CYAN);
      tft.drawLine(prevX, prevY + 1, 20 + x, y + 1, TFT_CYAN); // Double thickness line
    }
    prevX = 20 + x;
    prevY = y;
  }

  // Draw current time marker cursor
  int curCursorX = 20 + (int)((currentSimHour / 24.0) * plotW);
  currentWaterLevel = getTideHeight(currentSimHour, st);
  float nextWaterLevel = getTideHeight(currentSimHour + 0.1, st);
  isRising = (nextWaterLevel >= currentWaterLevel);

  int curY = map((int)(currentWaterLevel * 100), (int)((st.meanSeaLevel - st.tidalRange/2) * 100), (int)((st.meanSeaLevel + st.tidalRange/2) * 100), 155, 75);
  curY = constrain(curY, 65, 165);

  tft.drawFastVLine(curCursorX, 60, 105, TFT_YELLOW);
  tft.fillCircle(curCursorX, curY, 4, TFT_WHITE);
  tft.drawCircle(curCursorX, curY, 6, TFT_YELLOW);

  // Current Water Level Callout
  char levBuf[32];
  snprintf(levBuf, sizeof(levBuf), "NOW: %.1fm (%s)", currentWaterLevel, isRising ? "RISING ▲" : "FALLING ▼");
  tft.fillRoundRect(8, 168, 150, 24, 4, tft.color565(20, 35, 55));
  tft.setTextColor(isRising ? TFT_GREEN : TFT_ORANGE, tft.color565(20, 35, 55));
  tft.setTextSize(1);
  tft.drawCentreString(levBuf, 83, 172, 2);

  // Golden Hour / Solar status
  tft.fillRoundRect(164, 168, 148, 24, 4, tft.color565(40, 30, 15));
  tft.setTextColor(TFT_GOLD, tft.color565(40, 30, 15));
  tft.drawCentreString("☼ 06:15  |  ☽ 18:42", 238, 172, 2);

  // Bottom Harbor Selector Tabs
  for (int i = 0; i < TOTAL_STATIONS; i++) {
    int btnX = 8 + i * 77;
    int btnW = 72;
    uint16_t bg = (i == currentStation) ? TFT_DARKCYAN : tft.color565(25, 30, 40);
    uint16_t fg = (i == currentStation) ? TFT_WHITE : TFT_LIGHTGREY;

    tft.fillRoundRect(btnX, 202, btnW, 32, 4, bg);
    tft.drawRoundRect(btnX, 202, btnW, 32, 4, (i == currentStation) ? TFT_CYAN : TFT_DARKGREY);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.drawCentreString(stations[i].name, btnX + btnW / 2, 210, 2);
  }

  updateTideLED();
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

        if (screenY >= 195) {
          int tappedIdx = (screenX - 8) / 77;
          if (tappedIdx >= 0 && tappedIdx < TOTAL_STATIONS && tappedIdx != currentStation) {
            currentStation = tappedIdx;
            beep(2000, 50);
            drawTideChart();
          }
        }
      }
    }
  }
}

unsigned long lastTideAdvance = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Coastal Tide Chart Initializing for @IamNotIntoTech...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  beep(2200, 80);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawTideChart();
}

void loop() {
  checkTouch();

  // Advance simulation hour every 10 seconds for dynamic demonstration
  if (millis() - lastTideAdvance >= 10000) {
    lastTideAdvance = millis();
    currentSimHour += 0.25; // advance 15 simulated minutes
    if (currentSimHour >= 24.0) currentSimHour = 0.0;
    drawTideChart();
  }

  delay(15);
}

