/**
 * @file 12-lora-mesh-communicator.ino
 * @brief LoRa Meshtastic Terminal & Long-Range Communicator for Cheap Yellow Display 2.8" (ESP32-2432S028R)
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

// Chat Message Buffer
#define MAX_MSGS 4
struct ChatMsg {
  String sender;
  String text;
  int rssi;
  int snr;
  bool isMine;
};
ChatMsg chatFeed[MAX_MSGS];
int msgCount = 0;

// Input Message Buffer
String currentDraft = "HELLO MESH";
const char* CHANNELS[3] = {"#LongFast", "#Emergency", "#Admin"};
int activeChannelIdx = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

void addMessage(String sender, String text, int rssi, int snr, bool isMine) {
  if (msgCount < MAX_MSGS) {
    chatFeed[msgCount] = {sender, text, rssi, snr, isMine};
    msgCount++;
  } else {
    for (int i = 0; i < MAX_MSGS - 1; i++) {
      chatFeed[i] = chatFeed[i + 1];
    }
    chatFeed[MAX_MSGS - 1] = {sender, text, rssi, snr, isMine};
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 30, 0x0841);
  tft.setTextColor(0x07E0, 0x0841);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("📡 LoRa Meshtastic Terminal");

  // Channel badge
  tft.setTextColor(TFT_BLACK, 0x07FF);
  tft.setCursor(220, 8);
  tft.printf(" %s ", CHANNELS[activeChannelIdx]);
}

void drawChatViewport() {
  tft.fillRect(8, 34, 304, 106, 0x0000);
  tft.drawRect(8, 34, 304, 106, 0x2124);

  int y = 40;
  for (int i = 0; i < msgCount; i++) {
    tft.setCursor(14, y);
    if (chatFeed[i].isMine) {
      tft.setTextColor(0xFDC0, 0x0000);
      tft.print("ME: ");
    } else {
      tft.setTextColor(0x07E0, 0x0000);
      tft.print(chatFeed[i].sender + ": ");
    }

    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.print(chatFeed[i].text);

    // Telemetry annotation
    tft.setTextColor(TFT_DARKGREY, 0x0000);
    tft.setCursor(220, y);
    tft.printf("[%ddBm]", chatFeed[i].rssi);

    y += 24;
  }
}

void drawDraftBar() {
  // Text Input Box (8, 144, 215, 34)
  tft.fillRoundRect(8, 144, 215, 34, 4, 0x1082);
  tft.drawRoundRect(8, 144, 215, 34, 4, 0x31A6);

  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setTextSize(1);
  tft.setCursor(16, 156);
  tft.print("> " + currentDraft + "_");

  // SEND Button (228, 144, 84, 34)
  tft.fillRoundRect(228, 144, 84, 34, 6, 0x07E0);
  tft.drawRoundRect(228, 144, 84, 34, 6, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, 0x07E0);
  tft.setTextSize(1);
  tft.setCursor(252, 156);
  tft.print("SEND");
}

void drawQuickPresets() {
  // 4 Touch Preset Pills (8, 184, 304, 32)
  const char* presets[4] = {"OK / ACK", "SEND SOS", "LOC CHECK", "CH SWITCH"};
  uint16_t cols[4] = {0x07FF, 0xF800, 0xFDC0, 0x2124};

  for (int i = 0; i < 4; i++) {
    int x = 8 + (i * 77);
    int y = 184;
    tft.fillRoundRect(x, y, 73, 30, 4, cols[i]);
    tft.drawRoundRect(x, y, 73, 30, 4, TFT_WHITE);
    tft.setTextColor(TFT_BLACK, cols[i]);
    tft.setTextSize(1);
    tft.setCursor(x + 6, y + 10);
    tft.print(presets[i]);
  }

  // Footer Channel Bar
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
}

void sendPacket(String text) {
  setRgb(false, false, true); // Blue on LoRa TX
  beep(1800, 40);

  addMessage("ME", text, -45, 9, true);
  drawChatViewport();

  delay(100);
  setRgb(false, true, false); // Green standby
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

  addMessage("Node-Alpha", "Meshtastic 915MHz OK", -88, 7, false);
  addMessage("BaseCamp",   "Relay Tower 3 Active", -94, 5, false);

  drawHeader();
  drawChatViewport();
  drawDraftBar();
  drawQuickPresets();

  beep(2000, 60);
  setRgb(false, true, false);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // SEND Button (228, 144, 84, 34)
    if (x >= 228 && x <= 312 && y >= 144 && y <= 178) {
      sendPacket(currentDraft);
      delay(200);
    }
    // Preset 0: OK / ACK (8, 184, 73, 30)
    else if (x >= 8 && x <= 81 && y >= 184 && y <= 214) {
      sendPacket("ROGER / ACK");
      delay(200);
    }
    // Preset 1: SOS (85, 184, 73, 30)
    else if (x >= 85 && x <= 158 && y >= 184 && y <= 214) {
      beep(2800, 80);
      setRgb(true, false, false);
      sendPacket("EMERGENCY SOS");
      delay(200);
    }
    // Preset 2: LOC CHECK (162, 184, 73, 30)
    else if (x >= 162 && x <= 235 && y >= 184 && y <= 214) {
      sendPacket("LOC: 37.77N 122.41W");
      delay(200);
    }
    // Preset 3: CH SWITCH (239, 184, 73, 30)
    else if (x >= 239 && x <= 312 && y >= 184 && y <= 214) {
      activeChannelIdx = (activeChannelIdx + 1) % 3;
      beep(2200, 30);
      drawHeader();
      delay(200);
    }
  }

  delay(25);
}
