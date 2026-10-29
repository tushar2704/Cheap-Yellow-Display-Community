/**
 * @file 03-homelab-wol-deck.ino
 * @brief Standalone Homelab Wake-on-LAN (WoL) Touch Deck
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch
 * - Network: ESP32 Wi-Fi + UDP Magic Packet Broadcaster
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Target Homelab Wi-Fi Configuration (or standalone AP mode)
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";

const char MASTER_PIN[] = "1337";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
WiFiUDP udpClient;

enum ServerState {
  STATE_OFFLINE = 0,
  STATE_WAKING,
  STATE_ONLINE
};

struct ServerNode {
  const char* name;
  const char* ip;
  uint8_t mac[6];
  ServerState state;
  unsigned long wakeStartTime;
};

#define SERVER_COUNT 4
ServerNode serverNodes[SERVER_COUNT] = {
  {"PROXMOX VE NODE 1", "192.168.1.10", {0xBC, 0x24, 0x11, 0x4A, 0x55, 0x01}, STATE_OFFLINE, 0},
  {"TRUENAS SCALE CORE", "192.168.1.20", {0xD8, 0x3A, 0xDD, 0x9B, 0x12, 0x02}, STATE_OFFLINE, 0},
  {"PLEX & JELLYFIN 4K", "192.168.1.30", {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x03}, STATE_OFFLINE, 0},
  {"RTX 4090 AI WORKST", "192.168.1.40", {0xE4, 0x5F, 0x01, 0x77, 0x88, 0x04}, STATE_OFFLINE, 0}
};

bool isDeckLocked = true;
char enteredPin[5] = "";
int pinIndex = 0;
unsigned long lastTouchTime = 0;

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

void sendMagicPacket(uint8_t mac[6]) {
  // WoL Magic Packet: 6 bytes 0xFF followed by MAC repeated 16 times (102 bytes total)
  uint8_t packet[102];
  memset(packet, 0xFF, 6);
  for (int i = 0; i < 16; i++) {
    memcpy(&packet[6 + i * 6], mac, 6);
  }

  // Broadcast to LAN
  IPAddress broadcastIp(255, 255, 255, 255);
  udpClient.beginPacket(broadcastIp, 9);
  udpClient.write(packet, sizeof(packet));
  udpClient.endPacket();

  Serial.printf("⚡ Broadcasted 102-byte UDP Magic Packet to %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void drawPinLockScreen() {
  tft.fillScreen(0x0821);

  // Header
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("HOMELAB WoL DECK  ");
  tft.setTextColor(0xF800, 0x0010);
  tft.print("[SECURITY LOCKED]");

  // PIN Display Card
  tft.fillRoundRect(20, 34, 280, 40, 4, 0x0000);
  tft.drawRoundRect(20, 34, 280, 40, 4, TFT_RED);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(30, 40);
  tft.print("ENTER 4-DIGIT SECURITY PIN:");

  // Asterisks
  tft.setTextSize(2);
  tft.setTextColor(0xFDC0, 0x0000);
  tft.setCursor(120, 52);
  for (int i = 0; i < 4; i++) {
    if (i < pinIndex) tft.print("* ");
    else tft.print("- ");
  }

  // Keypad: 3x4 grid
  const char* keys[12] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "CLR", "0", "OK"};
  int startX = 60;
  int startY = 82;
  int btnW = 60;
  int btnH = 30;

  for (int i = 0; i < 12; i++) {
    int col = i % 3;
    int row = i / 3;
    int bx = startX + col * (btnW + 8);
    int by = startY + row * (btnH + 5);

    uint16_t bg = (i == 11) ? 0x03E0 : (i == 9 ? 0x8000 : 0x18E3);
    tft.fillRoundRect(bx, by, btnW, btnH, 4, bg);
    tft.drawRoundRect(bx, by, btnW, btnH, 4, TFT_LIGHTGREY);

    tft.setTextColor(TFT_WHITE, bg);
    tft.setTextSize(1);
    tft.setCursor(bx + (btnW / 2) - 8, by + 10);
    tft.print(keys[i]);
  }

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print("Default PIN: 1337 | @IamNotIntoTech");
}

void drawDashboard() {
  tft.fillScreen(0x0821);

  // Header
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("HOMELAB SERVER RACK  ");
  tft.setTextColor(0x07E0, 0x0010);
  tft.print("[READY]");

  // Lock Deck Button on top right
  tft.fillRoundRect(240, 2, 74, 20, 3, 0x8000);
  tft.setTextColor(TFT_WHITE, 0x8000);
  tft.setCursor(248, 6);
  tft.print("LOCK DECK");

  // Server Cards: 4 rows
  for (int i = 0; i < SERVER_COUNT; i++) {
    int y = 28 + i * 48;
    ServerNode& s = serverNodes[i];

    uint16_t cardBorder = (s.state == STATE_ONLINE) ? 0x07E0 : 
                          ((s.state == STATE_WAKING) ? 0xFDC0 : TFT_DARKGREY);

    tft.fillRoundRect(6, y, 308, 44, 4, 0x0000);
    tft.drawRoundRect(6, y, 308, 44, 4, cardBorder);

    // Status Indicator Dot
    uint16_t dotColor = (s.state == STATE_ONLINE) ? 0x07E0 : 
                        ((s.state == STATE_WAKING) ? 0xFDC0 : 0xF800);
    tft.fillCircle(18, y + 22, 6, dotColor);

    // Server Info
    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.setTextSize(1);
    tft.setCursor(32, y + 8);
    tft.print(s.name);

    tft.setTextColor(TFT_LIGHTGREY, 0x0000);
    tft.setCursor(32, y + 24);
    tft.printf("IP: %s | MAC: %02X:%02X..", s.ip, s.mac[0], s.mac[1]);

    // Wake Action Button on right
    int btnX = 224;
    int btnY = y + 8;
    int btnW = 82;
    int btnH = 28;

    uint16_t actionBg = (s.state == STATE_WAKING) ? 0xFDC0 : 0x041F;
    uint16_t actionFg = (s.state == STATE_WAKING) ? TFT_BLACK : TFT_WHITE;
    tft.fillRoundRect(btnX, btnY, btnW, btnH, 3, actionBg);
    tft.drawRoundRect(btnX, btnY, btnW, btnH, 3, cardBorder);

    tft.setTextColor(actionFg, actionBg);
    tft.setCursor(btnX + (s.state == STATE_WAKING ? 10 : 16), btnY + 10);
    if (s.state == STATE_WAKING) {
      tft.print("BOOTING..");
    } else if (s.state == STATE_ONLINE) {
      tft.print("ONLINE");
    } else {
      tft.print("⚡ WAKE");
    }
  }

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print("Touch WAKE to transmit UDP Magic Packet | @IamNotIntoTech");
}

void handleKeypadPress(int keyIdx) {
  playBeep(1800, 30);
  if (keyIdx >= 0 && keyIdx <= 8) { // 1 to 9
    if (pinIndex < 4) {
      enteredPin[pinIndex++] = '1' + keyIdx;
      enteredPin[pinIndex] = '\0';
    }
  } else if (keyIdx == 10) { // 0
    if (pinIndex < 4) {
      enteredPin[pinIndex++] = '0';
      enteredPin[pinIndex] = '\0';
    }
  } else if (keyIdx == 9) { // CLR
    pinIndex = 0;
    enteredPin[0] = '\0';
  } else if (keyIdx == 11) { // OK
    if (strcmp(enteredPin, MASTER_PIN) == 0) {
      isDeckLocked = false;
      pinIndex = 0;
      enteredPin[0] = '\0';
      playBeep(2400, 100);
      setRgbLed(false, true, false);
      drawDashboard();
      return;
    } else {
      // Wrong PIN
      playBeep(500, 200);
      setRgbLed(true, false, false);
      pinIndex = 0;
      enteredPin[0] = '\0';
      delay(200);
      setRgbLed(false, false, false);
    }
  }
  drawPinLockScreen();
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 03 Homelab WoL Touch Deck ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Initialize Wi-Fi in Station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Splash
  tft.fillRect(0, 0, 320, 240, 0x0821);
  tft.fillRoundRect(20, 30, 280, 180, 8, 0x0000);
  tft.drawRoundRect(20, 30, 280, 180, 8, 0xF800);

  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(44, 55);
  tft.print("IamNotIntoTech");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(44, 90);
  tft.print("ELECROW 2.8\" CYD PROJECT SUITE");

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(44, 115);
  tft.print("Project 03: Homelab Wake-on-LAN Deck");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("102-Byte Magic Packet | PIN Lock Protection");

  playBeep(1200, 70);
  delay(100);
  playBeep(1800, 120);
  delay(500);

  drawPinLockScreen();
}

void loop() {
  // Update booting states (simulate 8s boot duration)
  unsigned long now = millis();
  for (int i = 0; i < SERVER_COUNT; i++) {
    if (serverNodes[i].state == STATE_WAKING) {
      if (now - serverNodes[i].wakeStartTime > 8000) {
        serverNodes[i].state = STATE_ONLINE;
        playBeep(2200, 60);
        if (!isDeckLocked) drawDashboard();
      }
    }
  }

  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 220) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      if (isDeckLocked) {
        // Keypad grid coordinates
        int startX = 60;
        int startY = 82;
        int btnW = 60;
        int btnH = 30;

        for (int i = 0; i < 12; i++) {
          int col = i % 3;
          int row = i / 3;
          int bx = startX + col * (btnW + 8);
          int by = startY + row * (btnH + 5);

          if (screenX >= bx && screenX <= bx + btnW && screenY >= by && screenY <= by + btnH) {
            handleKeypadPress(i);
            break;
          }
        }
      } else {
        // Dashboard touches
        // Check LOCK DECK button
        if (screenX >= 240 && screenY >= 2 && screenY <= 24) {
          isDeckLocked = true;
          playBeep(1000, 60);
          drawPinLockScreen();
          return;
        }

        // Check 4 server WAKE buttons
        for (int i = 0; i < SERVER_COUNT; i++) {
          int y = 28 + i * 48;
          int btnX = 224;
          int btnY = y + 8;
          int btnW = 82;
          int btnH = 28;

          if (screenX >= btnX && screenX <= btnX + btnW && screenY >= btnY && screenY <= btnY + btnH) {
            if (serverNodes[i].state != STATE_ONLINE) {
              serverNodes[i].state = STATE_WAKING;
              serverNodes[i].wakeStartTime = millis();
              sendMagicPacket(serverNodes[i].mac);

              playBeep(2400, 80);
              setRgbLed(true, true, false); // Amber
              drawDashboard();
            }
            break;
          }
        }
      }
    }
  }

  delay(20);
}
