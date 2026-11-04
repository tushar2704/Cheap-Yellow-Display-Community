/**
 * @file main.cpp
 * @brief CYD BlackBerry Messenger (CYD-BBM) - Off-Grid P2P Communicator
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Hardware:
 * - Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Radio: ESP-NOW 2.4GHz Peer-to-Peer (No Router / Off-Grid)
 * - Audio: GPIO 26 Speaker / Piezo buzzer
 * - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) [Active LOW]
 * - LDR: GPIO 34 (Light sensor)
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// ---------------- Hardware Pinout ----------------
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26
#define LDR_PIN       34

// ---------------- Display & Touch ----------------
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// ---------------- Color Palette (BlackBerry Theme) ----------------
#define BBM_BG        0x0842  // Dark Charcoal
#define BBM_HEADER    0x10A4  // BlackBerry Slate Gray
#define BBM_CYAN      0x05DF  // Classic BBM Cyan Accent
#define BBM_GREEN     0x07E0  // Online Green
#define BBM_RED       0xF800  // PING Red
#define BBM_WHITE     0xFFFF  // Text White
#define BBM_GRAY      0x7BEF  // Subtle Gray
#define BBM_KEY_BG    0x2124  // Chiclet Key Dark Gray
#define BBM_KEY_TXT   0xFFFF  // Chiclet Key Text
#define BBM_BUBBLE_IN 0x18E3  // Incoming Chat Bubble
#define BBM_BUBBLE_OUT 0x0A69 // Outgoing Chat Bubble

// ---------------- Packet Protocol ----------------
#define PKT_BBM_BEACON  0x01
#define PKT_BBM_MSG     0x02
#define PKT_BBM_PING    0x03
#define PKT_BBM_ACK     0x04

struct __attribute__((packed)) BbmPacket {
  uint8_t  type;
  char     senderPin[9];
  char     senderName[12];
  char     targetPin[9];
  uint32_t msgId;
  char     text[44];
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------- Identity & State ----------------
char myPin[9] = "00000000";
char myName[12] = "CYD-User";
uint32_t outgoingMsgCounter = 1;
bool hasUnreadMessage = false;
unsigned long lastBlinkTime = 0;
bool blinkState = false;
unsigned long lastBeaconTime = 0;
unsigned long lastTouchTime = 0;

// Input Buffer
#define MAX_INPUT_LEN 36
char inputBuffer[MAX_INPUT_LEN + 1] = "";
int inputLen = 0;
bool cursorBlink = false;
unsigned long lastCursorToggle = 0;

// Symbol cycle table
const char* symList[] = {":-)", ";-)", ":-D", "CYD!", "<3", "LOL", "BBM", "OMG", "CU!"};
#define SYM_COUNT 9
int symIndex = 0;

// Chat History Buffer
struct ChatEntry {
  bool isMine;
  bool isPing;
  char sender[12];
  char text[44];
  char status[4]; // "", "[D]", "[R]"
};

#define MAX_HISTORY 4
ChatEntry chatLog[MAX_HISTORY];
int chatCount = 0;

// Buddy Table
struct Buddy {
  char pin[9];
  char name[12];
  unsigned long lastSeen;
};
#define MAX_BUDDIES 6
Buddy buddies[MAX_BUDDIES];
int buddyCount = 0;

// ---------------- Hardware Helpers ----------------
void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playChimeKey() {
  tone(SPEAKER_PIN, 1400, 15);
}

void playChimeSent() {
  tone(SPEAKER_PIN, 1200, 30);
  delay(35);
  tone(SPEAKER_PIN, 1800, 40);
}

void playChimeReceive() {
  tone(SPEAKER_PIN, 880, 50);
  delay(60);
  tone(SPEAKER_PIN, 1175, 50);
  delay(60);
  tone(SPEAKER_PIN, 1760, 90);
}

void playChimePing() {
  for (int i = 0; i < 3; i++) {
    tone(SPEAKER_PIN, 2400, 60);
    delay(70);
    tone(SPEAKER_PIN, 1800, 70);
    delay(80);
  }
}

// ---------------- GUI Drawing ----------------
void drawHeader() {
  tft.fillRect(0, 0, 320, 24, BBM_HEADER);
  tft.drawFastHLine(0, 24, 320, BBM_CYAN);

  // BBM Brand Icon & PIN
  tft.fillRoundRect(3, 3, 18, 18, 3, BBM_CYAN);
  tft.setTextColor(TFT_BLACK, BBM_CYAN);
  tft.setTextSize(1);
  tft.setCursor(6, 8);
  tft.print("BB");

  tft.setTextColor(BBM_WHITE, BBM_HEADER);
  tft.setTextSize(1);
  tft.setCursor(26, 8);
  tft.printf("PIN:%s", myPin);

  // Buddy Count
  tft.setTextColor(BBM_GRAY, BBM_HEADER);
  tft.setCursor(140, 8);
  tft.printf("Buddies:%d", buddyCount);

  // Status Indicator
  tft.fillCircle(250, 12, 4, BBM_GREEN);
  tft.setTextColor(BBM_GREEN, BBM_HEADER);
  tft.setCursor(258, 8);
  tft.print("Online");
}

void drawChatTimeline() {
  // Clear chat area
  tft.fillRect(0, 25, 320, 110, BBM_BG);

  int y = 28;
  for (int i = 0; i < chatCount; i++) {
    ChatEntry& e = chatLog[i];

    if (e.isPing) {
      // PING banner
      tft.fillRoundRect(20, y, 280, 22, 4, 0x6000); // Dark red
      tft.drawRoundRect(20, y, 280, 22, 4, BBM_RED);
      tft.setTextColor(BBM_WHITE, 0x6000);
      tft.setTextSize(1);
      tft.setCursor(30, y + 7);
      tft.printf("! PING! from %s", e.sender);
      y += 26;
    } else if (e.isMine) {
      // Outgoing message (Right aligned)
      int bubbleW = constrain((int)strlen(e.text) * 6 + 40, 70, 220);
      int bubbleX = 316 - bubbleW;
      tft.fillRoundRect(bubbleX, y, bubbleW, 22, 4, BBM_BUBBLE_OUT);
      tft.drawRoundRect(bubbleX, y, bubbleW, 22, 4, BBM_CYAN);

      tft.setTextColor(BBM_WHITE, BBM_BUBBLE_OUT);
      tft.setTextSize(1);
      tft.setCursor(bubbleX + 6, y + 7);
      tft.print(e.text);

      // BBM Status Checkmark [D] or [R]
      tft.setTextColor(BBM_CYAN, BBM_BUBBLE_OUT);
      tft.setCursor(bubbleX + bubbleW - 22, y + 7);
      tft.print(e.status);

      y += 26;
    } else {
      // Incoming message (Left aligned)
      int bubbleW = constrain((int)(strlen(e.text) + strlen(e.sender)) * 6 + 28, 90, 240);
      tft.fillRoundRect(4, y, bubbleW, 22, 4, BBM_BUBBLE_IN);
      tft.drawRoundRect(4, y, bubbleW, 22, 4, BBM_GRAY);

      tft.setTextColor(BBM_CYAN, BBM_BUBBLE_IN);
      tft.setTextSize(1);
      tft.setCursor(8, y + 7);
      tft.printf("%s: ", e.sender);

      tft.setTextColor(BBM_WHITE, BBM_BUBBLE_IN);
      tft.print(e.text);

      y += 26;
    }
  }
}

void drawInputField() {
  tft.fillRect(0, 137, 320, 21, 0x0000);
  tft.drawRect(0, 137, 320, 21, BBM_CYAN);

  tft.setTextColor(BBM_CYAN, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(4, 143);
  tft.print(">");

  tft.setTextColor(BBM_WHITE, 0x0000);
  tft.setCursor(16, 143);
  tft.print(inputBuffer);

  // Blinking cursor
  if (cursorBlink) {
    tft.print("_");
  } else {
    tft.print(" ");
  }
}

// ---------------- Chiclet Keyboard Definition ----------------
struct KeyBtn {
  int x, y, w, h;
  const char* label;
  char val;
  uint16_t bgCol;
};

#define KEY_COUNT 30
KeyBtn keys[KEY_COUNT];

void initKeyboard() {
  // Row 1 (10 keys: Q W E R T Y U I O P)
  const char* r1 = "QWERTYUIOP";
  int idx = 0;
  for (int i = 0; i < 10; i++) {
    keys[idx++] = { 2 + i * 31, 160, 30, 18, "", r1[i], BBM_KEY_BG };
  }

  // Row 2 (9 keys: A S D F G H J K L + DEL)
  const char* r2 = "ASDFGHJKL";
  for (int i = 0; i < 9; i++) {
    keys[idx++] = { 8 + i * 30, 180, 28, 18, "", r2[i], BBM_KEY_BG };
  }
  keys[idx++] = { 280, 180, 38, 18, "DEL", '\b', 0x7800 }; // Dark Red

  // Row 3 (7 keys: Z X C V B N M + SYM + CLR)
  const char* r3 = "ZXCVBNM";
  for (int i = 0; i < 7; i++) {
    keys[idx++] = { 14 + i * 30, 200, 28, 18, "", r3[i], BBM_KEY_BG };
  }
  keys[idx++] = { 226, 200, 44, 18, "SYM", '$', 0x31E6 }; // Navy
  keys[idx++] = { 272, 200, 46, 18, "CLR", '#', 0x52AA }; // Gray

  // Row 4 (PING + SPACE + SEND)
  keys[idx++] = { 2, 220, 68, 19, "PING!", '!', BBM_RED };    // Red PING
  keys[idx++] = { 72, 220, 160, 19, "SPACE", ' ', BBM_KEY_BG }; // Space
  keys[idx++] = { 234, 220, 84, 19, "SEND", '\r', 0x04F3 };   // Emerald Green
}

void drawKeyboard() {
  tft.fillRect(0, 158, 320, 82, 0x0000);
  tft.drawFastHLine(0, 158, 320, 0x2124);

  for (int i = 0; i < KEY_COUNT; i++) {
    KeyBtn& k = keys[i];
    tft.fillRoundRect(k.x, k.y, k.w, k.h, 3, k.bgCol);
    tft.drawRoundRect(k.x, k.y, k.w, k.h, 3, BBM_GRAY);

    tft.setTextColor(BBM_KEY_TXT, k.bgCol);
    tft.setTextSize(1);

    if (strlen(k.label) > 0) {
      int textX = k.x + (k.w - (int)strlen(k.label) * 6) / 2;
      tft.setCursor(textX, k.y + 5);
      tft.print(k.label);
    } else {
      tft.setCursor(k.x + (k.w - 6) / 2, k.y + 5);
      tft.print(k.val);
    }
  }
}

// ---------------- Chat & Packet Handling ----------------
void addChat(bool isMine, bool isPing, const char* sender, const char* text, const char* status = "") {
  if (chatCount < MAX_HISTORY) {
    chatLog[chatCount].isMine = isMine;
    chatLog[chatCount].isPing = isPing;
    strncpy(chatLog[chatCount].sender, sender, sizeof(chatLog[chatCount].sender) - 1);
    strncpy(chatLog[chatCount].text, text, sizeof(chatLog[chatCount].text) - 1);
    strncpy(chatLog[chatCount].status, status, sizeof(chatLog[chatCount].status) - 1);
    chatCount++;
  } else {
    // Shift up
    for (int i = 0; i < MAX_HISTORY - 1; i++) {
      chatLog[i] = chatLog[i + 1];
    }
    chatLog[MAX_HISTORY - 1].isMine = isMine;
    chatLog[MAX_HISTORY - 1].isPing = isPing;
    strncpy(chatLog[MAX_HISTORY - 1].sender, sender, sizeof(chatLog[MAX_HISTORY - 1].sender) - 1);
    strncpy(chatLog[MAX_HISTORY - 1].text, text, sizeof(chatLog[MAX_HISTORY - 1].text) - 1);
    strncpy(chatLog[MAX_HISTORY - 1].status, status, sizeof(chatLog[MAX_HISTORY - 1].status) - 1);
  }
  drawChatTimeline();
}

void registerBuddy(const char* pin, const char* name) {
  for (int i = 0; i < buddyCount; i++) {
    if (strcmp(buddies[i].pin, pin) == 0) {
      buddies[i].lastSeen = millis();
      strncpy(buddies[i].name, name, sizeof(buddies[i].name) - 1);
      return;
    }
  }
  if (buddyCount < MAX_BUDDIES) {
    strncpy(buddies[buddyCount].pin, pin, sizeof(buddies[buddyCount].pin) - 1);
    strncpy(buddies[buddyCount].name, name, sizeof(buddies[buddyCount].name) - 1);
    buddies[buddyCount].lastSeen = millis();
    buddyCount++;
    drawHeader();
  }
}

void sendPacket(uint8_t type, const char* targetPin, const char* text) {
  BbmPacket pkt;
  pkt.type = type;
  strncpy(pkt.senderPin, myPin, sizeof(pkt.senderPin) - 1);
  strncpy(pkt.senderName, myName, sizeof(pkt.senderName) - 1);
  strncpy(pkt.targetPin, targetPin, sizeof(pkt.targetPin) - 1);
  pkt.msgId = outgoingMsgCounter++;
  strncpy(pkt.text, text, sizeof(pkt.text) - 1);

  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
}

void triggerScreenStrobe() {
  for (int i = 0; i < 3; i++) {
    tft.fillScreen(TFT_WHITE);
    setRgbLed(true, false, false);
    delay(40);
    tft.fillScreen(BBM_RED);
    setRgbLed(false, false, true);
    delay(40);
  }
  // Restore screen
  tft.fillScreen(BBM_BG);
  drawHeader();
  drawChatTimeline();
  drawInputField();
  drawKeyboard();
}

// ESP-NOW Receive Callback
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(BbmPacket)) return;

  BbmPacket pkt;
  memcpy(&pkt, incomingData, sizeof(pkt));

  // Ignore self transmissions
  if (strcmp(pkt.senderPin, myPin) == 0) return;

  registerBuddy(pkt.senderPin, pkt.senderName);

  if (pkt.type == PKT_BBM_BEACON) {
    // Beacon handled by buddy registration
    return;
  }

  if (pkt.type == PKT_BBM_MSG) {
    addChat(false, false, pkt.senderName, pkt.text);
    playChimeReceive();
    hasUnreadMessage = true;

    // Send BBM Delivery & Read ACK back
    sendPacket(PKT_BBM_ACK, pkt.senderPin, "ACK");
  } else if (pkt.type == PKT_BBM_PING) {
    addChat(false, true, pkt.senderName, "PING!!!");
    hasUnreadMessage = true;
    playChimePing();
    triggerScreenStrobe();

    // Acknowledge PING
    sendPacket(PKT_BBM_ACK, pkt.senderPin, "ACK_PING");
  } else if (pkt.type == PKT_BBM_ACK) {
    // Peer acknowledged our message -> Update status to [R]
    for (int i = chatCount - 1; i >= 0; i--) {
      if (chatLog[i].isMine) {
        strcpy(chatLog[i].status, "[R]");
        drawChatTimeline();
        break;
      }
    }
  }
}

// ESP-NOW Send Callback (Delivery Receipt [D])
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    for (int i = chatCount - 1; i >= 0; i--) {
      if (chatLog[i].isMine && strlen(chatLog[i].status) == 0) {
        strcpy(chatLog[i].status, "[D]");
        drawChatTimeline();
        break;
      }
    }
  }
}

// ---------------- Actions ----------------
void handleKeyPress(char val) {
  playChimeKey();
  hasUnreadMessage = false;
  setRgbLed(false, false, false);

  if (val == '\b') { // Backspace
    if (inputLen > 0) {
      inputLen--;
      inputBuffer[inputLen] = '\0';
      drawInputField();
    }
  } else if (val == '#') { // Clear
    inputLen = 0;
    inputBuffer[0] = '\0';
    drawInputField();
  } else if (val == '$') { // Symbol cycle
    const char* sym = symList[symIndex];
    symIndex = (symIndex + 1) % SYM_COUNT;
    int sLen = strlen(sym);
    if (inputLen + sLen < MAX_INPUT_LEN) {
      strcat(inputBuffer, sym);
      inputLen += sLen;
      drawInputField();
    }
  } else if (val == '!') { // PING!!!
    sendPacket(PKT_BBM_PING, "ALL", "PING!!!");
    addChat(true, true, "Me", "PING!!!", "[D]");
    playChimeSent();
  } else if (val == '\r') { // Send
    if (inputLen > 0) {
      sendPacket(PKT_BBM_MSG, "ALL", inputBuffer);
      addChat(true, false, "Me", inputBuffer, "[D]");
      playChimeSent();

      inputLen = 0;
      inputBuffer[0] = '\0';
      drawInputField();
    }
  } else { // Regular character
    if (inputLen < MAX_INPUT_LEN) {
      inputBuffer[inputLen++] = val;
      inputBuffer[inputLen] = '\0';
      drawInputField();
    }
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=======================================================");
  Serial.println("  CYD BlackBerry Messenger (CYD-BBM) - Project 36");
  Serial.println("  Community Hub: https://github.com/tushar2704/Cheap-Yellow-Display-Community");
  Serial.println("  IamNotIntoTech (@IamNotIntoTech) & Makerrdesigns (@Makerrdesigns)");
  Serial.println("=======================================================\n");

  // GPIO Pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(false, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  // Display Init
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BBM_BG);

  // Touch Init (VSPI dedicated pins)
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // WiFi & Unique BBM PIN
  WiFi.mode(WIFI_STA);
  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(myPin, "%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
  sprintf(myName, "CYD-%02X%02X", mac[4], mac[5]);

  Serial.printf("[BBM] Initialized! My BBM PIN: %s | Name: %s\n", myPin, myName);

  // ESP-NOW Init
  if (esp_now_init() != ESP_OK) {
    Serial.println("[BBM] ESP-NOW Init Failed!");
  }
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  // Register broadcast peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Keyboard Setup
  initKeyboard();

  // Draw Initial UI
  drawHeader();
  drawChatTimeline();
  drawInputField();
  drawKeyboard();

  // Welcome Audio Chime
  playChimeReceive();
  setRgbLed(false, true, false); // Green flash
  delay(150);
  setRgbLed(false, false, false);
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // 1. Periodic Presence Beacon (Every 4 seconds)
  if (now - lastBeaconTime > 4000) {
    lastBeaconTime = now;
    sendPacket(PKT_BBM_BEACON, "ALL", "AVAILABLE");
  }

  // 2. Cursor Blink in Input Field (500ms)
  if (now - lastCursorToggle > 500) {
    lastCursorToggle = now;
    cursorBlink = !cursorBlink;
    drawInputField();
  }

  // 3. Classic BlackBerry Unread Notification Pulse (Red LED on GPIO 4)
  if (hasUnreadMessage) {
    if (now - lastBlinkTime > 1800) {
      lastBlinkTime = now;
      blinkState = !blinkState;
      digitalWrite(LED_RED, blinkState ? LOW : HIGH); // Pulse LED
    }
  }

  // 4. Touch Processing (Hardware Calibrated)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (now - lastTouchTime > 160) {
        lastTouchTime = now;

        // Accurate Landscape touch mapping
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // Check Keyboard Tap (Y >= 158)
        if (screenY >= 158) {
          for (int i = 0; i < KEY_COUNT; i++) {
            KeyBtn& k = keys[i];
            if (screenX >= k.x && screenX <= (k.x + k.w) &&
                screenY >= k.y && screenY <= (k.y + k.h)) {
              handleKeyPress(k.val);
              break;
            }
          }
        }
        // Tap on Chat or Header clears unread status & stops blinking LED
        else {
          hasUnreadMessage = false;
          digitalWrite(LED_RED, HIGH); // Turn off red LED
        }
      }
    }
  }

  delay(15);
}
