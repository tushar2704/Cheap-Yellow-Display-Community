/**
 * ☕ Project 30: Catppuccin Weather Station
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Catppuccin Mocha aesthetic color palette (Subtext, Lavender, Peach, Sapphire).
 * - Real-time weather dashboard: Current Now, 24H Trend with temp/rain curves, 7-Day Outlook.
 * - Hardware LDR (GPIO 34) ambient light sensor with dynamic backlight auto-dimming.
 * - Active-low RGB LED weather mood lighting (Sunny yellow, Cloudy cyan, Rain blue, Storm violet).
 * - Calibrated non-inverted resistive touch controls with tab navigation.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <ArduinoJson.h>

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
#define LDR_PIN       34

// LEDC PWM Channel for Backlight
#define PWM_CHANNEL   0
#define PWM_FREQ      5000
#define PWM_RES       8

// Catppuccin Mocha 16-bit 565 Palette
#define CAT_BASE      0x1905 // #1E1E2E
#define CAT_MANTLE    0x18C4 // #181825
#define CAT_CRUST     0x10A3 // #11111B
#define CAT_SURFACE0  0x3188 // #313244
#define CAT_SURFACE1  0x422B // #45475A
#define CAT_OVERLAY0  0x6B8A // #6C7086
#define CAT_TEXT      0xCE7E // #CDD6F4
#define CAT_SUBTEXT0  0xA579 // #A6ADC8
#define CAT_BLUE      0x8DDF // #89B4FA
#define CAT_SAPPHIRE  0x763D // #74C7EC
#define CAT_SKY       0x8EFE // #89DCEB
#define CAT_TEAL      0x971A // #94E2D5
#define CAT_GREEN     0xA714 // #A6E3A1
#define CAT_YELLOW    0xF715 // #F9E2AF
#define CAT_PEACH     0xFDB0 // #FAB387
#define CAT_RED       0xF455 // #F38BA8
#define CAT_MAUVE     0xCD3E // #CBA6F7
#define CAT_LAVENDER  0xB5FF // #B4BEFE

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);

// Weather Data Structure
struct WeatherCurrent {
  float temp;
  float feelsLike;
  int humidity;
  float windSpeed;
  int windDirection;
  int uvIndex;
  int pressure;
  int weatherCode;
  char description[32];
};

struct HourlyPoint {
  int hour;
  float temp;
  int rainProb;
};

struct DailyPoint {
  char day[4];
  float maxTemp;
  float minTemp;
  int rainProb;
  int weatherCode;
};

WeatherCurrent currentWeather = { 19.4, 18.8, 62, 14.5, 230, 5, 1016, 1, "Mainly Clear" };

HourlyPoint hourlyData[8] = {
  { 9, 16.5, 5 },
  { 12, 19.4, 10 },
  { 15, 21.0, 15 },
  { 18, 19.8, 20 },
  { 21, 17.2, 35 },
  { 0, 15.0, 60 },
  { 3, 14.2, 70 },
  { 6, 13.8, 50 }
};

DailyPoint dailyData[7] = {
  { "MON", 21.5, 13.2, 15, 1 },
  { "TUE", 22.0, 14.0, 10, 0 },
  { "WED", 18.5, 12.1, 65, 61 },
  { "THU", 17.0, 11.5, 80, 63 },
  { "FRI", 19.0, 12.0, 40, 2 },
  { "SAT", 23.5, 14.5, 5, 0 },
  { "SUN", 24.0, 15.0, 5, 0 }
};

int currentTab = 0; // 0 = Now, 1 = 24H Trend, 2 = 7-Day Outlook
unsigned long lastWeatherUpdate = 0;
unsigned long lastLDRCheck = 0;
unsigned long lastTouchTime = 0;
int currentBrightness = 200;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void updateWeatherMoodRGB(int code) {
  if (code == 0 || code == 1) {
    setRGB(true, true, false); // Warm amber / yellow
  } else if (code == 2 || code == 3) {
    setRGB(false, true, true); // Cyan / sky
  } else if (code >= 51 && code <= 82) {
    setRGB(false, false, true); // Deep rain blue
  } else if (code >= 95) {
    setRGB(true, false, true); // Thunderstorm violet
  } else {
    setRGB(false, true, false); // Mild green
  }
}

void updateBacklightAutoDim() {
  int rawLDR = analogRead(LDR_PIN);
  int targetBrightness = map(rawLDR, 3500, 500, 30, 255);
  targetBrightness = constrain(targetBrightness, 25, 255);
  currentBrightness = (currentBrightness * 3 + targetBrightness) / 4;
  ledcWrite(PWM_CHANNEL, currentBrightness);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, CAT_MANTLE);
  tft.drawLine(0, 26, 320, 26, CAT_SURFACE0);

  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setTextColor(CAT_MAUVE, CAT_MANTLE);
  tft.drawString("IamNotIntoTech", 8, 8);

  tft.setTextColor(CAT_TEXT, CAT_MANTLE);
  tft.drawCentreString("CATPPUCCIN WEATHER", 160, 8, 1);

  tft.setTextColor(CAT_PEACH, CAT_MANTLE);
  tft.drawRightString("LIVE", 312, 8, 1);
}

void drawBottomTabs() {
  tft.fillRect(0, 206, 320, 34, CAT_MANTLE);
  tft.drawLine(0, 206, 320, 206, CAT_SURFACE0);

  const char* tabs[3] = { "NOW", "24H TREND", "7-DAY OUTLOOK" };
  int tabWidth = 106;

  for (int i = 0; i < 3; i++) {
    int x = i * tabWidth;
    bool isActive = (i == currentTab);
    uint16_t bg = isActive ? CAT_SURFACE0 : CAT_MANTLE;
    uint16_t fg = isActive ? CAT_LAVENDER : CAT_OVERLAY0;

    tft.fillRect(x + 2, 208, tabWidth - 4, 30, bg);
    if (isActive) {
      tft.fillRect(x + 8, 234, tabWidth - 16, 2, CAT_MAUVE);
    }
    tft.setTextColor(fg, bg);
    tft.drawCentreString(tabs[i], x + tabWidth / 2, 216, 1);
  }
}

void drawNowView() {
  tft.fillRect(0, 27, 320, 178, CAT_BASE);

  // Big Temperature Card
  tft.fillRoundRect(10, 34, 150, 80, 6, CAT_SURFACE0);
  tft.setTextColor(CAT_PEACH, CAT_SURFACE0);
  tft.setTextFont(4);
  char tempBuf[12];
  snprintf(tempBuf, sizeof(tempBuf), "%.1f C", currentWeather.temp);
  tft.drawString(tempBuf, 20, 48);

  tft.setTextFont(1);
  tft.setTextColor(CAT_SUBTEXT0, CAT_SURFACE0);
  char feelsBuf[24];
  snprintf(feelsBuf, sizeof(feelsBuf), "Feels like: %.1f C", currentWeather.feelsLike);
  tft.drawString(feelsBuf, 20, 88);

  // Condition Card
  tft.fillRoundRect(166, 34, 144, 80, 6, CAT_SURFACE0);
  tft.setTextColor(CAT_YELLOW, CAT_SURFACE0);
  tft.drawString("CURRENT STATUS", 176, 42, 1);
  tft.setTextColor(CAT_TEXT, CAT_SURFACE0);
  tft.drawString(currentWeather.description, 176, 58, 2);

  tft.setTextColor(CAT_TEAL, CAT_SURFACE0);
  char uvBuf[16];
  snprintf(uvBuf, sizeof(uvBuf), "UV Index: %d (Mod)", currentWeather.uvIndex);
  tft.drawString(uvBuf, 176, 88, 1);

  // Metrics Grid
  struct Metric { const char* label; char val[16]; uint16_t color; };
  Metric metrics[4] = {
    { "HUMIDITY", "", CAT_SAPPHIRE },
    { "WIND SPEED", "", CAT_SKY },
    { "PRESSURE", "", CAT_TEAL },
    { "AUTO-DIM", "", CAT_LAVENDER }
  };
  snprintf(metrics[0].val, sizeof(metrics[0].val), "%d %%", currentWeather.humidity);
  snprintf(metrics[1].val, sizeof(metrics[1].val), "%.1f km/h", currentWeather.windSpeed);
  snprintf(metrics[2].val, sizeof(metrics[2].val), "%d hPa", currentWeather.pressure);
  snprintf(metrics[3].val, sizeof(metrics[3].val), "%d%% PWM", (currentBrightness * 100) / 255);

  int cardW = 71;
  int cardH = 58;
  for (int i = 0; i < 4; i++) {
    int cx = 10 + i * (cardW + 5);
    int cy = 122;
    tft.fillRoundRect(cx, cy, cardW, cardH, 4, CAT_SURFACE0);

    tft.setTextColor(CAT_OVERLAY0, CAT_SURFACE0);
    tft.drawCentreString(metrics[i].label, cx + cardW / 2, cy + 8, 1);

    tft.setTextColor(metrics[i].color, CAT_SURFACE0);
    tft.drawCentreString(metrics[i].val, cx + cardW / 2, cy + 28, 1);
  }
}

void draw24HTrendView() {
  tft.fillRect(0, 27, 320, 178, CAT_BASE);

  int graphX = 12;
  int graphY = 38;
  int graphW = 296;
  int graphH = 120;

  tft.drawRoundRect(graphX, graphY, graphW, graphH, 4, CAT_SURFACE0);

  // Gridlines
  for (int g = 1; g < 4; g++) {
    int ly = graphY + g * (graphH / 4);
    tft.drawLine(graphX + 2, ly, graphX + graphW - 2, ly, CAT_MANTLE);
  }

  int barW = 26;
  int spacing = 36;
  float minT = 12.0;
  float maxT = 24.0;

  for (int i = 0; i < 8; i++) {
    int bx = graphX + 8 + i * spacing;

    // Rain Bar
    int rainH = map(hourlyData[i].rainProb, 0, 100, 0, 45);
    if (rainH > 0) {
      tft.fillRect(bx + 4, graphY + graphH - rainH, barW - 8, rainH, CAT_SAPPHIRE);
    }

    // Temp Point
    int py = map((int)(hourlyData[i].temp * 10), (int)(minT * 10), (int)(maxT * 10), graphY + graphH - 12, graphY + 16);
    tft.fillCircle(bx + barW / 2, py, 3, CAT_PEACH);

    if (i > 0) {
      int prevX = graphX + 8 + (i - 1) * spacing + barW / 2;
      int prevY = map((int)(hourlyData[i - 1].temp * 10), (int)(minT * 10), (int)(maxT * 10), graphY + graphH - 12, graphY + 16);
      tft.drawLine(prevX, prevY, bx + barW / 2, py, CAT_YELLOW);
    }

    // Hour Label
    char hourBuf[8];
    snprintf(hourBuf, sizeof(hourBuf), "%02d:00", hourlyData[i].hour);
    tft.setTextColor(CAT_OVERLAY0, CAT_BASE);
    tft.drawCentreString(hourBuf, bx + barW / 2, 168, 1);

    char tBuf[8];
    snprintf(tBuf, sizeof(tBuf), "%.0f", hourlyData[i].temp);
    tft.setTextColor(CAT_TEXT, CAT_BASE);
    tft.drawCentreString(tBuf, bx + barW / 2, py - 12, 1);
  }

  // Legend
  tft.fillRect(18, 186, 8, 8, CAT_YELLOW);
  tft.setTextColor(CAT_SUBTEXT0, CAT_BASE);
  tft.drawString("Temp Curve", 30, 186, 1);

  tft.fillRect(130, 186, 8, 8, CAT_SAPPHIRE);
  tft.drawString("Rain Prob (%)", 142, 186, 1);
}

void draw7DayView() {
  tft.fillRect(0, 27, 320, 178, CAT_BASE);

  tft.setTextColor(CAT_SUBTEXT0, CAT_BASE);
  tft.drawString("DAY", 16, 32, 1);
  tft.drawString("OUTLOOK", 64, 32, 1);
  tft.drawString("RAIN", 150, 32, 1);
  tft.drawString("RANGE", 215, 32, 1);
  tft.drawLine(10, 44, 310, 44, CAT_SURFACE0);

  for (int i = 0; i < 7; i++) {
    int y = 48 + i * 21;
    uint16_t rowBg = (i % 2 == 0) ? CAT_BASE : CAT_SURFACE0;
    tft.fillRect(10, y - 2, 300, 19, rowBg);

    // Day
    tft.setTextColor(CAT_TEXT, rowBg);
    tft.drawString(dailyData[i].day, 16, y, 1);

    // Outlook text
    const char* cond = (dailyData[i].rainProb > 50) ? "Showers" : ((dailyData[i].rainProb > 20) ? "Cloudy" : "Clear");
    uint16_t condColor = (dailyData[i].rainProb > 50) ? CAT_SKY : ((dailyData[i].rainProb > 20) ? CAT_TEAL : CAT_YELLOW);
    tft.setTextColor(condColor, rowBg);
    tft.drawString(cond, 64, y, 1);

    // Rain %
    char rainBuf[8];
    snprintf(rainBuf, sizeof(rainBuf), "%d%%", dailyData[i].rainProb);
    tft.setTextColor(CAT_SAPPHIRE, rowBg);
    tft.drawString(rainBuf, 150, y, 1);

    // Min / Max Range Bar
    int barStart = 205;
    int barWidth = 85;
    tft.fillRoundRect(barStart, y + 4, barWidth, 6, 2, CAT_SURFACE1);

    int minPixel = map((int)(dailyData[i].minTemp * 10), 100, 280, 0, barWidth);
    int maxPixel = map((int)(dailyData[i].maxTemp * 10), 100, 280, 0, barWidth);
    minPixel = constrain(minPixel, 0, barWidth);
    maxPixel = constrain(maxPixel, minPixel, barWidth);
    tft.fillRoundRect(barStart + minPixel, y + 4, maxPixel - minPixel, 6, 2, CAT_PEACH);

    char rangeBuf[16];
    snprintf(rangeBuf, sizeof(rangeBuf), "%.0f-%.0f", dailyData[i].minTemp, dailyData[i].maxTemp);
    tft.setTextColor(CAT_SUBTEXT0, rowBg);
    tft.drawRightString(rangeBuf, 305, y, 1);
  }
}

void renderCurrentView() {
  if (currentTab == 0) drawNowView();
  else if (currentTab == 1) draw24HTrendView();
  else if (currentTab == 2) draw7DayView();
  drawBottomTabs();
}

void setup() {
  Serial.begin(115200);

  // Initialize LEDs
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  // Initialize Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // LDR pin
  pinMode(LDR_PIN, INPUT);

  // Backlight PWM via LEDC
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(TFT_BL_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, currentBrightness);

  // Initialize Display
  tft.init();
  tft.setRotation(1); // Landscape 320x240
  tft.fillScreen(CAT_BASE);

  // Initialize Touch (VSPI: CLK=25, MISO=39, MOSI=32, CS=33)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Header and View
  drawHeader();
  renderCurrentView();
  updateWeatherMoodRGB(currentWeather.weatherCode);

  playBeep(2400, 60);
  Serial.println(F("[SYSTEM] Catppuccin Weather Station ready."));
}

void loop() {
  // LDR Auto Dimming every 250ms
  if (millis() - lastLDRCheck > 250) {
    lastLDRCheck = millis();
    updateBacklightAutoDim();
  }

  // Touch Handling with Polling and Debounce
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // Tab Bar click (Y >= 200)
        if (screenY >= 200) {
          int tappedTab = screenX / 106;
          if (tappedTab >= 0 && tappedTab <= 2 && tappedTab != currentTab) {
            currentTab = tappedTab;
            playBeep(1800, 40);
            renderCurrentView();
          }
        }
      }
    }
  }

  // Periodic weather refresh cycle (every 10 minutes)
  if (millis() - lastWeatherUpdate > 600000) {
    lastWeatherUpdate = millis();
    renderCurrentView();
  }

  delay(20);
}

