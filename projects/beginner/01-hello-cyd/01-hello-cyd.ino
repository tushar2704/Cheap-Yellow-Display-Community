/**
 * @file 01-hello-cyd.ino
 * @brief "Hello CYD" Complete All-in-One Hardware Test & Starter Sketch
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Watch the full video tutorial on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware Supported:
 * - ESP32-2432S028R (2.8" Resistive Touch CYD - Default)
 * 
 * Features:
 * 1. TFT Display Graphics & Color Bar demonstration
 * 2. Real-time XPT2046 Touch Screen tracking with crosshairs & coordinates
 * 3. Onboard RGB LED cycling animation (GPIO 4, 16, 17)
 * 4. Ambient Light Sensor (LDR) real-time gauge (GPIO 34)
 * 5. Touch acoustic feedback beep via onboard speaker (GPIO 26)
 * 6. MicroSD card detection test (GPIO 5)
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SD.h>

// Onboard Hardware Pins for ESP32-2432S028R
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define SD_CS 5
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23

#define LED_RED 4     // Active LOW
#define LED_GREEN 16  // Active LOW
#define LED_BLUE 17   // Active LOW
#define LDR_PIN 34    // Analog light sensor
#define SPEAKER_PIN 26 // Audio DAC/Tone

// Touch SPI Bus & Instances
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
SPIClass sdSPI(HSPI);

// Calibration constants for 2.8" Resistive Touch (Landscape)
#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

bool sdCardDetected = false;
unsigned long lastSensorUpdate = 0;
int ledCycle = 0;

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("============================================");
  Serial.println("🟡 Cheap Yellow Display Community — Hello CYD");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("YouTube: @IamNotIntoTech | @Makerrdesigns");
  Serial.println("============================================");

  // Initialize LEDs
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false); // Red during boot

  // Initialize Speaker
  pinMode(SPEAKER_PIN, OUTPUT);
  playBeep(1200, 100);

  // Initialize Display
  tft.init();
  tft.setRotation(1); // Landscape (320x240)
  tft.fillScreen(TFT_BLACK);

  // Initialize Touchscreen SPI
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Initialize MicroSD Card test
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (SD.begin(SD_CS, sdSPI)) {
    sdCardDetected = true;
    Serial.println("✅ MicroSD Card detected successfully!");
  } else {
    sdCardDetected = false;
    Serial.println("ℹ️ No MicroSD Card detected (normal if empty).");
  }

  // Draw Initial UI Layout
  drawStaticUI();
  setRgbLed(false, true, false); // Green when ready
}

void drawStaticUI() {
  tft.fillScreen(0x0821); // Dark background

  // Header Banner
  tft.fillRect(0, 0, 320, 32, 0xFDC0); // Vibrant CYD Yellow
  tft.setTextColor(TFT_BLACK, 0xFDC0);
  tft.setTextSize(2);
  tft.setCursor(8, 8);
  tft.println("CYD COMMUNITY - HELLO!");

  // Channel Credits bar
  tft.fillRect(0, 222, 320, 18, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK); // Green text
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.println("YT: @IamNotIntoTech  |  3D: @Makerrdesigns");

  // Status Cards
  tft.drawRoundRect(8, 40, 148, 64, 4, TFT_LIGHTGREY);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(16, 46);
  tft.println("TOUCH INTERACTION");

  tft.drawRoundRect(164, 40, 148, 64, 4, TFT_LIGHTGREY);
  tft.setCursor(172, 46);
  tft.println("HARDWARE STATUS");

  // SD Status text inside card
  tft.setCursor(172, 62);
  tft.print("SD Card: ");
  if (sdCardDetected) {
    tft.setTextColor(TFT_GREEN, 0x0821);
    tft.println("MOUNTED");
  } else {
    tft.setTextColor(TFT_RED, 0x0821);
    tft.println("NONE");
  }

  // LDR Ambient Light Bar Frame
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(172, 78);
  tft.print("Light: ");
  tft.drawRect(216, 76, 88, 12, TFT_WHITE);

  // Interactive Touch Pad Canvas
  tft.drawRoundRect(8, 112, 304, 102, 6, 0xFD20); // Orange border
  tft.setTextColor(TFT_DARKGREY, 0x0821);
  tft.setCursor(70, 155);
  tft.setTextSize(1);
  tft.println(">>> TAP ANYWHERE ON SCREEN <<<");
}

void loop() {
  unsigned long now = millis();

  // Read Touchscreen
  if (ts.touched()) {
    TS_Point p = ts.getPoint();

    // Map raw ADC touch values to screen coordinates
    int touchX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int touchY = map(p.y, TS_MINY, TS_MAXY, 0, 240);
    touchX = constrain(touchX, 0, 319);
    touchY = constrain(touchY, 0, 239);

    // Update coordinates readout
    tft.fillRect(16, 60, 132, 36, 0x0821);
    tft.setTextColor(TFT_YELLOW, 0x0821);
    tft.setTextSize(1);
    tft.setCursor(16, 62);
    tft.printf("X: %3d  Y: %3d", touchX, touchY);
    tft.setCursor(16, 78);
    tft.printf("Raw Z: %4d", p.z);

    // Draw touch crosshair in interactive zone
    if (touchY > 115 && touchY < 210 && touchX > 12 && touchX < 308) {
      tft.fillCircle(touchX, touchY, 5, TFT_CYAN);
    }

    // Audible touch tone
    playBeep(2400, 20);

    // Flash Blue LED on touch
    setRgbLed(false, false, true);
  }

  // Update Ambient Light Sensor (every 250ms)
  if (now - lastSensorUpdate > 250) {
    lastSensorUpdate = now;

    int lightVal = analogRead(LDR_PIN); // 0 (dark) to 4095 (bright)
    int barWidth = map(lightVal, 0, 4095, 0, 84);
    barWidth = constrain(barWidth, 0, 84);

    tft.fillRect(218, 78, barWidth, 8, TFT_ORANGE);
    tft.fillRect(218 + barWidth, 78, 84 - barWidth, 8, 0x0821);

    // Subtle RGB LED idle pulse
    ledCycle = (ledCycle + 1) % 3;
    if (!ts.touched()) {
      if (ledCycle == 0) setRgbLed(true, false, false);
      else if (ledCycle == 1) setRgbLed(false, true, false);
      else setRgbLed(false, false, true);
    }
  }

  delay(20);
}
