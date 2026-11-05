/**
 * @file main.cpp
 * @brief CYD Keep Talking & Nobody Explodes - Two-Player Bomb Defusal
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Hardware:
 * - Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Audio: GPIO 26 Speaker / Piezo buzzer (Ticking clock, strikes, explosion)
 * - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) [Active LOW]
 * - Wireless: ESP-NOW Peer-to-Peer 2.4GHz Sync (Bomb <-> Manual)
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <esp_now.h>

// ---------------- Pinout ----------------
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26

// ---------------- Display & Touch ----------------
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// ---------------- Color Palette ----------------
#define COL_BG          0x0821  // Dark Metallic
#define COL_PANEL       0x18E3  // Military Slate
#define COL_BORDER      0x39E7  // Chassis Rivets
#define COL_RED         0xF800  // Red Wire / Alarm
#define COL_BLUE        0x001F  // Blue Wire
#define COL_YELLOW      0xFFE0  // Yellow Wire
#define COL_GREEN       0x07E0  // Green Wire / Defused
#define COL_WHITE       0xFFFF  // White Wire / Text
#define COL_GRAY        0x7BEF  // Cut Wire / Inactive
#define COL_DIGIT_RED   0xF900  // 7-Segment LED glow

// ---------------- Game Roles ----------------
enum GameRole {
  ROLE_SELECT,
  ROLE_BOMB,
  ROLE_MANUAL
};

GameRole currentRole = ROLE_SELECT;

// ---------------- Wire Color Definitions ----------------
const uint16_t WIRE_COLORS[] = { COL_RED, COL_BLUE, COL_YELLOW, COL_WHITE };
const char* WIRE_NAMES[] = { "RED", "BLUE", "YEL", "WHT" };

// ---------------- ESP-NOW Packet ----------------
struct DefusalPacket {
  uint8_t msgType;             // 0=STATE, 1=STRIKE, 2=DISARMED, 3=BOOM
  char serial[8];              // e.g. "CYD-7B9"
  uint8_t secondsRemaining;
  uint8_t strikes;
  uint8_t wireColors[4];       // 0=RED, 1=BLUE, 2=YELLOW, 3=WHITE
  bool wireCut[4];
  uint8_t targetWireIndex;     // Correct wire index
  uint8_t keypadOrder[4];      // Sequence 0..3
  uint8_t keypadStep;          // Next required step (0..3)
  bool keypadDone;
  bool isDefused;
  bool isDetonated;
};

DefusalPacket gameState;
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

// Timer state
unsigned long lastTickTime = 0;
unsigned long lastPacketSend = 0;
bool buzzerTickState = false;

// Manual Screen Tab
int manualTab = 0; // 0=Wires, 1=Keypad

// ---------------- Audio Engine ----------------
void playTick(bool urgent) {
  tone(SPEAKER_PIN, urgent ? 1600 : 1000, 15);
}

void playStrikeSound() {
  tone(SPEAKER_PIN, 220, 250);
  delay(260);
  noTone(SPEAKER_PIN);
}

void playDisarmFanfare() {
  int notes[] = { 440, 554, 659, 880, 1108 };
  for (int n : notes) {
    tone(SPEAKER_PIN, n, 80);
    delay(90);
  }
  noTone(SPEAKER_PIN);
}

void playExplosion() {
  for (int i = 0; i < 40; i++) {
    tone(SPEAKER_PIN, random(80, 350), 15);
    delay(15);
  }
  noTone(SPEAKER_PIN);
}

// ---------------- RGB Control ----------------
void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

// ---------------- Broadcast Packet ----------------
void sendGameState() {
  esp_now_send(broadcastAddress, (uint8_t*)&gameState, sizeof(gameState));
}

// ---------------- ESP-NOW Receive Callback ----------------
bool manualGameOverTriggered = false;
void drawGameOverScreen(bool victory);

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(DefusalPacket)) {
    DefusalPacket incoming;
    memcpy(&incoming, incomingData, sizeof(incoming));
    
    // If we are MANUAL unit, update our display from Bomb unit
    if (currentRole == ROLE_MANUAL) {
      gameState = incoming;
      if (!manualGameOverTriggered && (gameState.isDetonated || gameState.isDefused)) {
        manualGameOverTriggered = true;
        drawGameOverScreen(gameState.isDefused);
        if (gameState.isDefused) playDisarmFanfare();
        else playExplosion();
      }
    }
  }
}

// ---------------- Initialize New Bomb Game ----------------
void initNewGame() {
  randomSeed(millis() + analogRead(34));
  
  int serialNum = random(100, 999);
  snprintf(gameState.serial, sizeof(gameState.serial), "CYD-%d", serialNum);
  
  gameState.secondsRemaining = 60;
  gameState.strikes = 0;
  gameState.keypadStep = 0;
  gameState.keypadDone = false;
  gameState.isDefused = false;
  gameState.isDetonated = false;

  // Randomize Wire Colors
  for (int i = 0; i < 4; i++) {
    gameState.wireColors[i] = random(0, 4);
    gameState.wireCut[i] = false;
  }

  // Determine correct wire according to the rule:
  // If serial last digit is odd AND there is at least one RED wire -> Cut 2nd wire (index 1)
  // Else if serial last digit is even AND there is at least one BLUE wire -> Cut 4th wire (index 3)
  // Otherwise -> Cut 1st wire (index 0)
  int lastDigit = serialNum % 10;
  bool hasRed = false;
  bool hasBlue = false;
  for (int i = 0; i < 4; i++) {
    if (gameState.wireColors[i] == 0) hasRed = true;
    if (gameState.wireColors[i] == 1) hasBlue = true;
  }

  if ((lastDigit % 2 != 0) && hasRed) {
    gameState.targetWireIndex = 1;
  } else if ((lastDigit % 2 == 0) && hasBlue) {
    gameState.targetWireIndex = 3;
  } else {
    gameState.targetWireIndex = 0;
  }

  // Keypad correct sequence: 2 -> 0 -> 3 -> 1
  gameState.keypadOrder[0] = 2; // λ
  gameState.keypadOrder[1] = 0; // Ω
  gameState.keypadOrder[2] = 3; // Ψ
  gameState.keypadOrder[3] = 1; // π
}

// ---------------- UI: Role Selector ----------------
void drawRoleSelector() {
  tft.fillScreen(COL_BG);
  
  tft.drawRoundRect(8, 8, 304, 224, 8, COL_BORDER);
  tft.fillRect(10, 10, 300, 36, COL_PANEL);
  
  tft.setTextFont(4);
  tft.setTextColor(COL_YELLOW, COL_PANEL);
  tft.drawCentreString("DEFUSAL SQUAD", 160, 16, 4);
  
  tft.setTextFont(2);
  tft.setTextColor(COL_WHITE, COL_BG);
  tft.drawCentreString("SELECT CYD ROLE FOR THIS UNIT", 160, 56, 2);

  // Button 1: BOMB UNIT
  tft.fillRoundRect(24, 85, 272, 54, 8, COL_RED);
  tft.drawRoundRect(24, 85, 272, 54, 8, COL_WHITE);
  tft.setTextFont(4);
  tft.setTextColor(COL_WHITE, COL_RED);
  tft.drawCentreString("PLAY AS BOMB", 160, 98, 4);

  // Button 2: MANUAL UNIT
  tft.fillRoundRect(24, 155, 272, 54, 8, 0x03E0);
  tft.drawRoundRect(24, 155, 272, 54, 8, COL_WHITE);
  tft.setTextFont(4);
  tft.setTextColor(COL_WHITE, 0x03E0);
  tft.drawCentreString("DEFUSAL MANUAL", 160, 168, 4);
}

void drawStrikes() {
  for (int s = 0; s < 3; s++) {
    uint16_t ledCol = (s < gameState.strikes) ? COL_RED : 0x2104;
    tft.fillCircle(85 + (s * 18), 32, 6, ledCol);
    tft.drawCircle(85 + (s * 18), 32, 6, COL_WHITE);
  }
}

void drawSingleWire(int w) {
  int wy = 90 + (w * 32);
  tft.drawRoundRect(16, wy, 130, 24, 4, COL_PANEL);
  
  if (gameState.wireCut[w]) {
    tft.fillRect(18, wy + 2, 50, 20, COL_GRAY);
    tft.fillRect(80, wy + 2, 64, 20, COL_GRAY);
    tft.setTextFont(2);
    tft.setTextColor(COL_BG, COL_GRAY);
    tft.drawCentreString("CUT", 81, wy + 5, 2);
  } else {
    uint16_t wCol = WIRE_COLORS[gameState.wireColors[w]];
    tft.fillRect(18, wy + 2, 126, 20, wCol);
    tft.setTextFont(2);
    tft.setTextColor((wCol == COL_WHITE || wCol == COL_YELLOW) ? 0x0000 : COL_WHITE, wCol);
    tft.drawCentreString(WIRE_NAMES[gameState.wireColors[w]], 81, wy + 5, 2);
  }
}

const char* KEYPAD_SYMBOLS[] = { "[A] OMEGA", "[B] PI", "[C] LAMBDA", "[D] PSI" };

void drawSingleKeypad(int k) {
  int ky = 90 + (k * 32);
  uint16_t kCol = gameState.keypadDone ? 0x03E0 : COL_PANEL;
  tft.fillRoundRect(174, ky, 130, 24, 4, kCol);
  tft.drawRoundRect(174, ky, 130, 24, 4, COL_BORDER);
  tft.setTextFont(2);
  tft.setTextColor(COL_WHITE, kCol);
  tft.drawCentreString(KEYPAD_SYMBOLS[k], 239, ky + 5, 2);
}

// ---------------- UI: Bomb Screen ----------------
void drawBombScreen() {
  tft.fillScreen(COL_BG);

  // Top Chassis Panel
  tft.fillRect(0, 0, 320, 48, COL_PANEL);
  tft.drawFastHLine(0, 48, 320, COL_BORDER);

  // Serial Number
  tft.setTextFont(2);
  tft.setTextColor(COL_GRAY, COL_PANEL);
  tft.setCursor(10, 8);
  tft.print("SERIAL:");
  tft.setTextColor(COL_YELLOW, COL_PANEL);
  tft.setCursor(65, 8);
  tft.print(gameState.serial);

  // Strike LEDs (X X X)
  tft.setTextFont(2);
  tft.setTextColor(COL_GRAY, COL_PANEL);
  tft.setCursor(10, 26);
  tft.print("STRIKES:");
  drawStrikes();

  // Large 7-Segment Countdown Timer Box
  tft.drawRoundRect(205, 6, 105, 36, 4, COL_RED);
  tft.fillRect(207, 8, 101, 32, 0x0000);
  
  tft.setTextFont(4);
  tft.setTextColor(COL_DIGIT_RED, 0x0000);
  char timeBuf[8];
  snprintf(timeBuf, sizeof(timeBuf), "00:%02d", gameState.secondsRemaining);
  tft.drawCentreString(timeBuf, 257, 12, 4);

  // Left Module: Wires (X: 10, Y: 58, W: 140, H: 172)
  tft.drawRoundRect(8, 56, 146, 176, 6, COL_PANEL);
  tft.fillRoundRect(10, 58, 142, 172, 4, 0x0020);
  tft.setTextFont(2);
  tft.setTextColor(COL_WHITE, 0x0020);
  tft.drawCentreString("MODULE 1: WIRES", 81, 64, 2);

  // Draw 4 Wires
  for (int w = 0; w < 4; w++) {
    drawSingleWire(w);
  }

  // Right Module: Cryptic Keypad (X: 166, Y: 56, W: 146, H: 176)
  tft.drawRoundRect(166, 56, 146, 176, 6, COL_PANEL);
  tft.fillRoundRect(168, 58, 142, 172, 4, 0x0020);
  tft.setTextFont(2);
  tft.setTextColor(COL_WHITE, 0x0020);
  tft.drawCentreString("MODULE 2: KEYPAD", 239, 64, 2);

  for (int k = 0; k < 4; k++) {
    drawSingleKeypad(k);
  }
}

// ---------------- UI: Manual Screen ----------------
void drawManualScreen() {
  tft.fillScreen(0x0020); // Dark Blueprint Blue
  
  // Header
  tft.fillRect(0, 0, 320, 32, 0x0188);
  tft.drawFastHLine(0, 32, 320, COL_WHITE);
  
  tft.setTextFont(2);
  tft.setTextColor(COL_WHITE, 0x0188);
  tft.setCursor(8, 8);
  tft.printf("DEFUSAL MANUAL // BOMB: %s", gameState.serial);
  
  // Timer & Strikes live mirror
  tft.setTextColor(COL_YELLOW, 0x0188);
  tft.setCursor(240, 8);
  tft.printf("%02ds | %dX", gameState.secondsRemaining, gameState.strikes);

  // Tab Buttons
  tft.fillRoundRect(8, 38, 148, 26, 4, (manualTab == 0) ? COL_WHITE : 0x0104);
  tft.setTextFont(2);
  tft.setTextColor((manualTab == 0) ? 0x0000 : COL_WHITE, (manualTab == 0) ? COL_WHITE : 0x0104);
  tft.drawCentreString("PAGE 1: WIRES", 82, 43, 2);

  tft.fillRoundRect(164, 38, 148, 26, 4, (manualTab == 1) ? COL_WHITE : 0x0104);
  tft.setTextFont(2);
  tft.setTextColor((manualTab == 1) ? 0x0000 : COL_WHITE, (manualTab == 1) ? COL_WHITE : 0x0104);
  tft.drawCentreString("PAGE 2: KEYPAD", 238, 43, 2);

  // Content Area
  tft.drawRoundRect(8, 70, 304, 162, 6, COL_WHITE);
  tft.fillRect(10, 72, 300, 158, 0x0041);

  if (manualTab == 0) {
    tft.setTextFont(2);
    tft.setTextColor(COL_YELLOW, 0x0041);
    tft.setCursor(16, 78);
    tft.print("SECTION 1: 4-WIRE PROTOCOL");

    tft.setTextFont(1);
    tft.setTextColor(COL_WHITE, 0x0041);
    tft.setCursor(16, 102);
    tft.print("1. If Serial ends in an ODD number AND");
    tft.setCursor(16, 114);
    tft.print("   there is at least ONE RED wire:");
    tft.setTextColor(COL_GREEN, 0x0041);
    tft.setCursor(30, 128);
    tft.print("-> CUT THE 2ND WIRE FROM TOP!");

    tft.setTextColor(COL_WHITE, 0x0041);
    tft.setCursor(16, 146);
    tft.print("2. Else if Serial ends in EVEN number AND");
    tft.setCursor(16, 158);
    tft.print("   there is at least ONE BLUE wire:");
    tft.setTextColor(COL_GREEN, 0x0041);
    tft.setCursor(30, 172);
    tft.print("-> CUT THE 4TH WIRE FROM TOP!");

    tft.setTextColor(COL_WHITE, 0x0041);
    tft.setCursor(16, 190);
    tft.print("3. Otherwise:");
    tft.setTextColor(COL_GREEN, 0x0041);
    tft.setCursor(30, 204);
    tft.print("-> CUT THE VERY FIRST WIRE!");
  } else {
    tft.setTextFont(2);
    tft.setTextColor(COL_YELLOW, 0x0041);
    tft.setCursor(16, 78);
    tft.print("SECTION 2: KEYPAD SEQUENCE ORDER");

    tft.setTextFont(2);
    tft.setTextColor(COL_WHITE, 0x0041);
    tft.setCursor(16, 105);
    tft.print("ORDER OF PRESSING:");
    
    tft.setTextColor(COL_GREEN, 0x0041);
    tft.setCursor(24, 130);
    tft.print("1ST: [C] LAMBDA");
    tft.setCursor(24, 152);
    tft.print("2ND: [A] OMEGA");
    tft.setCursor(24, 174);
    tft.print("3RD: [D] PSI");
    tft.setCursor(24, 196);
    tft.print("4TH: [B] PI");
  }
}

// ---------------- Game Over Screen ----------------
void drawGameOverScreen(bool victory) {
  tft.fillScreen(victory ? 0x03E0 : COL_RED);
  
  tft.setTextFont(4);
  tft.setTextColor(COL_WHITE, victory ? 0x03E0 : COL_RED);
  tft.drawCentreString(victory ? "BOMB DEFUSED!" : "BOOM! DETONATED!", 160, 60, 4);

  tft.setTextFont(2);
  tft.drawCentreString(victory ? "TACTICAL SQUAD SUCCESSFUL" : "EVERYBODY EXPLODED", 160, 105, 2);

  // Restart Button
  tft.fillRoundRect(60, 150, 200, 48, 8, COL_WHITE);
  tft.setTextColor(0x0000, COL_WHITE);
  tft.setTextFont(4);
  tft.drawCentreString("PLAY AGAIN", 160, 160, 4);
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println(F("[BOMB DEFUSAL] Initializing Squad Systems..."));

  // Pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRGB(false, false, false);
  noTone(SPEAKER_PIN);

  // Initialize Display
  tft.init();
  tft.setRotation(1);

  // Initialize Touch
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // ESP-NOW Initialization
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 1;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
    esp_now_register_recv_cb(onDataRecv);
  }

  initNewGame();
  drawRoleSelector();
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // 1. Role Selection Handling
  if (currentRole == ROLE_SELECT) {
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      int tx = map(p.x, 240, 3800, 0, 320);
      int ty = map(p.y, 240, 3800, 0, 240);

      // Play As Bomb
      if (tx >= 24 && tx <= 296 && ty >= 85 && ty <= 140) {
        currentRole = ROLE_BOMB;
        tone(SPEAKER_PIN, 1200, 60);
        drawBombScreen();
        delay(250);
      }
      // Play As Manual
      else if (tx >= 24 && tx <= 296 && ty >= 155 && ty <= 210) {
        currentRole = ROLE_MANUAL;
        tone(SPEAKER_PIN, 900, 60);
        drawManualScreen();
        delay(250);
      }
    }
    delay(20);
    return;
  }

  // 2. Bomb Unit Loop
  if (currentRole == ROLE_BOMB) {
    if (!gameState.isDefused && !gameState.isDetonated) {
      // 1-Second Timer Tick
      if (now - lastTickTime >= 1000) {
        lastTickTime = now;
        if (gameState.secondsRemaining > 0) {
          gameState.secondsRemaining--;
          playTick(gameState.secondsRemaining <= 10);
        } else {
          gameState.isDetonated = true;
        }

        // Send state to manual unit
        sendGameState();

        // Update Timer Display
        tft.fillRect(207, 8, 101, 32, 0x0000);
        tft.setTextFont(4);
        tft.setTextColor(COL_DIGIT_RED, 0x0000);
        char timeBuf[8];
        snprintf(timeBuf, sizeof(timeBuf), "00:%02d", gameState.secondsRemaining);
        tft.drawCentreString(timeBuf, 257, 12, 4);

        // Flash Red LED when critical
        if (gameState.secondsRemaining <= 10) {
          setRGB(true, false, false);
        } else {
          setRGB(false, false, false);
        }
      }

      // Touch Input on Bomb Modules
      if (ts.touched()) {
        TS_Point p = ts.getPoint();
        int tx = map(p.x, 240, 3800, 0, 320);
        int ty = map(p.y, 240, 3800, 0, 240);

        // Check Wire Cutting (X: 16-146, Y: 90..218)
        if (tx >= 16 && tx <= 146) {
          for (int w = 0; w < 4; w++) {
            int wy = 90 + (w * 32);
            if (ty >= wy && ty <= wy + 24 && !gameState.wireCut[w]) {
              gameState.wireCut[w] = true;
              
              if (w == gameState.targetWireIndex) {
                // Correct Wire Cut!
                tone(SPEAKER_PIN, 1800, 100);
              } else {
                // Wrong Wire Cut -> STRIKE!
                gameState.strikes++;
                playStrikeSound();
                if (gameState.strikes >= 3) {
                  gameState.isDetonated = true;
                }
              }

              // Check if disarmed (correct wire cut and keypad done)
              if (gameState.wireCut[gameState.targetWireIndex] && gameState.keypadDone) {
                gameState.isDefused = true;
              }

              sendGameState();
              drawSingleWire(w);
              drawStrikes();
              delay(200);
              break;
            }
          }
        }

        // Check Keypad Module (X: 174-304, Y: 90..218)
        else if (tx >= 174 && tx <= 304 && !gameState.keypadDone) {
          for (int k = 0; k < 4; k++) {
            int ky = 90 + (k * 32);
            if (ty >= ky && ty <= ky + 24) {
              if (k == gameState.keypadOrder[gameState.keypadStep]) {
                gameState.keypadStep++;
                tone(SPEAKER_PIN, 1400 + (gameState.keypadStep * 200), 50);
                if (gameState.keypadStep >= 4) {
                  gameState.keypadDone = true;
                  tone(SPEAKER_PIN, 2400, 120);
                  if (gameState.wireCut[gameState.targetWireIndex]) {
                    gameState.isDefused = true;
                  }
                }
              } else {
                // Wrong Keypad order -> STRIKE!
                gameState.strikes++;
                gameState.keypadStep = 0; // Reset keypad progress
                playStrikeSound();
                if (gameState.strikes >= 3) {
                  gameState.isDetonated = true;
                }
              }
              sendGameState();
              drawStrikes();
              for (int ik = 0; ik < 4; ik++) drawSingleKeypad(ik);
              delay(200);
              break;
            }
          }
        }
      }

      // Check Terminal Outcome
      if (gameState.isDetonated) {
        setRGB(true, false, false);
        playExplosion();
        drawGameOverScreen(false);
      } else if (gameState.isDefused) {
        setRGB(false, true, false);
        playDisarmFanfare();
        drawGameOverScreen(true);
      }
    } else {
      // Game Over Screen - Play Again Tap
      if (ts.touched()) {
        TS_Point p = ts.getPoint();
        int tx = map(p.x, 240, 3800, 0, 320);
        int ty = map(p.y, 240, 3800, 0, 240);
        if (tx >= 60 && tx <= 260 && ty >= 150 && ty <= 200) {
          initNewGame();
          drawBombScreen();
          sendGameState();
          delay(300);
        }
      }
    }
  }

  // 3. Manual Unit Loop
  else if (currentRole == ROLE_MANUAL) {
    // Handle Page Switching
    if (ts.touched()) {
      TS_Point p = ts.getPoint();
      int tx = map(p.x, 240, 3800, 0, 320);
      int ty = map(p.y, 240, 3800, 0, 240);

      if (ty >= 38 && ty <= 64) {
        if (tx < 156 && manualTab != 0) {
          manualTab = 0;
          tone(SPEAKER_PIN, 1200, 30);
          drawManualScreen();
          delay(200);
        } else if (tx >= 164 && manualTab != 1) {
          manualTab = 1;
          tone(SPEAKER_PIN, 1200, 30);
          drawManualScreen();
          delay(200);
        }
      }
    }

    // Periodically refresh header timer from bomb broadcast
    if (now - lastTickTime >= 1000) {
      lastTickTime = now;
      tft.fillRect(240, 8, 70, 20, 0x0188);
      tft.setTextFont(2);
      tft.setTextColor(COL_YELLOW, 0x0188);
      tft.setCursor(240, 8);
      tft.printf("%02ds | %dX", gameState.secondsRemaining, gameState.strikes);
    }
  }

  delay(15);
}

