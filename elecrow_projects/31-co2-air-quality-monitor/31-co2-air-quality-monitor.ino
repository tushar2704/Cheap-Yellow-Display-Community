/**
 * 💨 Project 31: CO2 Indoor Air Quality Monitor
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Sensirion SCD40/41 NDIR photoacoustic CO2, temperature & humidity sensor support.
 * - Hardware I2C on HY2.0 port (SDA: GPIO 27, SCL: GPIO 22) with automatic simulation fallback.
 * - 48-point historical trend chart with color-coded safety thresholds (800 / 1200 / 1600 ppm).
 * - Multi-stage environmental alert engine with acoustic buzzer & active-low RGB status LED.
 * - Calibrated non-inverted resistive touch controls with mute toggle, unit switch (°C/°F), and manual refresh.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
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

// HY2.0 I2C Bus Pins
#define I2C_SDA       27
#define I2C_SCL       22
#define SCD4X_ADDR    0x62

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

// UI Colors (16-bit 565)
#define COLOR_BG        0x0842 // Very dark charcoal
#define COLOR_CARD      0x18E5 // Dark slate card
#define COLOR_CARD_BRD  0x2967
#define COLOR_TEXT      0xFFFF
#define COLOR_MUTED     0x9CF3
#define COLOR_GOOD      0x07E0 // Bright Green
#define COLOR_MODERATE  0xFFE0 // Bright Yellow
#define COLOR_WARNING   0xFD20 // Orange
#define COLOR_DANGER    0xF800 // Bright Red
#define COLOR_CYAN      0x07FF

// Telemetry State
uint16_t currentCO2 = 645;
float currentTempC = 22.4;
float currentHumidity = 48.2;
bool sensorDetected = false;
bool alarmSilenced = false;
bool tempUnitF = false;

// Trend History (48 samples)
#define HISTORY_LEN 48
uint16_t co2History[HISTORY_LEN];
int historyIdx = 0;

unsigned long lastSensorRead = 0;
unsigned long lastAlarmBlink = 0;
unsigned long lastTouchTime = 0;
bool alarmBlinkState = false;

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

// CRC8 calculation for Sensirion SCD4x
uint8_t sensirionCRC(const uint8_t* data, uint16_t count) {
  uint8_t crc = 0xFF;
  for (uint16_t i = 0; i < count; i++) {
    crc ^= data[i];
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (crc & 0x80) crc = (crc << 1) ^ 0x31;
      else crc = (crc << 1);
    }
  }
  return crc;
}

bool initSCD4x() {
  Wire.begin(I2C_SDA, I2C_SCL, 100000);
  delay(100);

  // Check if SCD40/41 responds at 0x62
  Wire.beginTransmission(SCD4X_ADDR);
  if (Wire.endTransmission() == 0) {
    // Start periodic measurement command: 0x21B1
    Wire.beginTransmission(SCD4X_ADDR);
    Wire.write(0x21);
    Wire.write(0xB1);
    Wire.endTransmission();
    return true;
  }
  return false;
}

bool readSCD4x(uint16_t &co2, float &temp, float &rh) {
  Wire.beginTransmission(SCD4X_ADDR);
  Wire.write(0xEC);
  Wire.write(0x05);
  if (Wire.endTransmission() != 0) return false;

  delay(5);
  int bytesRead = Wire.requestFrom(SCD4X_ADDR, 9);
  if (bytesRead == 9) {
    uint8_t data[9];
    for (int i = 0; i < 9; i++) data[i] = Wire.read();

    if (sensirionCRC(data, 2) == data[2] &&
        sensirionCRC(&data[3], 2) == data[5] &&
        sensirionCRC(&data[6], 2) == data[8]) {
      co2 = (data[0] << 8) | data[1];
      uint16_t rawTemp = (data[3] << 8) | data[4];
      uint16_t rawRh = (data[6] << 8) | data[7];
      temp = -45.0 + 175.0 * ((float)rawTemp / 65535.0);
      rh = 100.0 * ((float)rawRh / 65535.0);
      return true;
    }
  }
  return false;
}

uint16_t getAirQualityColor(uint16_t co2) {
  if (co2 < 800) return COLOR_GOOD;
  if (co2 < 1200) return COLOR_MODERATE;
  if (co2 < 1600) return COLOR_WARNING;
  return COLOR_DANGER;
}

const char* getAirQualityStatus(uint16_t co2) {
  if (co2 < 800) return "EXCELLENT AIR";
  if (co2 < 1200) return "MODERATE AIR";
  if (co2 < 1600) return "POOR VENTILATION";
  return "VENTILATE ROOM NOW!";
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x0000);
  tft.drawFastHLine(0, 26, 320, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_CYAN, 0x0000);
  tft.drawString("IamNotIntoTech", 8, 6, 2);

  tft.setTextColor(COLOR_TEXT, 0x0000);
  tft.drawCentreString("CO2 AIR QUALITY", 175, 6, 2);

  // Sensor Mode Tag
  if (sensorDetected) {
    tft.fillRoundRect(248, 4, 66, 18, 3, tft.color565(10, 50, 15));
    tft.drawRoundRect(248, 4, 66, 18, 3, COLOR_GOOD);
    tft.setTextColor(COLOR_GOOD, tft.color565(10, 50, 15));
    tft.drawCentreString("SCD4x", 281, 6, 1);
  } else {
    tft.fillRoundRect(248, 4, 66, 18, 3, tft.color565(40, 25, 10));
    tft.drawRoundRect(248, 4, 66, 18, 3, COLOR_WARNING);
    tft.setTextColor(COLOR_WARNING, tft.color565(40, 25, 10));
    tft.drawCentreString("SIMULATED", 281, 6, 1);
  }
}

void drawHeroCard() {
  uint16_t qColor = getAirQualityColor(currentCO2);

  tft.fillRoundRect(10, 32, 195, 100, 6, COLOR_CARD);
  tft.drawRoundRect(10, 32, 195, 100, 6, qColor);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("CARBON DIOXIDE", 18, 40, 1);

  // Large PPM Text
  tft.setTextColor(qColor, COLOR_CARD);
  char co2Str[16];
  snprintf(co2Str, sizeof(co2Str), "%d", currentCO2);
  tft.drawString(co2Str, 18, 52, 6);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("ppm", 145, 75, 2);

  // Status Banner
  tft.fillRoundRect(16, 96, 183, 26, 4, tft.color565(20, 25, 30));
  tft.setTextColor(qColor, tft.color565(20, 25, 30));
  tft.drawCentreString(getAirQualityStatus(currentCO2), 107, 102, 1);
}

void drawEnvironmentalCards() {
  // Temperature Card
  tft.fillRoundRect(212, 32, 98, 48, 6, COLOR_CARD);
  tft.drawRoundRect(212, 32, 98, 48, 6, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("TEMPERATURE", 218, 38, 1);

  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  char tStr[16];
  if (tempUnitF) {
    snprintf(tStr, sizeof(tStr), "%.1f F", currentTempC * 1.8 + 32.0);
  } else {
    snprintf(tStr, sizeof(tStr), "%.1f C", currentTempC);
  }
  tft.drawString(tStr, 218, 52, 2);

  // Humidity Card
  tft.fillRoundRect(212, 84, 98, 48, 6, COLOR_CARD);
  tft.drawRoundRect(212, 84, 98, 48, 6, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("HUMIDITY", 218, 90, 1);

  tft.setTextColor(COLOR_CYAN, COLOR_CARD);
  char rhStr[16];
  snprintf(rhStr, sizeof(rhStr), "%.1f %%", currentHumidity);
  tft.drawString(rhStr, 218, 104, 2);
}

void drawTrendChart() {
  tft.fillRoundRect(10, 138, 300, 66, 6, COLOR_CARD);
  tft.drawRoundRect(10, 138, 300, 66, 6, COLOR_CARD_BRD);

  tft.setTextColor(COLOR_MUTED, COLOR_CARD);
  tft.drawString("HISTORICAL PROFILE (48 SAMPLES)", 18, 143, 1);

  int graphX = 18;
  int graphY = 156;
  int graphW = 284;
  int graphH = 42;

  // Threshold lines
  uint16_t minPPM = 400;
  uint16_t maxPPM = 2000;

  int y800 = graphY + graphH - map(800, minPPM, maxPPM, 0, graphH);
  int y1200 = graphY + graphH - map(1200, minPPM, maxPPM, 0, graphH);
  tft.drawFastHLine(graphX, y800, graphW, tft.color565(30, 60, 30));
  tft.drawFastHLine(graphX, y1200, graphW, tft.color565(60, 50, 20));

  int stepX = graphW / HISTORY_LEN;
  for (int i = 0; i < HISTORY_LEN; i++) {
    int idx = (historyIdx + i) % HISTORY_LEN;
    uint16_t val = co2History[idx];
    if (val == 0) val = 600;

    int py = graphY + graphH - map(constrain(val, minPPM, maxPPM), minPPM, maxPPM, 0, graphH);
    int px = graphX + i * stepX;

    uint16_t ptColor = getAirQualityColor(val);
    tft.drawFastVLine(px, py, (graphY + graphH) - py, tft.color565(20, 35, 45));
    tft.drawPixel(px, py, ptColor);

    if (i > 0) {
      int prevIdx = (historyIdx + i - 1) % HISTORY_LEN;
      uint16_t prevVal = co2History[prevIdx];
      if (prevVal == 0) prevVal = 600;
      int prevY = graphY + graphH - map(constrain(prevVal, minPPM, maxPPM), minPPM, maxPPM, 0, graphH);
      int prevX = graphX + (i - 1) * stepX;
      tft.drawLine(prevX, prevY, px, py, ptColor);
    }
  }
}

void drawControlBar() {
  tft.fillRect(0, 208, 320, 32, 0x0000);
  tft.drawFastHLine(0, 208, 320, COLOR_CARD_BRD);

  // Button 1: Unit Toggle
  tft.fillRoundRect(10, 212, 90, 24, 4, COLOR_CARD);
  tft.drawRoundRect(10, 212, 90, 24, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_TEXT, COLOR_CARD);
  tft.drawCentreString(tempUnitF ? "UNIT: F" : "UNIT: C", 55, 216, 1);

  // Button 2: Alarm Mute
  uint16_t muteBg = alarmSilenced ? tft.color565(60, 20, 20) : COLOR_CARD;
  uint16_t muteFg = alarmSilenced ? COLOR_DANGER : COLOR_TEXT;
  tft.fillRoundRect(115, 212, 90, 24, 4, muteBg);
  tft.drawRoundRect(115, 212, 90, 24, 4, alarmSilenced ? COLOR_DANGER : COLOR_CARD_BRD);
  tft.setTextColor(muteFg, muteBg);
  tft.drawCentreString(alarmSilenced ? "ALARM: MUTED" : "ALARM: ON", 160, 216, 1);

  // Button 3: Refresh
  tft.fillRoundRect(220, 212, 90, 24, 4, COLOR_CARD);
  tft.drawRoundRect(220, 212, 90, 24, 4, COLOR_CARD_BRD);
  tft.setTextColor(COLOR_CYAN, COLOR_CARD);
  tft.drawCentreString("REFRESH", 265, 216, 1);
}

void renderAll() {
  drawHeader();
  drawHeroCard();
  drawEnvironmentalCards();
  drawTrendChart();
  drawControlBar();
}

void updateLEDAndAlarm() {
  if (currentCO2 < 800) {
    setRGB(false, true, false); // Green
  } else if (currentCO2 < 1200) {
    setRGB(true, true, false); // Yellow
  } else if (currentCO2 < 1600) {
    setRGB(true, false, false); // Red
  } else {
    // Critical Alarm state: flash Red
    if (millis() - lastAlarmBlink > 400) {
      lastAlarmBlink = millis();
      alarmBlinkState = !alarmBlinkState;
      setRGB(alarmBlinkState, false, false);
      if (alarmBlinkState && !alarmSilenced) {
        playBeep(2600, 60);
      }
    }
  }
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

  // Attempt SCD4x I2C Init
  sensorDetected = initSCD4x();

  for (int i = 0; i < HISTORY_LEN; i++) {
    co2History[i] = 580 + random(0, 150) + (i > 30 ? (i - 30) * 15 : 0);
  }

  renderAll();
  updateLEDAndAlarm();

  playBeep(2200, 80);
  Serial.println(F("[SYSTEM] CO2 Air Quality Monitor Ready."));
}

void loop() {
  // Update sensor data every 5 seconds
  if (millis() - lastSensorRead > 5000) {
    lastSensorRead = millis();

    if (sensorDetected) {
      uint16_t newCO2;
      float newTemp, newRh;
      if (readSCD4x(newCO2, newTemp, newRh)) {
        currentCO2 = newCO2;
        currentTempC = newTemp;
        currentHumidity = newRh;
      }
    } else {
      int delta = random(-20, 25);
      currentCO2 = constrain((int)currentCO2 + delta, 420, 1850);
      currentTempC += (random(-5, 6) / 20.0);
      currentHumidity += (random(-5, 6) / 15.0);
    }

    co2History[historyIdx] = currentCO2;
    historyIdx = (historyIdx + 1) % HISTORY_LEN;

    drawHeroCard();
    drawEnvironmentalCards();
    drawTrendChart();
  }

  updateLEDAndAlarm();

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        if (screenY >= 205) {
          if (screenX >= 10 && screenX <= 100) {
            tempUnitF = !tempUnitF;
            playBeep(1600, 30);
            drawEnvironmentalCards();
            drawControlBar();
          } else if (screenX >= 115 && screenX <= 205) {
            alarmSilenced = !alarmSilenced;
            playBeep(1800, 30);
            drawControlBar();
          } else if (screenX >= 220 && screenX <= 310) {
            playBeep(2400, 40);
            lastSensorRead = 0;
          }
        }
      }
    }
  }

  delay(20);
}
