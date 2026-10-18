/**
 * @file 02-wifi-scanner.ino
 * @brief Touchscreen WiFi Spectrum & Network Analyzer for Cheap Yellow Display (CYD)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 */

#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Touch Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

// Hardware Indicators
#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26

// Calibration constants
#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

int networkCount = 0;
bool isScanning = false;

void setLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

const char* getEncryptionName(wifi_auth_mode_t auth) {
  switch (auth) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "ENT";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    default: return "AUTO";
  }
}

uint16_t getRssiColor(int rssi) {
  if (rssi >= -60) return TFT_GREEN;
  if (rssi >= -75) return TFT_YELLOW;
  return TFT_RED;
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 34, 0x18C3); // Deep Navy
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(8, 9);
  tft.print("CYD WiFi Analyzer");

  // Re-scan Touch Button
  tft.fillRoundRect(225, 4, 90, 26, 4, 0xFDC0); // CYD Yellow
  tft.setTextColor(TFT_BLACK, 0xFDC0);
  tft.setTextSize(1);
  tft.setCursor(240, 13);
  tft.print("SCAN NOW");
}

void performScan() {
  isScanning = true;
  setLed(true, true, false); // Yellow LED while scanning
  beep(1000, 80);

  // Status banner
  tft.fillRect(0, 36, 320, 20, 0x0821);
  tft.setTextColor(TFT_YELLOW, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(8, 42);
  tft.print("Scanning 2.4 GHz airwaves... please wait");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  networkCount = WiFi.scanNetworks();

  // Clear list area
  tft.fillRect(0, 36, 320, 186, 0x0821);

  // Status text
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(8, 42);
  tft.printf("Found %d networks nearby:", networkCount);

  // Column headers
  tft.setTextColor(TFT_CYAN, 0x0821);
  tft.setCursor(8, 56);
  tft.print("SSID");
  tft.setCursor(180, 56);
  tft.print("CH");
  tft.setCursor(212, 56);
  tft.print("AUTH");
  tft.setCursor(260, 56);
  tft.print("SIGNAL");
  tft.drawLine(6, 68, 314, 68, TFT_DARKGREY);

  // Display top 6 networks
  int displayLimit = min(networkCount, 6);
  for (int i = 0; i < displayLimit; ++i) {
    int y = 74 + (i * 24);
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) ssid = "<Hidden>";
    if (ssid.length() > 20) ssid = ssid.substring(0, 18) + "..";

    int rssi = WiFi.RSSI(i);
    int channel = WiFi.channel(i);
    const char* enc = getEncryptionName(WiFi.encryptionType(i));
    uint16_t sigColor = getRssiColor(rssi);

    // SSID
    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.setCursor(8, y + 4);
    tft.print(ssid);

    // Channel
    tft.setTextColor(TFT_LIGHTGREY, 0x0821);
    tft.setCursor(182, y + 4);
    tft.print(channel);

    // Encryption
    tft.setCursor(212, y + 4);
    tft.print(enc);

    // Signal strength bar graph
    int barWidth = map(constrain(rssi, -100, -40), -100, -40, 2, 45);
    tft.fillRect(260, y + 5, barWidth, 10, sigColor);
    tft.drawRect(260, y + 5, 45, 10, TFT_DARKGREY);
  }

  // Footer bar
  tft.fillRect(0, 224, 320, 16, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setCursor(6, 228);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");

  setLed(false, true, false); // Green when ready
  beep(1800, 100);
  isScanning = false;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setLed(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawHeader();
  performScan();
}

void loop() {
  if (ts.touched() && !isScanning) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Check if "SCAN NOW" button tapped
    if (x >= 220 && x <= 318 && y >= 0 && y <= 36) {
      beep(2000, 50);
      performScan();
    }
  }
  delay(50);
}
