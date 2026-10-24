/**
 * @file 07-retro-matrix.ino
 * @brief Cyberpunk Matrix Digital Rain Screensaver with LDR Auto-Dimming for CYD 2.8"
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch Cheap Yellow Display)
 * - Onboard Ambient Light Sensor (LDR: GPIO 34) for automatic backlight dimming
 * - Onboard Speaker (GPIO 26) for cybernetic audio chirps
 * - Onboard RGB LED (GPIO 4: Red, 16: Green, 17: Blue)
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// CYD 2.8" Touchscreen Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

// Hardware Indicators & Sensors
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

// Matrix Grid Configuration (320x240 landscape)
// Font 2 is ~14 pixels wide and 16 pixels tall
const int CHAR_WIDTH = 10;
const int CHAR_HEIGHT = 15;
const int COLS = 320 / CHAR_WIDTH;   // 32 columns
const int ROWS = 240 / CHAR_HEIGHT;  // 16 rows

int streamY[COLS];     // Current row position of each falling stream
int streamSpeed[COLS]; // Falling speed multiplier per column
int streamLen[COLS];   // Tail length per column

unsigned long lastRainFrame = 0;
unsigned long lastLdrCheck = 0;
int currentBrightness = 220;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void setBacklight(int duty) {
  currentBrightness = constrain(duty, 25, 255);
  ledcWrite(3, currentBrightness);
}

char getRandomMatrixChar() {
  // Random characters: numbers, uppercase letters, symbols
  int r = random(0, 36);
  if (r < 10) return '0' + r;
  return 'A' + (r - 10);
}

void initMatrixStreams() {
  for (int c = 0; c < COLS; ++c) {
    streamY[c] = random(-ROWS, 0);
    streamSpeed[c] = random(1, 3);
    streamLen[c] = random(5, 12);
  }
}

void updateAmbientLighting() {
  int rawLdr = analogRead(LDR_PIN); // 0 (pitch dark) to 4095 (direct light)
  // Map ambient light to screen brightness (smooth auto-dim)
  int targetBrightness = map(rawLdr, 200, 3500, 35, 255);
  targetBrightness = constrain(targetBrightness, 30, 255);

  // Smooth filter
  currentBrightness = (currentBrightness * 3 + targetBrightness) / 4;
  setBacklight(currentBrightness);
}

void triggerTouchRipple(int touchCol) {
  beep(1800 + random(0, 600), 25);
  setRgb(false, true, false); // Flash Green LED

  // Reset surrounding streams to top with high speed
  for (int i = max(0, touchCol - 3); i <= min(COLS - 1, touchCol + 3); ++i) {
    streamY[i] = 0;
    streamSpeed[i] = 3;
    streamLen[i] = 14;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, true, false);

  // Setup Backlight PWM on Channel 3
  ledcSetup(3, 5000, 8);
  ledcAttachPin(TFT_BL, 3);
  setBacklight(200);

  tft.init();
  tft.setRotation(1); // Landscape 320x240
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Splash Screen
  tft.setTextColor(0x07E0, TFT_BLACK); // Neon Matrix Green
  tft.setTextSize(2);
  tft.setCursor(30, 90);
  tft.print("INITIALIZING CYD RAIN...");
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(30, 130);
  tft.print("Cheap Yellow Display Community | @IamNotIntoTech");

  beep(1200, 100);
  delay(1200);
  tft.fillScreen(TFT_BLACK);

  initMatrixStreams();
  beep(2400, 60);
}

void loop() {
  unsigned long now = millis();

  // Auto-dim backlight based on LDR sensor every 500ms
  if (now - lastLdrCheck > 500) {
    lastLdrCheck = now;
    updateAmbientLighting();
  }

  // Interactive Touch Screen Input
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    x = constrain(x, 0, 319);
    int touchCol = x / CHAR_WIDTH;
    triggerTouchRipple(touchCol);
    delay(40);
  } else {
    setRgb(false, false, false);
  }

  // Render Matrix Falling Rain Frame (~30 FPS)
  if (now - lastRainFrame > 35) {
    lastRainFrame = now;

    for (int c = 0; c < COLS; ++c) {
      int headRow = streamY[c];

      // Draw the bright white/cyan leading head glyph
      if (headRow >= 0 && headRow < ROWS) {
        int x = c * CHAR_WIDTH;
        int y = headRow * CHAR_HEIGHT;
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawChar(x, y, getRandomMatrixChar(), TFT_WHITE, TFT_BLACK, 1);
      }

      // Draw glowing green body glyphs trailing behind the head
      int bodyRow = headRow - 1;
      if (bodyRow >= 0 && bodyRow < ROWS) {
        int x = c * CHAR_WIDTH;
        int y = bodyRow * CHAR_HEIGHT;
        tft.setTextColor(0x07E0, TFT_BLACK); // Vibrant Bright Green
        tft.drawChar(x, y, getRandomMatrixChar(), 0x07E0, TFT_BLACK, 1);
      }

      // Draw darker dim green trailing glyph
      int tailRow = headRow - 3;
      if (tailRow >= 0 && tailRow < ROWS) {
        int x = c * CHAR_WIDTH;
        int y = tailRow * CHAR_HEIGHT;
        tft.setTextColor(0x03E0, TFT_BLACK); // Darker Green
        tft.drawChar(x, y, getRandomMatrixChar(), 0x03E0, TFT_BLACK, 1);
      }

      // Erase character behind the tail
      int eraseRow = headRow - streamLen[c];
      if (eraseRow >= 0 && eraseRow < ROWS) {
        int x = c * CHAR_WIDTH;
        int y = eraseRow * CHAR_HEIGHT;
        tft.fillRect(x, y, CHAR_WIDTH, CHAR_HEIGHT, TFT_BLACK);
      }

      // Advance column position
      streamY[c] += 1;

      // When stream completely passes below the screen, reset to top
      if (streamY[c] - streamLen[c] > ROWS) {
        streamY[c] = random(-8, 0);
        streamLen[c] = random(5, 14);
        streamSpeed[c] = random(1, 3);
      }
    }
  }
}
