/**
 * @file 06-pomodoro-timer.ino
 * @brief Touchscreen Pomodoro Productivity Timer & Clock for CYD 2.8"
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch Cheap Yellow Display)
 * - Onboard Speaker (GPIO 26) for alarm chimes
 * - Onboard RGB LED (GPIO 4, 16, 17) for work/break state indicators
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26
#define LDR_PIN 34
#define TFT_BL 21

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

enum TimerMode {
  MODE_POMODORO = 0, // 25 min
  MODE_SHORT_BREAK = 1, // 5 min
  MODE_LONG_BREAK = 2  // 15 min
};

TimerMode currentMode = MODE_POMODORO;
const int MODE_DURATIONS[] = {25 * 60, 5 * 60, 15 * 60};
int totalSeconds = 25 * 60;
int remainingSeconds = 25 * 60;
bool isRunning = false;
int completedSessions = 0;

unsigned long lastTick = 0;
unsigned long lastAlarmTone = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void playSessionAlarm() {
  for (int i = 0; i < 4; ++i) {
    beep(2200, 120);
    delay(80);
    beep(2800, 120);
    delay(80);
  }
}

void updateLedState() {
  if (!isRunning) {
    setRgb(false, false, true); // Blue when paused
  } else if (currentMode == MODE_POMODORO) {
    setRgb(true, false, false); // Red for focus / work
  } else {
    setRgb(false, true, false); // Green for break
  }
}

void drawTimerDigits() {
  int mins = remainingSeconds / 60;
  int secs = remainingSeconds % 60;

  tft.fillRect(70, 78, 180, 52, 0x0821);
  tft.setTextColor(currentMode == MODE_POMODORO ? 0xF800 : 0x07E0, 0x0821);
  tft.setTextSize(6);
  tft.setCursor(76, 84);
  tft.printf("%02d:%02d", mins, secs);

  // Progress Bar under digits
  int barW = map(totalSeconds - remainingSeconds, 0, totalSeconds, 0, 220);
  tft.fillRect(50, 142, barW, 8, currentMode == MODE_POMODORO ? 0xFDC0 : 0x07FF);
  tft.fillRect(50 + barW, 142, 220 - barW, 8, 0x2104);
  tft.drawRect(50, 142, 220, 8, TFT_DARKGREY);
}

void drawUI() {
  tft.fillScreen(0x0821);

  // Header Bar
  tft.fillRect(0, 0, 320, 32, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.print("CYD 2.8\" Pomodoro");

  // Mode Selection Tabs
  // Tab 1: Focus 25m
  tft.fillRoundRect(10, 38, 96, 28, 4, (currentMode == MODE_POMODORO) ? 0xF800 : 0x2104);
  tft.setTextColor(TFT_WHITE, (currentMode == MODE_POMODORO) ? 0xF800 : 0x2104);
  tft.setTextSize(1);
  tft.setCursor(24, 46);
  tft.print("FOCUS 25m");

  // Tab 2: Short Break 5m
  tft.fillRoundRect(112, 38, 96, 28, 4, (currentMode == MODE_SHORT_BREAK) ? 0x07E0 : 0x2104);
  tft.setTextColor((currentMode == MODE_SHORT_BREAK) ? TFT_BLACK : TFT_WHITE, (currentMode == MODE_SHORT_BREAK) ? 0x07E0 : 0x2104);
  tft.setCursor(120, 46);
  tft.print("SHORT 5m");

  // Tab 3: Long Break 15m
  tft.fillRoundRect(214, 38, 96, 28, 4, (currentMode == MODE_LONG_BREAK) ? 0x07FF : 0x2104);
  tft.setTextColor((currentMode == MODE_LONG_BREAK) ? TFT_BLACK : TFT_WHITE, (currentMode == MODE_LONG_BREAK) ? 0x07FF : 0x2104);
  tft.setCursor(224, 46);
  tft.print("LONG 15m");

  // Center Timer Box Frame
  tft.drawRoundRect(40, 72, 240, 86, 8, 0x39C7);
  drawTimerDigits();

  // Control Buttons
  // START / PAUSE Button
  uint16_t btnColor = isRunning ? 0xFD20 : 0x03E0; // Orange Pause / Green Start
  tft.fillRoundRect(30, 166, 120, 42, 6, btnColor);
  tft.setTextColor(TFT_WHITE, btnColor);
  tft.setTextSize(2);
  tft.setCursor(55, 178);
  tft.print(isRunning ? "PAUSE" : "START");

  // RESET Button
  tft.fillRoundRect(170, 166, 120, 42, 6, 0x2104);
  tft.drawRoundRect(170, 166, 120, 42, 6, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setCursor(200, 178);
  tft.print("RESET");

  // Footer Stats
  tft.fillRect(0, 222, 320, 18, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(8, 226);
  tft.printf("Completed: %d sessions | YT: @IamNotIntoTech | @Makerrdesigns", completedSessions);

  updateLedState();
}

void selectMode(TimerMode mode) {
  currentMode = mode;
  totalSeconds = MODE_DURATIONS[mode];
  remainingSeconds = totalSeconds;
  isRunning = false;
  beep(1800, 40);
  drawUI();
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawUI();
  beep(1400, 100);
}

void loop() {
  unsigned long now = millis();

  // 1-second countdown clock
  if (isRunning && now - lastTick >= 1000) {
    lastTick = now;

    if (remainingSeconds > 0) {
      remainingSeconds--;
      drawTimerDigits();
    } else {
      // Timer finished!
      isRunning = false;
      if (currentMode == MODE_POMODORO) {
        completedSessions++;
      }
      playSessionAlarm();
      drawUI();
    }
  }

  // Touch Interactions
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Mode Tabs (y: 38 to 66)
    if (y >= 38 && y <= 66) {
      if (x >= 10 && x <= 106) {
        selectMode(MODE_POMODORO);
        delay(200);
      } else if (x >= 112 && x <= 208) {
        selectMode(MODE_SHORT_BREAK);
        delay(200);
      } else if (x >= 214 && x <= 310) {
        selectMode(MODE_LONG_BREAK);
        delay(200);
      }
    }
    // START / PAUSE Button (30, 166, 120, 42)
    else if (x >= 30 && x <= 150 && y >= 166 && y <= 208) {
      isRunning = !isRunning;
      beep(isRunning ? 2000 : 1000, 50);
      drawUI();
      delay(250);
    }
    // RESET Button (170, 166, 120, 42)
    else if (x >= 170 && x <= 290 && y >= 166 && y <= 208) {
      isRunning = false;
      remainingSeconds = totalSeconds;
      beep(800, 60);
      drawUI();
      delay(250);
    }
  }

  delay(20);
}
