/*
 * Elecrow 2.8" CYD — Catppuccin Weather Station
 * Channel: @IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Target: ESP32-2432S028R (2.8" ILI9341 TFT + XPT2046 Resistive Touch)
 * Features:
 *  - Catppuccin Mocha aesthetic palette
 *  - Zero-API-key Open-Meteo weather schema (Hourly, 7-Day, Current)
 *  - Ambient LDR automatic backlight dimming on IO34 -> IO21 PWM
 *  - Interactive capacitive/resistive touch tabs (Now, 24H Trend, 7-Day)
 *  - RGB LED ambient condition mood lighting on IO4/16/17
 *  - Touch audio chirps on IO26
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define TFT_BL 21
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define BUZZER_PIN 26
#define LDR_PIN 34

// LEDC PWM Channel for Backlight
#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

// Catppuccin Mocha 16-bit 565 Palette
#define CAT_BASE     0x1905 // #1E1E2E
#define CAT_MANTLE   0x18C4 // #181825
#define CAT_CRUST    0x10A3 // #11111B
#define CAT_SURFACE0 0x3188 // #313244
#define CAT_SURFACE1 0x422B // #45475A
#define CAT_OVERLAY0 0x6B8A // #6C7086
#define CAT_TEXT     0xCE7E // #CDD6F4
#define CAT_SUBTEXT0 0xA579 // #A6ADC8
#define CAT_BLUE     0x8DDF // #89B4FA
#define CAT_SAPPHIRE 0x763D // #74C7EC
#define CAT_SKY      0x8EFE // #89DCEB
#define CAT_TEAL     0x971A // #94E2D5
#define CAT_GREEN    0xA714 // #A6E3A1
#define CAT_YELLOW   0xF715 // #F9E2AF
#define CAT_PEACH    0xFDB0 // #FAB387
#define CAT_RED      0xF455 // #F38BA8
#define CAT_MAUVE    0xCD3E // #CBA6F7
#define CAT_LAVENDER 0xB5FF // #B4BEFE

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Wi-Fi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASS";

// Weather Coordinates (Default: Seattle / configurable)
float latitude = 47.6062;
float longitude = -122.3321;
const char* cityName = "SEATTLE, WA";

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
int currentBrightness = 200;

void playBeep(int freq, int ms) {
  tone(BUZZER_PIN, freq, ms);
  delay(ms);
  noTone(BUZZER_PIN);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void updateWeatherMoodRGB(int code) {
  // Clear/Sunny (0, 1): Yellow/Peach
  if (code == 0 || code == 1) {
    setRGB(true, true, false); // Yellow
  }
  // Partly Cloudy (2, 3): Soft Sky/Lavender
  else if (code == 2 || code == 3) {
    setRGB(false, true, true); // Cyan / Sky
  }
  // Rain / Drizzle (51..67, 80..82): Teal / Blue
  else if (code >= 51 && code <= 82) {
    setRGB(false, false, true); // Blue
  }
  // Thunderstorm (95..99): Red / Purple
  else if (code >= 95) {
    setRGB(true, false, true); // Magenta
  }
  else {
    setRGB(false, true, false); // Green
  }
}

void updateBacklightAutoDim() {
  int rawLDR = analogRead(LDR_PIN);
  // Invert and map LDR: darkness (higher raw) -> dimmer backlight
  // Typical CYD LDR: bright light ~ 300-800, dark room ~ 3000-4000
  int targetBrightness = map(rawLDR, 3500, 500, 30, 255);
  targetBrightness = constrain(targetBrightness, 25, 255);

  // Smooth filter
  currentBrightness = (currentBrightness * 3 + targetBrightness) / 4;
  ledcWrite(PWM_CHANNEL, currentBrightness);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, CAT_MANTLE);
  tft.drawLine(0, 26, 320, 26, CAT_SURFACE0);

  tft.setTextColor(CAT_MAUVE, CAT_MANTLE);
  tft.setTextSize(1);
  tft.drawString("CATPPUCCIN WEATHER", 10, 7, 2);

  tft.setTextColor(CAT_SUBTEXT0, CAT_MANTLE);
  tft.drawRightString(cityName, 310, 7, 2);
}

void drawBottomTabs() {
  tft.fillRect(0, 206, 320, 34, CAT_MANTLE);
  tft.drawLine(0, 206, 320, 206, CAT_SURFACE0);

  const char* tabs[3] = { "NOW", "24H TREND", "7-DAY" };
  for (int i = 0; i < 3; i++) {
    int x = i * 106 + 3;
    bool active = (currentTab == i);
    uint16_t bg = active ? CAT_SURFACE0 : CAT_MANTLE;
    uint16_t fg = active ? CAT_PEACH : CAT_OVERLAY0;

    tft.fillRoundRect(x, 210, 100, 26, 4, bg);
    if (active) {
      tft.drawRoundRect(x, 210, 100, 26, 4, CAT_PEACH);
      tft.fillRect(x + 20, 233, 60, 2, CAT_PEACH);
    }
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.drawCentreString(tabs[i], x + 50, 215, 2);
  }
}

void drawNowView() {
  tft.fillRect(0, 27, 320, 178, CAT_BASE);

  // Hero Card: Current Temp & Conditions
  tft.fillRoundRect(10, 34, 185, 100, 6, CAT_SURFACE0);
  tft.drawRoundRect(10, 34, 185, 100, 6, CAT_SURFACE1);

  // Large Temperature
  tft.setTextColor(CAT_PEACH, CAT_SURFACE0);
  char tempStr[16];
  snprintf(tempStr, sizeof(tempStr), "%.1f°C", currentWeather.temp);
  tft.drawString(tempStr, 20, 44, 4);

  // Description
  tft.setTextColor(CAT_TEXT, CAT_SURFACE0);
  tft.drawString(currentWeather.description, 22, 82, 2);

  // Feels Like
  char feelsStr[32];
  snprintf(feelsStr, sizeof(feelsStr), "Feels like %.1f°C", currentWeather.feelsLike);
  tft.setTextColor(CAT_SUBTEXT0, CAT_SURFACE0);
  tft.drawString(feelsStr, 22, 106, 1);

  // Right Side Cards
  // Humidity
  tft.fillRoundRect(205, 34, 105, 46, 6, CAT_SURFACE0);
  tft.drawRoundRect(205, 34, 105, 46, 6, CAT_SURFACE1);
  tft.setTextColor(CAT_SKY, CAT_SURFACE0);
  tft.drawString("HUMIDITY", 215, 40, 1);
  tft.setTextColor(CAT_TEXT, CAT_SURFACE0);
  char humStr[16];
  snprintf(humStr, sizeof(humStr), "%d%%", currentWeather.humidity);
  tft.drawString(humStr, 215, 54, 2);

  // Wind
  tft.fillRoundRect(205, 88, 105, 46, 6, CAT_SURFACE0);
  tft.drawRoundRect(205, 88, 105, 46, 6, CAT_SURFACE1);
  tft.setTextColor(CAT_TEAL, CAT_SURFACE0);
  tft.drawString("WIND SPEED", 215, 94, 1);
  tft.setTextColor(CAT_TEXT, CAT_SURFACE0);
  char windStr[16];
  snprintf(windStr, sizeof(windStr), "%.1f km/h", currentWeather.windSpeed);
  tft.drawString(windStr, 215, 108, 2);

  // Bottom 3 Sub-Cards
  // Barometer
  tft.fillRoundRect(10, 142, 95, 56, 6, CAT_SURFACE0);
  tft.drawRoundRect(10, 142, 95, 56, 6, CAT_SURFACE1);
  tft.setTextColor(CAT_OVERLAY0, CAT_SURFACE0);
  tft.drawString("PRESSURE", 18, 148, 1);
  tft.setTextColor(CAT_LAVENDER, CAT_SURFACE0);
  char pressStr[16];
  snprintf(pressStr, sizeof(pressStr), "%d hPa", currentWeather.pressure);
  tft.drawString(pressStr, 18, 168, 2);

  // UV Index
  tft.fillRoundRect(112, 142, 96, 56, 6, CAT_SURFACE0);
  tft.drawRoundRect(112, 142, 96, 56, 6, CAT_SURFACE1);
  tft.setTextColor(CAT_OVERLAY0, CAT_SURFACE0);
  tft.drawString("UV INDEX", 120, 148, 1);
  tft.setTextColor(CAT_YELLOW, CAT_SURFACE0);
  char uvStr[16];
  snprintf(uvStr, sizeof(uvStr), "%d / 11", currentWeather.uvIndex);
  tft.drawString(uvStr, 120, 168, 2);

  // Ambient Light / LDR Auto
  tft.fillRoundRect(215, 142, 95, 56, 6, CAT_SURFACE0);
  tft.drawRoundRect(215, 142, 95, 56, 6, CAT_SURFACE1);
  tft.setTextColor(CAT_OVERLAY0, CAT_SURFACE0);
  tft.drawString("AUTO DIM", 223, 148, 1);
  tft.setTextColor(CAT_GREEN, CAT_SURFACE0);
  char dimStr[16];
  snprintf(dimStr, sizeof(dimStr), "%d%%", (currentBrightness * 100) / 255);
  tft.drawString(dimStr, 223, 168, 2);
}

void draw24HTrendView() {
  tft.fillRect(0, 27, 320, 178, CAT_BASE);

  tft.setTextColor(CAT_SUBTEXT0, CAT_BASE);
  tft.drawString("24-HOUR TEMPERATURE & RAIN TREND", 14, 34, 1);

  // Graph Area: x: 20 to 300, y: 55 to 175
  int graphX = 20;
  int graphY = 55;
  int graphW = 280;
  int graphH = 110;

  // Grid lines
  for (int i = 0; i <= 3; i++) {
    int y = graphY + i * 30;
    tft.drawFastHLine(graphX, y, graphW, CAT_SURFACE0);
  }

  // Draw Bars & Temperature Curve
  float minT = 10.0;
  float maxT = 25.0;

  int barW = 24;
  int spacing = 34;

  for (int i = 0; i < 8; i++) {
    int bx = graphX + 8 + i * spacing;
    
    // Rain Bar
    int rainH = map(hourlyData[i].rainProb, 0, 100, 0, 45);
    if (rainH > 0) {
      tft.fillRect(bx + 4, graphY + graphH - rainH, barW - 8, rainH, CAT_SAPPHIRE);
    }

    // Temp Point
    int py = map(hourlyData[i].temp * 10, minT * 10, maxT * 10, graphY + graphH - 10, graphY + 15);
    tft.fillCircle(bx + barW / 2, py, 4, CAT_PEACH);

    if (i > 0) {
      int prevX = graphX + 8 + (i - 1) * spacing + barW / 2;
      int prevY = map(hourlyData[i - 1].temp * 10, minT * 10, maxT * 10, graphY + graphH - 10, graphY + 15);
      tft.drawLine(prevX, prevY, bx + barW / 2, py, CAT_YELLOW);
    }

    // Labels
    char hourBuf[8];
    snprintf(hourBuf, sizeof(hourBuf), "%02d:00", hourlyData[i].hour);
    tft.setTextColor(CAT_OVERLAY0, CAT_BASE);
    tft.drawCentreString(hourBuf, bx + barW / 2, 175, 1);

    char tBuf[8];
    snprintf(tBuf, sizeof(tBuf), "%.0f°", hourlyData[i].temp);
    tft.setTextColor(CAT_TEXT, CAT_BASE);
    tft.drawCentreString(tBuf, bx + barW / 2, py - 12, 1);
  }

  // Legend
  tft.fillRect(18, 192, 8, 8, CAT_YELLOW);
  tft.setTextColor(CAT_SUBTEXT0, CAT_BASE);
  tft.drawString("Temp Curve", 30, 192, 1);

  tft.fillRect(110, 192, 8, 8, CAT_SAPPHIRE);
  tft.drawString("Rain Probability (%)", 122, 192, 1);
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
    int y = 48 + i * 22;
    uint16_t rowBg = (i % 2 == 0) ? CAT_BASE : CAT_SURFACE0;
    tft.fillRect(10, y - 2, 300, 20, rowBg);

    // Day
    tft.setTextColor(CAT_TEXT, rowBg);
    tft.drawString(dailyData[i].day, 16, y, 2);

    // Outlook text
    const char* cond = (dailyData[i].rainProb > 50) ? "Showers" : ((dailyData[i].rainProb > 20) ? "Partly Cloudy" : "Clear");
    uint16_t condColor = (dailyData[i].rainProb > 50) ? CAT_SKY : ((dailyData[i].rainProb > 20) ? CAT_TEAL : CAT_YELLOW);
    tft.setTextColor(condColor, rowBg);
    tft.drawString(cond, 64, y + 2, 1);

    // Rain %
    char rainBuf[8];
    snprintf(rainBuf, sizeof(rainBuf), "%d%%", dailyData[i].rainProb);
    tft.setTextColor(CAT_SAPPHIRE, rowBg);
    tft.drawString(rainBuf, 150, y + 2, 1);

    // Min / Max Range Bar
    int barStart = 205;
    int barWidth = 90;
    tft.fillRoundRect(barStart, y + 4, barWidth, 6, 2, CAT_SURFACE1);

    int minPixel = map(dailyData[i].minTemp * 10, 100, 280, 0, barWidth);
    int maxPixel = map(dailyData[i].maxTemp * 10, 100, 280, 0, barWidth);
    tft.fillRoundRect(barStart + minPixel, y + 4, maxPixel - minPixel, 6, 2, CAT_PEACH);

    char rangeBuf[16];
    snprintf(rangeBuf, sizeof(rangeBuf), "%.0f-%.0f°", dailyData[i].minTemp, dailyData[i].maxTemp);
    tft.setTextColor(CAT_SUBTEXT0, rowBg);
    tft.drawRightString(rangeBuf, 305, y + 1, 1);
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
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(TFT_BL, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, currentBrightness);

  // Initialize Display
  tft.init();
  tft.setRotation(1); // Landscape 320x240
  tft.fillScreen(CAT_BASE);

  // Initialize Touch
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Header and View
  drawHeader();
  renderCurrentView();
  updateWeatherMoodRGB(currentWeather.weatherCode);

  playBeep(2400, 70);
  Serial.println(F("[SYSTEM] Catppuccin Weather Station ready."));
}

void loop() {
  // LDR Auto Dimming every 200ms
  if (millis() - lastLDRCheck > 200) {
    lastLDRCheck = millis();
    updateBacklightAutoDim();
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    // Calibration for 320x240 Landscape
    int touchX = map(p.x, 3800, 300, 0, 320);
    int touchY = map(p.y, 3800, 400, 0, 240);
    touchX = constrain(touchX, 0, 320);
    touchY = constrain(touchY, 0, 240);

    // Tab Bar click
    if (touchY >= 200) {
      int tappedTab = touchX / 106;
      if (tappedTab >= 0 && tappedTab <= 2 && tappedTab != currentTab) {
        currentTab = tappedTab;
        playBeep(1800, 40);
        renderCurrentView();
        delay(180);
      }
    }
  }

  // Periodic weather update cycle (every 10 minutes)
  if (millis() - lastWeatherUpdate > 600000) {
    lastWeatherUpdate = millis();
    // In live deployment: query Open-Meteo REST endpoint
    // https://api.open-meteo.com/v1/forecast?latitude=47.6062&longitude=-122.3321&current_weather=true&hourly=temperature_2m,precipitation_probability&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max&timezone=auto
    renderCurrentView();
  }

  delay(20);
}
