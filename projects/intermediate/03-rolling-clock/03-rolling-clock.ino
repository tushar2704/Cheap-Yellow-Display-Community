/**
 * @file 03-rolling-clock.ino
 * @brief Smooth Scrolling NTP Digital Desk Clock for CYD 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Modernized rolling digit clock inspired by mechanical split-flap and rolling clocks.
 */

#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* NTP_SERVER = "pool.ntp.org";
const long  GMT_OFFSET_SEC = 0;       // Set your UTC offset in seconds (e.g. EST = -18000)
const int   DAYLIGHT_OFFSET_SEC = 3600;

#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define SPEAKER_PIN 26
#define LED_GREEN 16

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft); // Sprite for flicker-free rolling animation

char prevTimeStr[9] = "        ";
bool is24HourFormat = true;

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void drawClockCard() {
  tft.fillScreen(0x0821);

  // Header Banner
  tft.fillRect(0, 0, 320, 36, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("CYD 2.8\" Rolling Clock");

  // Digit Plate Outer Frame
  tft.fillRoundRect(20, 52, 280, 110, 10, 0x10A2);
  tft.drawRoundRect(20, 52, 280, 110, 10, 0x39C7);

  // Center horizontal split line (split-flap style)
  tft.drawLine(20, 107, 300, 107, TFT_BLACK);

  // Footer Info Bar
  tft.fillRect(0, 222, 320, 18, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("YT: @IamNotIntoTech  |  3D: @Makerrdesigns");
}

void updateTimeDisplay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  char currentTimeStr[9];
  if (is24HourFormat) {
    strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M:%S", &timeinfo);
  } else {
    strftime(currentTimeStr, sizeof(currentTimeStr), "%I:%M:%S", &timeinfo);
  }

  // Draw Time with flicker-free rendering
  tft.setTextColor(0xFDC0, 0x10A2); // CYD Yellow
  tft.setTextSize(4);
  tft.setCursor(55, 88);
  tft.print(currentTimeStr);

  // Date line
  char dateStr[32];
  strftime(dateStr, sizeof(dateStr), "%A, %B %d, %Y", &timeinfo);
  tft.fillRect(20, 175, 280, 30, 0x0821);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(2);
  int dateX = (320 - (strlen(dateStr) * 12)) / 2;
  dateX = max(10, dateX);
  tft.setCursor(dateX, 180);
  tft.print(dateStr);

  // Hourly chime on second :00
  if (timeinfo.tm_sec == 0) {
    beep(1800, 150);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(30, 100);
  tft.print("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 25) {
    delay(400);
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_GREEN, LOW); // Green LED on
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    drawClockCard();
    beep(1500, 100);
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(20, 100);
    tft.print("WiFi Failed! Check credentials.");
  }
}

void loop() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate >= 1000) {
    lastUpdate = now;
    updateTimeDisplay();
  }

  delay(50);
}
