/**
 * @file 04-zigbee-mqtt-commander.ino
 * @brief Zigbee2MQTT & Generic MQTT Touch Commander for Cheap Yellow Display 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - ESP32-2432S028R (2.8" Resistive Touch CYD)
 * - Onboard RGB LED (GPIO 4, 16, 17)
 * - Onboard Speaker (GPIO 26)
 * - Onboard LDR Sensor (GPIO 34)
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Enter your WiFi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// MQTT Broker Configuration
const char* MQTT_SERVER   = "192.168.1.100";
const int   MQTT_PORT     = 1883;
const char* MQTT_USER     = "";
const char* MQTT_PASS     = "";
const char* CLIENT_ID     = "CYD-Zigbee2MQTT-Panel";

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
#define LDR_PIN 34
#define TFT_BL 21

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Zigbee Device States
struct ZigbeeDevice {
  const char* friendlyName;
  const char* type;      // "switch", "light", "plug", "relay"
  const char* icon;
  bool state;
  const char* topicSet;
  const char* topicState;
  uint16_t color;
};

ZigbeeDevice devices[4] = {
  {"Living Switch", "switch", "S1", true,  "zigbee2mqtt/living_switch/set", "zigbee2mqtt/living_switch", 0x07E0}, // Emerald
  {"Kitchen Light", "light",  "L1", false, "zigbee2mqtt/kitchen_light/set", "zigbee2mqtt/kitchen_light", 0xFDC0}, // Amber Gold
  {"Power Plug",    "plug",   "P1", true,  "zigbee2mqtt/power_plug/set",    "zigbee2mqtt/power_plug",    0x07FF}, // Cyan
  {"Garden Relay",  "relay",  "R1", false, "zigbee2mqtt/garden_relay/set",  "zigbee2mqtt/garden_relay",  0xF81F}  // Magenta
};

// Telemetry Sensors
float liveWatts = 428.5;
float liveTemp  = 23.8;
int   liveLQI   = 142; // Link Quality Indicator (0-255)
unsigned long lastMqttRetry = 0;
unsigned long lastTelemetryUpdate = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 32, 0x18E3);
  tft.setTextColor(0x07E0, 0x18E3);
  tft.setTextSize(1);
  tft.setCursor(10, 11);
  tft.print("⚡ Zigbee2MQTT Commander");

  // Broker Status Badge
  bool connected = mqttClient.connected();
  tft.setTextColor(connected ? TFT_GREEN : TFT_RED, 0x18E3);
  tft.setCursor(205, 11);
  tft.print(connected ? "BROKER: ON" : "BROKER: ERR");
}

void drawDeviceTile(int idx, int x, int y, int w, int h) {
  ZigbeeDevice dev = devices[idx];
  uint16_t bg = dev.state ? dev.color : 0x10A2;
  uint16_t textCol = dev.state ? TFT_BLACK : TFT_WHITE;
  uint16_t borderCol = dev.state ? TFT_WHITE : 0x3186;

  tft.fillRoundRect(x, y, w, h, 6, bg);
  tft.drawRoundRect(x, y, w, h, 6, borderCol);

  // Icon Badge
  tft.fillRoundRect(x + 8, y + 6, 26, 24, 4, dev.state ? TFT_BLACK : 0x2104);
  tft.setTextColor(dev.state ? dev.color : TFT_WHITE, dev.state ? TFT_BLACK : 0x2104);
  tft.setTextSize(2);
  tft.setCursor(x + 13, y + 10);
  tft.print(dev.icon);

  // Device Name
  tft.setTextColor(textCol, bg);
  tft.setTextSize(1);
  tft.setCursor(x + 40, y + 10);
  tft.print(dev.friendlyName);

  // Status text
  tft.setCursor(x + 40, y + 22);
  tft.print(dev.state ? "STATE: ON" : "STATE: OFF");
}

void drawTelemetryCard() {
  tft.fillRoundRect(10, 138, 300, 78, 6, 0x0841);
  tft.drawRoundRect(10, 138, 300, 78, 6, 0x2965);

  tft.setTextColor(0x07FF, 0x0841);
  tft.setTextSize(1);
  tft.setCursor(18, 146);
  tft.print("ZIGBEE MESH TELEMETRY & HEALTH");

  // Power (W)
  tft.setTextColor(TFT_WHITE, 0x0841);
  tft.setCursor(18, 166);
  tft.print("Power Meter:");
  tft.setTextColor(0xFDC0, 0x0841);
  tft.setTextSize(2);
  tft.setCursor(18, 182);
  tft.printf("%.1f W", liveWatts);

  // Mesh Temp & LQI
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, 0x0841);
  tft.setCursor(160, 166);
  tft.printf("Temp: %.1f C", liveTemp);
  tft.setCursor(160, 182);
  tft.printf("Mesh LQI: %d/255", liveLQI);
  tft.setCursor(160, 198);
  tft.setTextColor(0x07E0, 0x0841);
  tft.print("Network: Online (8 nodes)");
}

void drawFullUI() {
  tft.fillScreen(0x0000);
  drawHeader();

  // 4 Device Control Tiles
  drawDeviceTile(0, 10,  38, 145, 44);
  drawDeviceTile(1, 165, 38, 145, 44);
  drawDeviceTile(2, 10,  88, 145, 44);
  drawDeviceTile(3, 165, 88, 145, 44);

  // Telemetry Section
  drawTelemetryCard();

  // Footer Channel Bar
  tft.fillRect(0, 222, 320, 18, 0x10A2);
  tft.setTextColor(0x07FF, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Flash RGB Yellow on incoming packet
  setRgb(true, true, false);

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Parse state updates
  for (int i = 0; i < 4; i++) {
    if (String(topic) == devices[i].topicState) {
      if (message.indexOf("\"state\":\"ON\"") >= 0 || message == "ON") {
        devices[i].state = true;
      } else if (message.indexOf("\"state\":\"OFF\"") >= 0 || message == "OFF") {
        devices[i].state = false;
      }
      drawDeviceTile(i, (i % 2 == 0 ? 10 : 165), (i < 2 ? 38 : 88), 145, 44);
    }
  }

  // Restore green LED
  setRgb(false, true, false);
}

void reconnectMqtt() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (!mqttClient.connected()) {
    setRgb(false, false, true); // Blue while reconnecting
    if (mqttClient.connect(CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      setRgb(false, true, false); // Green connected
      for (int i = 0; i < 4; i++) {
        mqttClient.subscribe(devices[i].topicState);
      }
      drawHeader();
    } else {
      setRgb(true, false, false); // Red error
    }
  }
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
  tft.fillScreen(0x0000);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Splash
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(25, 95);
  tft.print("Starting Zigbee2MQTT...");
  tft.setTextSize(1);
  tft.setCursor(25, 125);
  tft.print("Connecting to WiFi & Broker...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 15) {
    delay(200);
    retry++;
  }

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  drawFullUI();
  beep(1750, 70);
  setRgb(false, true, false);
}

void loop() {
  if (!mqttClient.connected()) {
    if (millis() - lastMqttRetry > 5000) {
      lastMqttRetry = millis();
      reconnectMqtt();
    }
  } else {
    mqttClient.loop();
  }

  // Periodic simulated telemetry updates
  if (millis() - lastTelemetryUpdate > 4000) {
    lastTelemetryUpdate = millis();
    liveWatts = 380.0 + random(0, 1200) / 10.0;
    liveTemp  = 23.0 + random(0, 25) / 10.0;
    liveLQI   = 130 + random(0, 30);
    drawTelemetryCard();
  }

  // Handle Touch Inputs
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Tile 0: Living Switch (10, 38, 145, 44)
    if (x >= 10 && x <= 155 && y >= 38 && y <= 82) {
      devices[0].state = !devices[0].state;
      beep(1900, 35);
      if (mqttClient.connected()) {
        mqttClient.publish(devices[0].topicSet, devices[0].state ? "{\"state\":\"ON\"}" : "{\"state\":\"OFF\"}");
      }
      drawDeviceTile(0, 10, 38, 145, 44);
      delay(200);
    }
    // Tile 1: Kitchen Light (165, 38, 145, 44)
    else if (x >= 165 && x <= 310 && y >= 38 && y <= 82) {
      devices[1].state = !devices[1].state;
      beep(1900, 35);
      if (mqttClient.connected()) {
        mqttClient.publish(devices[1].topicSet, devices[1].state ? "{\"state\":\"ON\"}" : "{\"state\":\"OFF\"}");
      }
      drawDeviceTile(1, 165, 38, 145, 44);
      delay(200);
    }
    // Tile 2: Power Plug (10, 88, 145, 44)
    else if (x >= 10 && x <= 155 && y >= 88 && y <= 132) {
      devices[2].state = !devices[2].state;
      beep(1600, 35);
      if (mqttClient.connected()) {
        mqttClient.publish(devices[2].topicSet, devices[2].state ? "{\"state\":\"ON\"}" : "{\"state\":\"OFF\"}");
      }
      drawDeviceTile(2, 10, 88, 145, 44);
      delay(200);
    }
    // Tile 3: Garden Relay (165, 88, 145, 44)
    else if (x >= 165 && x <= 310 && y >= 88 && y <= 132) {
      devices[3].state = !devices[3].state;
      beep(1400, 35);
      if (mqttClient.connected()) {
        mqttClient.publish(devices[3].topicSet, devices[3].state ? "{\"state\":\"ON\"}" : "{\"state\":\"OFF\"}");
      }
      drawDeviceTile(3, 165, 88, 145, 44);
      delay(200);
    }
  }

  delay(20);
}
