/**
 * @file 37-cyd-to-cyd-remote-telecommander.ino
 * @brief CYD-to-CYD Remote Tele-Commander (Master-Slave KVM & Actuator Deck)
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Hardware:
 * - 2× Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Radio: ESP-NOW 2.4GHz Peer-to-Peer (sub-5ms ultra-low latency)
 * - Actuators:
 *     - Backlight PWM: GPIO 21
 *     - Audio Speaker: GPIO 26 (Piezo buzzer tone generation)
 *     - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) [Active LOW]
 *     - Sensors: GPIO 34 (LDR Ambient Light Sensor)
 *     - Aux Relay/Actuator Pin: GPIO 22
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// ---------------- Hardware Pinout ----------------
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define TFT_BL_PIN    21
#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26
#define LDR_PIN       34
#define RELAY_PIN     22

// PWM Backlight Channel
#define BL_PWM_CHANNEL 0
#define BL_PWM_FREQ    5000
#define BL_PWM_RES     8

// ---------------- Display & Touch ----------------
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
Preferences prefs;

// ---------------- Color Palette ----------------
#define COL_BG         0x0842  // Dark Slate
#define COL_PANEL      0x10A4  // Surface Slate
#define COL_BORDER     0x2965  // Subtle Border
#define COL_CYAN       0x05DF  // Master Cyan Accent
#define COL_GREEN      0x07E0  // Slave Emerald Accent
#define COL_RED        0xF800  // Alert Red
#define COL_AMBER      0xFDA0  // Warning Amber
#define COL_WHITE      0xFFFF  // White Text
#define COL_GRAY       0x7BEF  // Dim Text

// ---------------- Packet Definitions ----------------
#define PKT_CMD_CURSOR   0x10
#define PKT_CMD_AUDIO    0x11
#define PKT_CMD_RGB      0x12
#define PKT_CMD_BRIGHT   0x13
#define PKT_CMD_RELAY    0x14
#define PKT_TELEMETRY    0x20

struct __attribute__((packed)) CommandPacket {
  uint8_t  type;
  uint16_t cursorX;
  uint16_t cursorY;
  uint8_t  param1; // Audio ID, RGB code, Brightness 0-100, Relay state
  uint8_t  param2;
};

struct __attribute__((packed)) TelemetryPacket {
  uint8_t  type;
  uint16_t ldrValue;
  int8_t   cpuTemp;
  uint16_t packetCount;
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------- State Variables ----------------
enum DeviceRole {
  ROLE_MASTER = 0,
  ROLE_SLAVE  = 1
};

DeviceRole currentRole = ROLE_MASTER;

// Master state
uint8_t masterBrightness = 100;
bool masterRelayState = false;
uint16_t slaveLdrVal = 0;
int8_t slaveCpuTemp = 0;
uint16_t slavePktCount = 0;
unsigned long lastTelemetryRecv = 0;
unsigned long lastCursorSendTime = 0;

// Slave state
int cursorTargetX = 160;
int cursorTargetY = 120;
int cursorCurrentX = 160;
int cursorCurrentY = 120;
uint8_t slaveBrightness = 100;
bool slaveRelay = false;
uint8_t slaveRgbMode = 0; // 0=OFF, 1=RED, 2=GREEN, 3=BLUE, 4=AMBER
String slaveLastAudio = "None";
unsigned long slaveLastCmdTime = 0;
unsigned long lastTelemetrySendTime = 0;
uint16_t totalPacketsReceived = 0;

unsigned long lastTouchTime = 0;
bool needsFullRedraw = true;

// ---------------- Hardware Output Functions ----------------
void setLocalRgb(uint8_t mode) {
  // Active LOW: LOW=ON, HIGH=OFF
  switch (mode) {
    case 1: // Red
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_BLUE, HIGH);
      break;
    case 2: // Green
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, HIGH);
      break;
    case 3: // Blue
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_BLUE, LOW);
      break;
    case 4: // Amber (Red + Green)
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_BLUE, HIGH);
      break;
    default: // OFF
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_BLUE, HIGH);
      break;
  }
}

void setLocalBrightness(uint8_t percent) {
  percent = constrain(percent, 5, 100);
  uint32_t duty = map(percent, 0, 100, 0, 255);
  ledcWrite(BL_PWM_CHANNEL, duty);
}

void playLocalSound(uint8_t soundId) {
  switch (soundId) {
    case 1: // SIREN
      for (int i = 0; i < 2; i++) {
        tone(SPEAKER_PIN, 1200, 80);
        delay(90);
        tone(SPEAKER_PIN, 1800, 80);
        delay(90);
      }
      break;
    case 2: // LASER
      for (int f = 2400; f > 600; f -= 150) {
        tone(SPEAKER_PIN, f, 12);
        delay(14);
      }
      break;
    case 3: // HORN
      tone(SPEAKER_PIN, 440, 220);
      delay(230);
      break;
    case 4: // CHIRP
      tone(SPEAKER_PIN, 2000, 40);
      delay(50);
      tone(SPEAKER_PIN, 2800, 60);
      break;
  }
}

void playTouchClick() {
  tone(SPEAKER_PIN, 1600, 12);
}

// ---------------- ESP-NOW Send Helper ----------------
void sendCommand(uint8_t type, uint16_t x = 0, uint16_t y = 0, uint8_t p1 = 0, uint8_t p2 = 0) {
  CommandPacket pkt;
  pkt.type = type;
  pkt.cursorX = x;
  pkt.cursorY = y;
  pkt.param1 = p1;
  pkt.param2 = p2;
  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
}

void sendTelemetry() {
  TelemetryPacket pkt;
  pkt.type = PKT_TELEMETRY;
  pkt.ldrValue = analogRead(LDR_PIN);
  pkt.cpuTemp = (int8_t)temperatureRead();
  pkt.packetCount = ++totalPacketsReceived;
  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
}

// ---------------- ESP-NOW Receive Callback ----------------
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(CommandPacket) && currentRole == ROLE_SLAVE) {
    CommandPacket pkt;
    memcpy(&pkt, incomingData, sizeof(pkt));
    slaveLastCmdTime = millis();
    totalPacketsReceived++;

    if (pkt.type == PKT_CMD_CURSOR) {
      cursorTargetX = pkt.cursorX;
      cursorTargetY = pkt.cursorY;
    } else if (pkt.type == PKT_CMD_AUDIO) {
      uint8_t soundId = pkt.param1;
      const char* names[] = {"None", "SIREN", "LASER", "HORN", "CHIRP"};
      slaveLastAudio = (soundId <= 4) ? names[soundId] : "Sound";
      playLocalSound(soundId);
    } else if (pkt.type == PKT_CMD_RGB) {
      slaveRgbMode = pkt.param1;
      setLocalRgb(slaveRgbMode);
    } else if (pkt.type == PKT_CMD_BRIGHT) {
      slaveBrightness = pkt.param1;
      setLocalBrightness(slaveBrightness);
    } else if (pkt.type == PKT_CMD_RELAY) {
      slaveRelay = (pkt.param1 == 1);
      digitalWrite(RELAY_PIN, slaveRelay ? HIGH : LOW);
    }
  } else if (len == sizeof(TelemetryPacket) && currentRole == ROLE_MASTER) {
    TelemetryPacket pkt;
    memcpy(&pkt, incomingData, sizeof(pkt));
    slaveLdrVal = pkt.ldrValue;
    slaveCpuTemp = pkt.cpuTemp;
    slavePktCount = pkt.packetCount;
    lastTelemetryRecv = millis();
  }
}

// ---------------- MASTER GUI Drawing ----------------
void drawMasterUI() {
  tft.fillScreen(COL_BG);

  // Top Header
  tft.fillRect(0, 0, 320, 24, COL_PANEL);
  tft.drawFastHLine(0, 24, 320, COL_CYAN);

  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(6, 8);
  tft.print("[TELE-COMMANDER]");

  // Link status
  bool isLinked = (millis() - lastTelemetryRecv < 2500);
  tft.setTextColor(isLinked ? COL_GREEN : COL_AMBER, COL_PANEL);
  tft.setCursor(140, 8);
  tft.print(isLinked ? "LINK: CONNECTED" : "LINK: SCANNING...");

  // Role Toggle Button
  tft.fillRoundRect(248, 3, 68, 18, 3, COL_CYAN);
  tft.setTextColor(TFT_BLACK, COL_CYAN);
  tft.setCursor(254, 8);
  tft.print("ROLE: MTR");

  // Left Section: Virtual Trackpad
  tft.fillRoundRect(6, 30, 154, 134, 4, COL_PANEL);
  tft.drawRoundRect(6, 30, 154, 134, 4, COL_BORDER);

  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setCursor(14, 36);
  tft.print("TRACKPAD (DRAG)");

  // Cross lines inside trackpad
  tft.drawFastHLine(16, 97, 134, 0x18E3);
  tft.drawFastVLine(83, 46, 102, 0x18E3);
  tft.drawCircle(83, 97, 18, 0x18E3);

  // Backlight Slider
  tft.fillRoundRect(6, 170, 154, 40, 4, COL_PANEL);
  tft.drawRoundRect(6, 170, 154, 40, 4, COL_BORDER);
  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setCursor(12, 176);
  tft.printf("SLAVE BRIGHT: %d%%", masterBrightness);

  // Slider Bar
  tft.fillRoundRect(14, 192, 138, 10, 3, 0x0000);
  int fillW = map(masterBrightness, 0, 100, 0, 138);
  tft.fillRoundRect(14, 192, fillW, 10, 3, COL_CYAN);

  // Right Section: Remote Soundboard (Y: 30 to 96)
  tft.setTextColor(COL_WHITE, COL_BG);
  tft.setCursor(168, 30);
  tft.print("REMOTE SOUNDS:");

  tft.fillRoundRect(168, 42, 68, 24, 3, 0x6000); // Siren (Red)
  tft.setTextColor(COL_WHITE, 0x6000);
  tft.setCursor(182, 50);
  tft.print("SIREN");

  tft.fillRoundRect(244, 42, 68, 24, 3, 0x03E0); // Laser (Green)
  tft.setTextColor(COL_WHITE, 0x03E0);
  tft.setCursor(258, 50);
  tft.print("LASER");

  tft.fillRoundRect(168, 72, 68, 24, 3, 0x001F); // Horn (Blue)
  tft.setTextColor(COL_WHITE, 0x001F);
  tft.setCursor(184, 80);
  tft.print("HORN");

  tft.fillRoundRect(244, 72, 68, 24, 3, 0x7BE0); // Chirp (Amber)
  tft.setTextColor(TFT_BLACK, 0x7BE0);
  tft.setCursor(256, 80);
  tft.print("CHIRP");

  // Remote RGB Picker (Y: 104 to 142)
  tft.setTextColor(COL_WHITE, COL_BG);
  tft.setCursor(168, 104);
  tft.print("REMOTE RGB LED:");

  tft.fillRoundRect(168, 116, 32, 22, 3, COL_RED);
  tft.drawRoundRect(168, 116, 32, 22, 3, COL_WHITE);

  tft.fillRoundRect(204, 116, 32, 22, 3, COL_GREEN);
  tft.drawRoundRect(204, 116, 32, 22, 3, COL_WHITE);

  tft.fillRoundRect(240, 116, 32, 22, 3, 0x001F);
  tft.drawRoundRect(240, 116, 32, 22, 3, COL_WHITE);

  tft.fillRoundRect(276, 116, 36, 22, 3, 0x2124);
  tft.drawRoundRect(276, 116, 36, 22, 3, COL_WHITE);
  tft.setTextColor(COL_WHITE, 0x2124);
  tft.setCursor(282, 123);
  tft.print("OFF");

  // Remote Relay Switch (Y: 148 to 178)
  tft.fillRoundRect(168, 148, 144, 26, 4, masterRelayState ? COL_GREEN : 0x4208);
  tft.drawRoundRect(168, 148, 144, 26, 4, COL_WHITE);
  tft.setTextColor(masterRelayState ? TFT_BLACK : COL_WHITE, masterRelayState ? COL_GREEN : 0x4208);
  tft.setCursor(180, 157);
  tft.printf("ACTUATOR RELAY: %s", masterRelayState ? "ON " : "OFF");

  // Footer: Live Telemetry
  tft.fillRect(0, 218, 320, 22, COL_PANEL);
  tft.drawFastHLine(0, 217, 320, COL_BORDER);
  tft.setTextColor(COL_GRAY, COL_PANEL);
  tft.setCursor(6, 224);
  tft.printf("SLAVE LDR:%04d | CPU:%dC | PKT:%d", slaveLdrVal, slaveCpuTemp, slavePktCount);
}

void updateMasterFooter() {
  tft.fillRect(0, 218, 320, 22, COL_PANEL);
  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(6, 224);
  tft.printf("SLAVE LDR:%04d | CPU:%dC | PKT:%d", slaveLdrVal, slaveCpuTemp, slavePktCount);
}

// ---------------- SLAVE GUI Drawing ----------------
void drawSlaveUI() {
  tft.fillScreen(0x0000);

  // Top Header
  tft.fillRect(0, 0, 320, 24, COL_PANEL);
  tft.drawFastHLine(0, 24, 320, COL_GREEN);

  tft.setTextColor(COL_GREEN, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(6, 8);
  tft.print("[TARGET HUD APPLIANCE]");

  // Connection Indicator
  bool isConnected = (millis() - slaveLastCmdTime < 3000);
  tft.setTextColor(isConnected ? COL_GREEN : COL_AMBER, COL_PANEL);
  tft.setCursor(160, 8);
  tft.print(isConnected ? "RADIO: LINKED" : "RADIO: WAITING");

  // Role Toggle Button
  tft.fillRoundRect(248, 3, 68, 18, 3, COL_GREEN);
  tft.setTextColor(TFT_BLACK, COL_GREEN);
  tft.setCursor(254, 8);
  tft.print("ROLE: SLV");

  // Canvas background grid lines
  for (int x = 20; x < 320; x += 40) tft.drawFastVLine(x, 26, 188, 0x0842);
  for (int y = 46; y < 214; y += 40) tft.drawFastHLine(0, y, 320, 0x0842);

  // Status HUD widgets in corners
  tft.fillRoundRect(8, 30, 130, 60, 4, COL_PANEL);
  tft.drawRoundRect(8, 30, 130, 60, 4, COL_BORDER);
  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setCursor(14, 36);
  tft.print("TELE-STATE HUD");
  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setCursor(14, 48);
  tft.printf("PWM Backlight: %d%%", slaveBrightness);
  tft.setCursor(14, 60);
  tft.printf("Relay Actuator: %s", slaveRelay ? "ON" : "OFF");
  tft.setCursor(14, 72);
  tft.printf("Last Sound: %s", slaveLastAudio.c_str());

  // Bottom Telemetry Bar
  tft.fillRect(0, 218, 320, 22, COL_PANEL);
  tft.drawFastHLine(0, 217, 320, COL_GREEN);
  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setCursor(6, 224);
  tft.printf("LDR SENSOR: %d | ESP32 CPU: %dC | TOTAL PKTS: %d",
             analogRead(LDR_PIN), (int)temperatureRead(), totalPacketsReceived);
}

void updateSlaveCursor() {
  // Erase previous crosshair cursor
  if (cursorCurrentX != cursorTargetX || cursorCurrentY != cursorTargetY) {
    // Redraw grid under old cursor
    tft.drawFastHLine(cursorCurrentX - 12, cursorCurrentY, 25, 0x0000);
    tft.drawFastVLine(cursorCurrentX, cursorCurrentY - 12, 25, 0x0000);
    tft.drawCircle(cursorCurrentX, cursorCurrentY, 6, 0x0000);

    cursorCurrentX = cursorTargetX;
    cursorCurrentY = cursorTargetY;

    // Draw new crosshair cursor in bright emerald green
    tft.drawFastHLine(cursorCurrentX - 12, cursorCurrentY, 25, COL_GREEN);
    tft.drawFastVLine(cursorCurrentX, cursorCurrentY - 12, 25, COL_GREEN);
    tft.drawCircle(cursorCurrentX, cursorCurrentY, 6, COL_GREEN);
    tft.drawPixel(cursorCurrentX, cursorCurrentY, COL_WHITE);
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=======================================================");
  Serial.println("  CYD-to-CYD Remote Tele-Commander - Project 37");
  Serial.println("  Dual CYD Handheld KVM & Remote Actuator Deck");
  Serial.println("  Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("  Channels: @IamNotIntoTech | @Makerrdesigns");
  Serial.println("=======================================================\n");

  // Load Saved Role from NVS
  prefs.begin("cyd_tele", false);
  currentRole = (DeviceRole)prefs.getUChar("role", ROLE_MASTER);

  // Initialize GPIOs
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setLocalRgb(0); // Off

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Backlight PWM Setup
  ledcSetup(BL_PWM_CHANNEL, BL_PWM_FREQ, BL_PWM_RES);
  ledcAttachPin(TFT_BL_PIN, BL_PWM_CHANNEL);
  setLocalBrightness(100);

  // Display Init
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COL_BG);

  // Touch Init (VSPI dedicated pins)
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Radio (ESP-NOW)
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("[RADIO] ESP-NOW Init Failed!");
  }
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Sound indication on boot
  playLocalSound(4);

  // Render initial interface
  if (currentRole == ROLE_MASTER) {
    drawMasterUI();
  } else {
    drawSlaveUI();
  }
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // 1. Role Toggle Check on Touch (Top Right button: X: 248..316, Y: 3..22)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Check if user tapped Role Switcher in header
      if (screenX >= 248 && screenY <= 24) {
        if (now - lastTouchTime > 400) {
          lastTouchTime = now;
          playTouchClick();
          currentRole = (currentRole == ROLE_MASTER) ? ROLE_SLAVE : ROLE_MASTER;
          prefs.putUChar("role", (uint8_t)currentRole);

          if (currentRole == ROLE_MASTER) {
            drawMasterUI();
          } else {
            drawSlaveUI();
          }
          return;
        }
      }

      // ---------------- MASTER TOUCH CONTROLS ----------------
      if (currentRole == ROLE_MASTER) {
        // Trackpad Drag (X: 6..160, Y: 46..164)
        if (screenX >= 6 && screenX <= 160 && screenY >= 46 && screenY <= 164) {
          // Normalize trackpad coordinates to Slave screen (0..320, 26..214)
          int normX = map(screenX, 6, 160, 20, 300);
          int normY = map(screenY, 46, 164, 30, 210);

          if (now - lastCursorSendTime > 25) { // 40Hz streaming
            lastCursorSendTime = now;
            sendCommand(PKT_CMD_CURSOR, normX, normY);
          }
        }
        // Brightness Slider (X: 14..152, Y: 185..205)
        else if (screenX >= 14 && screenX <= 152 && screenY >= 180 && screenY <= 210) {
          if (now - lastTouchTime > 100) {
            lastTouchTime = now;
            playTouchClick();
            masterBrightness = map(screenX, 14, 152, 10, 100);
            masterBrightness = constrain(masterBrightness, 10, 100);

            // Redraw slider bar
            int fillW = map(masterBrightness, 0, 100, 0, 138);
            tft.fillRect(14, 192, 138, 10, 0x0000);
            tft.fillRoundRect(14, 192, fillW, 10, 3, COL_CYAN);

            tft.fillRect(100, 176, 50, 12, COL_PANEL);
            tft.setTextColor(COL_WHITE, COL_PANEL);
            tft.setCursor(12, 176);
            tft.printf("SLAVE BRIGHT: %d%% ", masterBrightness);

            sendCommand(PKT_CMD_BRIGHT, 0, 0, masterBrightness);
          }
        }
        // Remote Sounds (Y: 42 to 96)
        else if (screenX >= 168 && screenX <= 236 && screenY >= 42 && screenY <= 66) {
          if (now - lastTouchTime > 200) {
            lastTouchTime = now;
            playTouchClick();
            sendCommand(PKT_CMD_AUDIO, 0, 0, 1); // Siren
          }
        } else if (screenX >= 244 && screenX <= 312 && screenY >= 42 && screenY <= 66) {
          if (now - lastTouchTime > 200) {
            lastTouchTime = now;
            playTouchClick();
            sendCommand(PKT_CMD_AUDIO, 0, 0, 2); // Laser
          }
        } else if (screenX >= 168 && screenX <= 236 && screenY >= 72 && screenY <= 96) {
          if (now - lastTouchTime > 200) {
            lastTouchTime = now;
            playTouchClick();
            sendCommand(PKT_CMD_AUDIO, 0, 0, 3); // Horn
          }
        } else if (screenX >= 244 && screenX <= 312 && screenY >= 72 && screenY <= 96) {
          if (now - lastTouchTime > 200) {
            lastTouchTime = now;
            playTouchClick();
            sendCommand(PKT_CMD_AUDIO, 0, 0, 4); // Chirp
          }
        }
        // Remote RGB Palette (Y: 116 to 138)
        else if (screenY >= 116 && screenY <= 138) {
          if (now - lastTouchTime > 200) {
            lastTouchTime = now;
            playTouchClick();
            if (screenX >= 168 && screenX <= 200) sendCommand(PKT_CMD_RGB, 0, 0, 1); // Red
            else if (screenX >= 204 && screenX <= 236) sendCommand(PKT_CMD_RGB, 0, 0, 2); // Green
            else if (screenX >= 240 && screenX <= 272) sendCommand(PKT_CMD_RGB, 0, 0, 3); // Blue
            else if (screenX >= 276 && screenX <= 312) sendCommand(PKT_CMD_RGB, 0, 0, 0); // Off
          }
        }
        // Remote Relay Switch (Y: 148 to 174)
        else if (screenX >= 168 && screenX <= 312 && screenY >= 148 && screenY <= 174) {
          if (now - lastTouchTime > 250) {
            lastTouchTime = now;
            playTouchClick();
            masterRelayState = !masterRelayState;

            tft.fillRoundRect(168, 148, 144, 26, 4, masterRelayState ? COL_GREEN : 0x4208);
            tft.drawRoundRect(168, 148, 144, 26, 4, COL_WHITE);
            tft.setTextColor(masterRelayState ? TFT_BLACK : COL_WHITE, masterRelayState ? COL_GREEN : 0x4208);
            tft.setCursor(180, 157);
            tft.printf("ACTUATOR RELAY: %s", masterRelayState ? "ON " : "OFF");

            sendCommand(PKT_CMD_RELAY, 0, 0, masterRelayState ? 1 : 0);
          }
        }
      }
    }
  }

  // ---------------- PERIODIC WORK ----------------
  if (currentRole == ROLE_SLAVE) {
    // Smoothly update target cursor position
    updateSlaveCursor();

    // Stream telemetry back to Master (every 200ms)
    if (now - lastTelemetrySendTime > 200) {
      lastTelemetrySendTime = now;
      sendTelemetry();
    }
  } else if (currentRole == ROLE_MASTER) {
    // Update footer telemetry readout periodically
    static unsigned long lastFooterRefresh = 0;
    if (now - lastFooterRefresh > 500) {
      lastFooterRefresh = now;
      updateMasterFooter();
    }
  }

  delay(12);
}
