/**
 * @file 01-weather-station.ino
 * @brief Live WiFi Weather Station & Clock for Cheap Yellow Display (CYD)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Uses Open-Meteo API (100% Free, NO API key required!)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Enter your WiFi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Geolocation (Default: New York City - change to your latitude & longitude)
const float LATITUDE  = 40.7128;
const float LONGITUDE = -74.0060;
const char* CITY_NAME = "New York, USA";
const char* NTP_SERVER = "pool.ntp.org";
const long  GMT_OFFSET_SEC = -18000; // EST (UTC -5)
const int   DAYLIGHT_OFFSET_SEC = 3600;

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

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
SPIClass touchSPI(VSPI);

// Weather Data Struct
struct WeatherData {
  float temperature = 0.0;
  int humidity = 0;
  float windSpeed = 0.0;
  int weatherCode = 0;
  float tempMax = 0.0;
  float tempMin = 0.0;
  String conditionStr = "Sunny";
} weather;

unsigned long lastWeatherUpdate = 0;
const unsigned long WEATHER_UPDATE_INTERVAL = 900000; // 15 minutes

const char* decodeWeatherCode(int code) {
  switch (code) {
    case 0: return "Clear Sky";
    case 1: return "Mainly Clear";
    case 2: return "Partly Cloudy";
    case 3: return "Overcast";
    case 45: case 48: return "Foggy";
    case 51: case 53: case 55: return "Drizzle";
    case 61: case 63: case 65: return "Rain";
    case 71: case 73: case 75: return "Snow";
    case 80: case 81: case 82: return "Showers";
    case 95: case 96: case 99: return "Thunderstorm";
    default: return "Cloudy";
  }
}

void setLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void fetchWeatherData() {
  if (WiFi.status() != WL_CONNECTED) return;

  setLed(false, false, true); // Blue while fetching
  HTTPClient http;
  String url = String("http://api.open-meteo.com/v1/forecast?latitude=") + LATITUDE +
               "&longitude=" + LONGITUDE +
               "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m" +
               "&daily=temperature_2m_max,temperature_2m_min&timezone=auto";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    StaticJsonDocument<1536> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (!err) {
      weather.temperature = doc["current"]["temperature_2m"];
      weather.humidity    = doc["current"]["relative_humidity_2m"];
      weather.windSpeed   = doc["current"]["wind_speed_10m"];
      weather.weatherCode = doc["current"]["weather_code"];
      weather.tempMax     = doc["daily"]["temperature_2m_max"][0];
      weather.tempMin     = doc["daily"]["temperature_2m_min"][0];
      weather.conditionStr = decodeWeatherCode(weather.weatherCode);
      Serial.printf("Weather: %.1f C, %s, Humidity: %d%%\n", 
                    weather.temperature, weather.conditionStr.c_str(), weather.humidity);
    }
  }
  http.end();
  setLed(false, true, false); // Green when done
}

void renderUI() {
  tft.fillScreen(0x0821); // Dark sleek background

  // Header Bar
  tft.fillRect(0, 0, 320, 32, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(8, 8);
  tft.print(CITY_NAME);

  // Time & Date card
  tft.drawRoundRect(8, 40, 148, 84, 6, 0x39C7);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[10];
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    tft.setTextColor(0xFDC0, 0x0821); // CYD Yellow
    tft.setTextSize(3);
    tft.setCursor(24, 52);
    tft.print(timeStr);

    char dateStr[20];
    strftime(dateStr, sizeof(dateStr), "%a, %b %d", &timeinfo);
    tft.setTextColor(TFT_LIGHTGREY, 0x0821);
    tft.setTextSize(1);
    tft.setCursor(26, 96);
    tft.print(dateStr);
  }

  // Current Temperature Big Card
  tft.drawRoundRect(164, 40, 148, 84, 6, 0xFD20); // Orange
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(3);
  tft.setCursor(180, 52);
  tft.printf("%.1f C", weather.temperature);

  tft.setTextColor(TFT_CYAN, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(176, 96);
  tft.print(weather.conditionStr);

  // Details Row Cards
  // Humidity
  tft.drawRoundRect(8, 132, 96, 68, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_LIGHTGREY, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(16, 140);
  tft.print("HUMIDITY");
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(24, 162);
  tft.printf("%d%%", weather.humidity);

  // Wind Speed
  tft.drawRoundRect(112, 132, 96, 68, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_LIGHTGREY, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(120, 140);
  tft.print("WIND");
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(118, 162);
  tft.printf("%.0f km/h", weather.windSpeed);

  // High / Low
  tft.drawRoundRect(216, 132, 96, 68, 4, TFT_DARKGREY);
  tft.setTextColor(TFT_LIGHTGREY, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(226, 140);
  tft.print("H / L TEMP");
  tft.setTextColor(0xF800, 0x0821); // Red High
  tft.setTextSize(1);
  tft.setCursor(224, 160);
  tft.printf("H: %.0f C", weather.tempMax);
  tft.setTextColor(0x07FF, 0x0821); // Cyan Low
  tft.setCursor(224, 178);
  tft.printf("L: %.0f C", weather.tempMin);

  // Footer
  tft.fillRect(0, 222, 320, 18, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(8, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
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

  // Connecting banner
  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(30, 100);
  tft.print("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 25) {
    delay(400);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    fetchWeatherData();
    lastWeatherUpdate = millis();
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

  // Periodic weather refresh
  if (now - lastWeatherUpdate > WEATHER_UPDATE_INTERVAL) {
    lastWeatherUpdate = now;
    fetchWeatherData();
    renderUI();
  }

  // Refresh clock every minute
  static unsigned long lastClockUpdate = 0;
  if (now - lastClockUpdate > 30000) {
    lastClockUpdate = now;
    renderUI();
  }

  delay(200);
}
