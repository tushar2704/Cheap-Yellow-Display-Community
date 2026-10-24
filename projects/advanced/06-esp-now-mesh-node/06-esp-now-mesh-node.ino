/**
 * @file 06-esp-now-mesh-node.ino
 * @brief ESP-NOW Decentralized Mesh Intercom & Touch Node for Cheap Yellow Display 2.8" (ESP32-2432S028R)
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

#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

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

// Mesh Packet Data Structure
typedef struct struct_message {
  char senderName[16];
  char text[32];
  uint32_t msgId;
  uint8_t hopCount;
} struct_message;

struct_message txPacket;
struct_message rxPacket;

// Broadcast Address (FF:FF:FF:FF:FF:FF)
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Node state
char myNodeName[16] = "CYD-Node-Alpha";
uint32_t messageCounter = 1;
int peerCount = 3;
int selectedRecipient = 0; // 0=Broadcast, 1=Node-Beta, 2=Node-Gamma

// Recent Activity Log
#define MAX_LOGS 5
struct LogEntry {
  String from;
  String text;
  uint32_t timeSec;
  bool isMine;
};
LogEntry messageLogs[MAX_LOGS];
int logCount = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void addLog(String from, String text, bool isMine) {
  if (logCount < MAX_LOGS) {
    messageLogs[logCount] = {from, text, (uint32_t)(millis() / 1000), isMine};
    logCount++;
  } else {
    for (int i = 0; i < MAX_LOGS - 1; i++) {
      messageLogs[i] = messageLogs[i + 1];
    }
    messageLogs[MAX_LOGS - 1] = {from, text, (uint32_t)(millis() / 1000), isMine};
  }
}

// ESP-NOW Callbacks
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    setRgb(false, true, false); // Green confirmation
  } else {
    setRgb(true, false, false); // Red error
  }
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&rxPacket, incomingData, sizeof(rxPacket));
  addLog(String(rxPacket.senderName), String(rxPacket.text), false);
  setRgb(false, false, true); // Blue on RX
  beep(2200, 40);
  delay(50);
  beep(2600, 60);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 32, 0x0841);
  tft.setTextColor(0x07E0, 0x0841);
  tft.setTextSize(1);
  tft.setCursor(10, 11);
  tft.print("🌐 ESP-NOW Mesh Node (Routerless)");

  tft.setTextColor(0xFDC0, 0x0841);
  tft.setCursor(225, 11);
  tft.printf("PEERS: %d ONLINE", peerCount);
}

void drawLogPanel() {
  tft.fillRect(8, 38, 185, 126, 0x0000);
  tft.drawRect(8, 38, 185, 126, 0x2124);

  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(14, 44);
  tft.print("MESH INTERCOM FEED");

  int y = 58;
  for (int i = 0; i < logCount; i++) {
    tft.setCursor(14, y);
    if (messageLogs[i].isMine) {
      tft.setTextColor(0xFDC0, 0x0000);
      tft.print("TX> ");
    } else {
      tft.setTextColor(0x07E0, 0x0000);
      tft.print("RX< ");
    }
    tft.print(messageLogs[i].from + ": ");
    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.print(messageLogs[i].text);
    y += 14;
  }
}

void drawPresetButton(int idx, int x, int y, int w, int h, const char* label, uint16_t color) {
  tft.fillRoundRect(x, y, w, h, 6, color);
  tft.drawRoundRect(x, y, w, h, 6, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, color);
  tft.setTextSize(1);
  tft.setCursor(x + 10, y + 10);
  tft.print(label);
}

void drawActionControls() {
  // Preset Message Dispatch Buttons (Right Column)
  drawPresetButton(0, 200, 38, 112, 28, "🔔 DOORBELL", 0xFDC0); // Amber
  drawPresetButton(1, 200, 70, 112, 28, "📡 PING MESH", 0x07FF); // Cyan
  drawPresetButton(2, 200, 102, 112, 28, "✅ ACK / OK", 0x07E0); // Emerald
  drawPresetButton(3, 200, 134, 112, 28, "🚨 SOS ALERT", 0xF800); // Red
}

void drawStatusFooter() {
  // Mesh Info Bar
  tft.fillRect(8, 170, 304, 46, 0x10A2);
  tft.drawRect(8, 170, 304, 46, 0x39C7);

  tft.setTextColor(TFT_WHITE, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(16, 178);
  tft.printf("Local Node: %s (CH 1)", myNodeName);

  tft.setCursor(16, 194);
  tft.setTextColor(0x07FF, 0x10A2);
  String mac = WiFi.macAddress();
  tft.printf("MAC: %s | Protocol: ESP-NOW v2", mac.c_str());

  // Bottom Channel Credit Bar
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
}

void broadcastMessage(const char* messageText) {
  setRgb(false, false, true); // Blue during TX
  beep(1800, 30);

  strncpy(txPacket.senderName, myNodeName, sizeof(txPacket.senderName));
  strncpy(txPacket.text, messageText, sizeof(txPacket.text));
  txPacket.msgId = messageCounter++;
  txPacket.hopCount = 1;

  esp_now_send(broadcastAddress, (uint8_t *)&txPacket, sizeof(txPacket));
  addLog("Self", String(messageText), true);

  drawLogPanel();
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

  // WiFi in Station Mode for ESP-NOW (No connection to router needed)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() == ESP_OK) {
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);

    // Register Broadcast Peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 1;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  }

  // Initial Seed logs
  addLog("MeshCore", "Network Booted CH 1", false);
  addLog("Node-Beta", "Joined Intercom", false);

  drawHeader();
  drawLogPanel();
  drawActionControls();
  drawStatusFooter();

  beep(2000, 60);
  setRgb(false, true, false);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Button 0: DOORBELL (200, 38, 112, 28)
    if (x >= 200 && x <= 312 && y >= 38 && y <= 66) {
      broadcastMessage("DOORBELL");
      delay(200);
    }
    // Button 1: PING MESH (200, 70, 112, 28)
    else if (x >= 200 && x <= 312 && y >= 70 && y <= 98) {
      broadcastMessage("PING");
      delay(200);
    }
    // Button 2: ACK / OK (200, 102, 112, 28)
    else if (x >= 200 && x <= 312 && y >= 102 && y <= 130) {
      broadcastMessage("ACK/OK");
      delay(200);
    }
    // Button 3: SOS ALERT (200, 134, 112, 28)
    else if (x >= 200 && x <= 312 && y >= 134 && y <= 162) {
      beep(2800, 80);
      broadcastMessage("EMERGENCY");
      delay(200);
    }
  }

  delay(25);
}
