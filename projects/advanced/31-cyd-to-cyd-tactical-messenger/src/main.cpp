/**
 * @file main.cpp
 * @brief 31 - CYD-to-CYD Tactical P2P Off-Grid Messenger & Pager
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Community Channels:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - Two or more ESP32-2432S028R Cheap Yellow Displays
 * - Protocol: Raw ESP-NOW 2.4GHz Peer-to-Peer
 * - Audio Chimes: GPIO 26
 * - RGB Alert: GPIO 4, 16, 17
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_now.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions
#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26

// Touchscreen Calibration
#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Initialize Display & Touch
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// Packet Definitions
#define PKT_TYPE_CHAT   0x01
#define PKT_TYPE_BEACON 0x02

struct __attribute__((packed)) ChatPacket {
  uint8_t msgType;
  char sender[10];
  char text[42];
  uint32_t msgId;
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Chat History Buffer (Last 5 messages)
struct ChatMessage {
  bool isMine;
  char sender[10];
  char text[42];
  unsigned long timeReceived;
};
#define MAX_CHAT_HISTORY 5
ChatMessage chatLog[MAX_CHAT_HISTORY];
int chatCount = 0;

// Typing Buffer
char inputBuffer[36] = "";
int inputLen = 0;
char myNodeName[10] = "CYD-ALPHA";
bool numMode = false;

// Peer Status
unsigned long lastPeerSeen = 0;
char lastPeerName[10] = "SEARCHING";

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int dur) {
  tone(SPEAKER_PIN, freq, dur);
}

// Draw Functions
void drawHeader();
void drawChatLog();
void drawInputBox();
void drawKeyboard();

void addChatMessage(bool isMine, const char* sender, const char* text) {
  if (chatCount < MAX_CHAT_HISTORY) {
    chatLog[chatCount].isMine = isMine;
    strncpy(chatLog[chatCount].sender, sender, 9);
    chatLog[chatCount].sender[9] = '\0';
    strncpy(chatLog[chatCount].text, text, 41);
    chatLog[chatCount].text[41] = '\0';
    chatLog[chatCount].timeReceived = millis() / 1000;
    chatCount++;
  } else {
    // Shift left
    for (int i = 0; i < MAX_CHAT_HISTORY - 1; i++) {
      chatLog[i] = chatLog[i + 1];
    }
    chatLog[MAX_CHAT_HISTORY - 1].isMine = isMine;
    strncpy(chatLog[MAX_CHAT_HISTORY - 1].sender, sender, 9);
    chatLog[MAX_CHAT_HISTORY - 1].sender[9] = '\0';
    strncpy(chatLog[MAX_CHAT_HISTORY - 1].text, text, 41);
    chatLog[MAX_CHAT_HISTORY - 1].text[41] = '\0';
    chatLog[MAX_CHAT_HISTORY - 1].timeReceived = millis() / 1000;
  }
  drawChatLog();
}

// ESP-NOW Receive Callback
void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len == sizeof(ChatPacket)) {
    ChatPacket pkt;
    memcpy(&pkt, data, sizeof(pkt));

    if (pkt.msgType == PKT_TYPE_CHAT) {
      addChatMessage(false, pkt.sender, pkt.text);
      // Double acoustic chime
      setRgb(false, false, true);
      playBeep(1800, 40);
      delay(60);
      playBeep(2400, 60);
      delay(80);
      setRgb(false, false, false);
    } else if (pkt.msgType == PKT_TYPE_BEACON) {
      lastPeerSeen = millis();
      strncpy(lastPeerName, pkt.sender, 9);
      lastPeerName[9] = '\0';
      drawHeader();
    }
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, 0x1082); // Header navy

  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(1);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(String("NODE: ") + myNodeName, 8, 12);

  bool peerOnline = (millis() - lastPeerSeen < 6000);
  tft.setTextDatum(MR_DATUM);
  if (peerOnline) {
    tft.setTextColor(0x07E0, 0x1082);
    tft.drawString(String("LINKED: ") + lastPeerName, 312, 12);
  } else {
    tft.setTextColor(TFT_LIGHTGREY, 0x1082);
    tft.drawString("PEER: LISTENING...", 312, 12);
  }
}

void drawChatLog() {
  tft.fillRect(0, 24, 320, 110, 0x0821);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  for (int i = 0; i < chatCount; i++) {
    int y = 28 + (i * 21);
    bool isMine = chatLog[i].isMine;

    // Chat Bubble Background
    uint16_t bubbleBg = isMine ? 0x18C3 : 0x2104;
    uint16_t borderCol = isMine ? 0xFDC0 : 0x39C7;
    int bubbleX = isMine ? 60 : 8;
    int bubbleW = 252;

    tft.fillRoundRect(bubbleX, y, bubbleW, 18, 3, bubbleBg);
    tft.drawRoundRect(bubbleX, y, bubbleW, 18, 3, borderCol);

    // Text prefix
    char lineBuf[64];
    snprintf(lineBuf, sizeof(lineBuf), "%s: %s", isMine ? "YOU" : chatLog[i].sender, chatLog[i].text);
    tft.setTextColor(isMine ? 0xFDC0 : TFT_WHITE, bubbleBg);
    tft.drawString(lineBuf, bubbleX + 6, y + 5);
  }
}

void drawInputBox() {
  tft.fillRect(0, 136, 320, 26, 0x1082);
  tft.drawRect(4, 138, 312, 22, 0x39C7);
  tft.fillRect(5, 139, 310, 20, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(String(inputBuffer) + "_", 10, 149);
}

// Virtual Keyboard Layout
const char* keyRowsAlpha[4] = {
  "QWERTYUIOP",
  "ASDFGHJKL",
  "ZXCVBNM<",
  "#  SEND"
};

const char* keyRowsNum[4] = {
  "1234567890",
  "-/:;()$&@\"",
  ".,?!'\"*<",
  "A  SEND"
};

void drawKeyboard() {
  tft.fillRect(0, 164, 320, 76, 0x18C3);

  const char** rows = numMode ? keyRowsNum : keyRowsAlpha;
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);

  // Row 0 (10 keys)
  int row0W = 31;
  for (int i = 0; i < 10; i++) {
    int kx = 4 + i * row0W;
    tft.fillRoundRect(kx, 166, row0W - 2, 16, 2, 0x2104);
    tft.setTextColor(TFT_WHITE, 0x2104);
    char ch[2] = {rows[0][i], '\0'};
    tft.drawString(ch, kx + (row0W / 2), 174);
  }

  // Row 1 (9 keys)
  int row1W = 33;
  for (int i = 0; i < 9; i++) {
    int kx = 11 + i * row1W;
    tft.fillRoundRect(kx, 184, row1W - 2, 16, 2, 0x2104);
    tft.setTextColor(TFT_WHITE, 0x2104);
    char ch[2] = {rows[1][i], '\0'};
    tft.drawString(ch, kx + (row1W / 2), 192);
  }

  // Row 2 (8 keys: 7 letters + Backspace '<')
  int row2W = 36;
  for (int i = 0; i < 8; i++) {
    int kx = 16 + i * row2W;
    bool isDel = (rows[2][i] == '<');
    uint16_t bg = isDel ? 0xF800 : 0x2104;
    tft.fillRoundRect(kx, 202, row2W - 2, 16, 2, bg);
    tft.setTextColor(TFT_WHITE, bg);
    if (isDel) {
      tft.drawString("DEL", kx + (row2W / 2), 210);
    } else {
      char ch[2] = {rows[2][i], '\0'};
      tft.drawString(ch, kx + (row2W / 2), 210);
    }
  }

  // Row 3: Mode Switch, Spacebar, Send
  // Mode switch
  tft.fillRoundRect(4, 220, 48, 18, 2, 0x39C7);
  tft.setTextColor(TFT_WHITE, 0x39C7);
  tft.drawString(numMode ? "ABC" : "123", 28, 229);

  // Spacebar
  tft.fillRoundRect(56, 220, 180, 18, 2, 0x2104);
  tft.setTextColor(TFT_LIGHTGREY, 0x2104);
  tft.drawString("SPACE", 146, 229);

  // Send Button
  tft.fillRoundRect(240, 220, 76, 18, 2, 0x07E0);
  tft.setTextColor(TFT_BLACK, 0x07E0);
  tft.drawString("SEND", 278, 229);
}

void sendMessage() {
  if (inputLen == 0) return;

  ChatPacket pkt;
  pkt.msgType = PKT_TYPE_CHAT;
  strncpy(pkt.sender, myNodeName, 9);
  pkt.sender[9] = '\0';
  strncpy(pkt.text, inputBuffer, 41);
  pkt.text[41] = '\0';
  pkt.msgId = millis();

  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));

  addChatMessage(true, myNodeName, inputBuffer);

  // Clear input
  inputBuffer[0] = '\0';
  inputLen = 0;
  drawInputBox();

  playBeep(2200, 30);
  setRgb(false, true, false);
  delay(15);
  setRgb(false, false, false);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[CYD-31] CYD Tactical Messenger Initializing...");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Pick unique node ID based on MAC
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(myNodeName, sizeof(myNodeName), "CYD-%02X%02X", mac[4], mac[5]);

  // ESP-NOW Setup
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_now_init();
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  drawHeader();
  drawChatLog();
  drawInputBox();
  drawKeyboard();

  playBeep(1500, 70);
  delay(80);
  playBeep(2100, 90);
}

unsigned long lastBeacon = 0;

void loop() {
  // Broadcast Presence Beacon every 3 seconds
  if (millis() - lastBeacon > 3000) {
    lastBeacon = millis();
    ChatPacket pkt;
    pkt.msgType = PKT_TYPE_BEACON;
    strncpy(pkt.sender, myNodeName, 9);
    pkt.sender[9] = '\0';
    pkt.text[0] = '\0';
    pkt.msgId = millis();
    esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
    drawHeader();
  }

  // Handle Touch Inputs on Virtual Keyboard
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    const char** rows = numMode ? keyRowsNum : keyRowsAlpha;

    // Keyboard Y ranges:
    // Row 0: 166 - 182
    if (y >= 164 && y <= 182) {
      int idx = (x - 4) / 31;
      if (idx >= 0 && idx < 10 && inputLen < 35) {
        inputBuffer[inputLen++] = rows[0][idx];
        inputBuffer[inputLen] = '\0';
        drawInputBox();
        playBeep(1800, 15);
        delay(140);
      }
    }
    // Row 1: 184 - 200
    else if (y >= 183 && y <= 200) {
      int idx = (x - 11) / 33;
      if (idx >= 0 && idx < 9 && inputLen < 35) {
        inputBuffer[inputLen++] = rows[1][idx];
        inputBuffer[inputLen] = '\0';
        drawInputBox();
        playBeep(1800, 15);
        delay(140);
      }
    }
    // Row 2: 202 - 218
    else if (y >= 201 && y <= 218) {
      int idx = (x - 16) / 36;
      if (idx >= 0 && idx < 7 && inputLen < 35) {
        inputBuffer[inputLen++] = rows[2][idx];
        inputBuffer[inputLen] = '\0';
        drawInputBox();
        playBeep(1800, 15);
        delay(140);
      } else if (idx == 7) {
        // Backspace
        if (inputLen > 0) {
          inputLen--;
          inputBuffer[inputLen] = '\0';
          drawInputBox();
          playBeep(1200, 20);
          delay(140);
        }
      }
    }
    // Row 3: 219 - 238
    else if (y >= 219 && y <= 240) {
      // Mode switch (x: 4 to 52)
      if (x >= 4 && x <= 52) {
        numMode = !numMode;
        drawKeyboard();
        playBeep(1600, 20);
        delay(180);
      }
      // Spacebar (x: 56 to 236)
      else if (x >= 56 && x <= 236 && inputLen < 35) {
        inputBuffer[inputLen++] = ' ';
        inputBuffer[inputLen] = '\0';
        drawInputBox();
        playBeep(1600, 15);
        delay(140);
      }
      // Send button (x >= 240)
      else if (x >= 240) {
        sendMessage();
        delay(200);
      }
    }
  }

  delay(15);
}
