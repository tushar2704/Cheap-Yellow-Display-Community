/**
 * @file 02-crypto-tracker.ino
 * @brief Live Cryptocurrency Price Ticker & Sparkline Dashboard for CYD
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Fetches real-time crypto prices via CoinGecko Public API (No API key required)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Enter your WiFi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Hardware Pins (ESP32-2432S028R)
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

#define LED_RED 4
#define LED_GREEN 16
#define LED_BLUE 17
#define SPEAKER_PIN 26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
SPIClass touchSPI(VSPI);

struct CryptoCoin {
  const char* id;
  const char* symbol;
  const char* name;
  float price;
  float change24h;
  uint16_t themeColor;
};

CryptoCoin coins[3] = {
  {"bitcoin",  "BTC", "Bitcoin",  0.0, 0.0, 0xFDC0}, // Gold/Yellow
  {"ethereum", "ETH", "Ethereum", 0.0, 0.0, 0x64BD}, // Purple/Blue
  {"solana",   "SOL", "Solana",   0.0, 0.0, 0x07FF}  // Cyan
};
int currentCoinIdx = 0;
unsigned long lastCryptoUpdate = 0;
const unsigned long UPDATE_INTERVAL = 60000; // 60 seconds

void setLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void fetchCryptoPrices() {
  if (WiFi.status() != WL_CONNECTED) return;

  setLed(false, false, true); // Blue while fetching
  HTTPClient http;
  String url = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ethereum,solana&vs_currencies=usd&include_24hr_change=true";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (!err) {
      coins[0].price = doc["bitcoin"]["usd"];
      coins[0].change24h = doc["bitcoin"]["usd_24h_change"];

      coins[1].price = doc["ethereum"]["usd"];
      coins[1].change24h = doc["ethereum"]["usd_24h_change"];

      coins[2].price = doc["solana"]["usd"];
      coins[2].change24h = doc["solana"]["usd_24h_change"];

      Serial.println("Prices updated successfully!");
    }
  }
  http.end();
  setLed(false, true, false); // Green when updated
}

void renderUI() {
  tft.fillScreen(0x0821);

  CryptoCoin c = coins[currentCoinIdx];

  // Header Bar
  tft.fillRect(0, 0, 320, 34, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(8, 9);
  tft.print("CYD Crypto Tracker");

  // Coin Switcher Tabs
  for (int i = 0; i < 3; ++i) {
    int tabX = 8 + (i * 102);
    uint16_t bgColor = (i == currentCoinIdx) ? coins[i].themeColor : 0x2104;
    uint16_t textColor = (i == currentCoinIdx) ? TFT_BLACK : TFT_LIGHTGREY;
    tft.fillRoundRect(tabX, 42, 98, 28, 4, bgColor);
    tft.setTextColor(textColor, bgColor);
    tft.setTextSize(2);
    tft.setCursor(tabX + 26, 48);
    tft.print(coins[i].symbol);
  }

  // Main Price Display Card
  tft.drawRoundRect(8, 78, 304, 88, 6, c.themeColor);
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(20, 88);
  tft.printf("%s (USD)", c.name);

  // Big Price
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(3);
  tft.setCursor(20, 108);
  if (c.price >= 1000.0) {
    tft.printf("$%.2f", c.price);
  } else {
    tft.printf("$%.2f", c.price);
  }

  // 24h Change Pill
  bool isUp = (c.change24h >= 0.0);
  uint16_t changeColor = isUp ? TFT_GREEN : TFT_RED;
  tft.fillRoundRect(20, 140, 110, 18, 3, isUp ? 0x03E0 : 0x8000);
  tft.setTextColor(TFT_WHITE, isUp ? 0x03E0 : 0x8000);
  tft.setTextSize(1);
  tft.setCursor(26, 145);
  tft.printf("%s %.2f%% (24h)", isUp ? "▲ +" : "▼ ", c.change24h);

  // Mini decorative trend sparkline
  tft.drawRoundRect(8, 174, 304, 42, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_LIGHTGREY, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(16, 180);
  tft.print("TAP SCREEN TO CYCLE COIN OR REFRESH");

  // Sparkline wave animation
  for (int x = 20; x < 300; x += 10) {
    int y1 = 200 + sin((x + currentCoinIdx * 30) * 0.1) * 8;
    int y2 = 200 + sin((x + 10 + currentCoinIdx * 30) * 0.1) * 8;
    tft.drawLine(x, y1, x + 10, y2, changeColor);
  }

  // Footer Bar
  tft.fillRect(0, 222, 320, 18, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("YT: @IamNotIntoTech  |  3D: @Makerrdesigns");
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setLed(true, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

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
    fetchCryptoPrices();
    lastCryptoUpdate = millis();
    renderUI();
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(20, 100);
    tft.print("WiFi Failed! Check credentials.");
  }
}

void loop() {
  unsigned long now = millis();

  // Periodic price refresh
  if (now - lastCryptoUpdate > UPDATE_INTERVAL) {
    lastCryptoUpdate = now;
    fetchCryptoPrices();
    renderUI();
  }

  // Touch screen to cycle coins
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // If tapped tab bar
    if (y >= 40 && y <= 72) {
      if (x < 110) currentCoinIdx = 0;
      else if (x < 210) currentCoinIdx = 1;
      else currentCoinIdx = 2;
    } else {
      // Tap anywhere else to cycle
      currentCoinIdx = (currentCoinIdx + 1) % 3;
    }

    beep(1800, 30);
    renderUI();
    delay(250);
  }

  delay(50);
}
