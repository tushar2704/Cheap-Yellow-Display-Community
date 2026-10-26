/**
 * @file main.cpp
 * @brief 33 - CYD-to-CYD Naval Battleship Tactical Combat
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Community Channels:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - Two ESP32-2432S028R Cheap Yellow Displays
 * - Protocol: Raw ESP-NOW 2.4GHz
 * - Sound: FM8002A (GPIO 26)
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

#define GRID_SIZE 8
#define CELL_SIZE 22

// Board States
#define CELL_EMPTY 0
#define CELL_SHIP  1
#define CELL_MISS  2
#define CELL_HIT   3

uint8_t myFleetGrid[GRID_SIZE][GRID_SIZE];
uint8_t enemyRadarGrid[GRID_SIZE][GRID_SIZE];

// Game Phases
enum GamePhase {
  PHASE_PLACEMENT = 0,
  PHASE_WAIT_PEER,
  PHASE_MY_TURN,
  PHASE_ENEMY_TURN,
  PHASE_VICTORY,
  PHASE_DEFEAT
};
GamePhase currentPhase = PHASE_PLACEMENT;

// Packet Definitions
#define BATTLE_PKT_READY  0x01
#define BATTLE_PKT_FIRE   0x02
#define BATTLE_PKT_RESULT 0x03
#define BATTLE_PKT_RESET  0x04

struct __attribute__((packed)) BattlePacket {
  uint8_t msgType;
  uint8_t row;
  uint8_t col;
  uint8_t result; // 0 = Miss, 1 = Hit, 2 = Sunk All
  uint8_t remainingShips;
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

bool peerReady = false;
int myShipHitsLeft = 10;
int enemyShipHitsLeft = 10;
int lastFiredR = -1;
int lastFiredC = -1;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int dur) {
  tone(SPEAKER_PIN, freq, dur);
}

void autoDeployShips() {
  memset(myFleetGrid, CELL_EMPTY, sizeof(myFleetGrid));
  // 1x 4-cell ship (Carrier)
  for (int c = 1; c <= 4; c++) myFleetGrid[1][c] = CELL_SHIP;
  // 1x 3-cell ship (Battleship)
  for (int r = 3; r <= 5; r++) myFleetGrid[r][6] = CELL_SHIP;
  // 1x 2-cell ship (Cruiser)
  myFleetGrid[6][2] = CELL_SHIP;
  myFleetGrid[6][3] = CELL_SHIP;
  // 1x 1-cell ship (Submarine)
  myFleetGrid[4][2] = CELL_SHIP;
  myShipHitsLeft = 10;
}

void sendBattlePacket(uint8_t type, uint8_t r, uint8_t c, uint8_t res, uint8_t rem) {
  BattlePacket pkt;
  pkt.msgType = type;
  pkt.row = r;
  pkt.col = c;
  pkt.result = res;
  pkt.remainingShips = rem;
  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
}

void drawGrid(int startX, int startY, uint8_t grid[GRID_SIZE][GRID_SIZE], bool isRadar) {
  for (int r = 0; r < GRID_SIZE; r++) {
    for (int c = 0; c < GRID_SIZE; c++) {
      int x = startX + c * CELL_SIZE;
      int y = startY + r * CELL_SIZE;

      uint16_t cellCol = 0x0821; // Dark Sea
      uint8_t val = grid[r][c];

      if (val == CELL_SHIP && !isRadar) cellCol = 0x39C7; // Navy Steel
      else if (val == CELL_MISS)        cellCol = 0x0410; // Dark Cyan Splash
      else if (val == CELL_HIT)         cellCol = 0xF800; // Red Hit / Fire

      tft.fillRect(x + 1, y + 1, CELL_SIZE - 2, CELL_SIZE - 2, cellCol);
      tft.drawRect(x, y, CELL_SIZE, CELL_SIZE, 0x18C3);

      if (val == CELL_MISS) {
        tft.fillCircle(x + (CELL_SIZE / 2), y + (CELL_SIZE / 2), 3, 0x07FF);
      } else if (val == CELL_HIT) {
        tft.drawLine(x + 3, y + 3, x + CELL_SIZE - 3, y + CELL_SIZE - 3, TFT_WHITE);
        tft.drawLine(x + CELL_SIZE - 3, y + 3, x + 3, y + CELL_SIZE - 3, TFT_WHITE);
      }
    }
  }
}

void renderScreen();

// Receive Callback
void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len == sizeof(BattlePacket)) {
    BattlePacket pkt;
    memcpy(&pkt, data, sizeof(pkt));

    if (pkt.msgType == BATTLE_PKT_READY) {
      peerReady = true;
      if (currentPhase == PHASE_WAIT_PEER) {
        currentPhase = PHASE_MY_TURN; // Player who was waiting goes first
        renderScreen();
      }
    } else if (pkt.msgType == BATTLE_PKT_FIRE) {
      // Enemy fired at our coordinates (pkt.row, pkt.col)
      uint8_t r = pkt.row;
      uint8_t c = pkt.col;
      uint8_t res = 0; // Miss

      if (myFleetGrid[r][c] == CELL_SHIP) {
        myFleetGrid[r][c] = CELL_HIT;
        myShipHitsLeft--;
        res = (myShipHitsLeft == 0) ? 2 : 1; // 2 = Sunk All, 1 = Hit

        setRgb(true, false, false);
        playBeep(400, 300);
      } else {
        myFleetGrid[r][c] = CELL_MISS;
        playBeep(1200, 50);
      }

      sendBattlePacket(BATTLE_PKT_RESULT, r, c, res, myShipHitsLeft);

      if (res == 2) {
        currentPhase = PHASE_DEFEAT;
      } else {
        currentPhase = PHASE_MY_TURN; // Our turn to fire!
      }
      renderScreen();
    } else if (pkt.msgType == BATTLE_PKT_RESULT) {
      // Received response for our previous fire
      if (lastFiredR >= 0 && lastFiredC >= 0) {
        if (pkt.result == 1 || pkt.result == 2) {
          enemyRadarGrid[lastFiredR][lastFiredC] = CELL_HIT;
          enemyShipHitsLeft = pkt.remainingShips;
          setRgb(false, true, false);
          playBeep(1800, 100);
        } else {
          enemyRadarGrid[lastFiredR][lastFiredC] = CELL_MISS;
          playBeep(600, 60);
        }
      }

      if (pkt.result == 2) {
        currentPhase = PHASE_VICTORY;
      } else {
        currentPhase = PHASE_ENEMY_TURN;
      }
      renderScreen();
    }
  }
}

void renderScreen() {
  tft.fillRect(0, 0, 320, 240, 0x0821);

  // Header Bar
  tft.fillRect(0, 0, 320, 24, 0x1082);
  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(1);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("CYD NAVAL BATTLESHIP TACTICAL", 8, 12);

  // Status Badge
  const char* phaseNames[] = {"DEPLOY FLEET", "WAITING FOR PEER", "YOUR TURN (FIRE)", "ENEMY FIRING...", "VICTORY!", "DEFEAT!"};
  uint16_t phaseColors[] = {0xFDC0, 0x39C7, 0x07E0, 0xF800, 0x07E0, 0xF800};
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(phaseColors[currentPhase], 0x1082);
  tft.drawString(phaseNames[currentPhase], 312, 12);

  if (currentPhase == PHASE_PLACEMENT) {
    // Placement Screen
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.drawString("Position fleet or tap 'AUTO DEPLOY':", 12, 32);

    drawGrid(12, 50, myFleetGrid, false);

    // Auto Deploy Button
    tft.fillRoundRect(204, 60, 106, 38, 4, 0x18C3);
    tft.drawRoundRect(204, 60, 106, 38, 4, 0x39C7);
    tft.setTextColor(TFT_CYAN, 0x18C3);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("AUTO DEPLOY", 257, 79);

    // Lock & Ready Button
    tft.fillRoundRect(204, 116, 106, 42, 6, 0x07E0);
    tft.setTextColor(TFT_BLACK, 0x07E0);
    tft.drawString("LOCK & READY", 257, 137);

    // Fleet Specs
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_LIGHTGREY, 0x0821);
    tft.drawString("Ships: Carrier (4)", 204, 170);
    tft.drawString("Battleship (3)", 204, 184);
    tft.drawString("Cruiser (2) | Sub (1)", 204, 198);
  } else if (currentPhase == PHASE_WAIT_PEER) {
    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("WAITING FOR PARTNER...", 160, 100);
    tft.setTextSize(1);
    tft.setTextColor(0xFDC0, 0x0821);
    tft.drawString("Ensure partner CYD locks their fleet.", 160, 130);
  } else if (currentPhase == PHASE_MY_TURN || currentPhase == PHASE_ENEMY_TURN) {
    // Combat Screen: Enemy Radar Grid on Left, Your Fleet on Right
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(0x07FF, 0x0821);
    tft.drawString("ENEMY RADAR (TAP CELL TO FIRE)", 12, 30);
    drawGrid(12, 46, enemyRadarGrid, true);

    // Mini Your Fleet Status
    tft.setTextColor(0xFDC0, 0x0821);
    tft.drawString("YOUR FLEET STATUS", 204, 30);
    // Draw mini grid (half scale, 12px)
    for (int r = 0; r < GRID_SIZE; r++) {
      for (int c = 0; c < GRID_SIZE; c++) {
        int x = 204 + c * 13;
        int y = 46 + r * 13;
        uint16_t col = (myFleetGrid[r][c] == CELL_HIT) ? 0xF800 : (myFleetGrid[r][c] == CELL_SHIP ? 0x07E0 : 0x18C3);
        tft.fillRect(x, y, 11, 11, col);
      }
    }

    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.drawString("Hits Remaining: " + String(myShipHitsLeft), 204, 160);
    tft.drawString("Enemy Hits Left: " + String(enemyShipHitsLeft), 204, 176);

    tft.fillRoundRect(204, 200, 106, 26, 4, currentPhase == PHASE_MY_TURN ? 0x07E0 : 0x2104);
    tft.setTextColor(currentPhase == PHASE_MY_TURN ? TFT_BLACK : TFT_LIGHTGREY, currentPhase == PHASE_MY_TURN ? 0x07E0 : 0x2104);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(currentPhase == PHASE_MY_TURN ? "YOUR TURN" : "WAIT ENEMY", 257, 213);
  } else if (currentPhase == PHASE_VICTORY || currentPhase == PHASE_DEFEAT) {
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    tft.setTextColor(currentPhase == PHASE_VICTORY ? 0x07E0 : 0xF800, 0x0821);
    tft.drawString(currentPhase == PHASE_VICTORY ? "VICTORY!" : "DEFEAT!", 160, 90);

    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, 0x0821);
    tft.drawString(currentPhase == PHASE_VICTORY ? "You sank the entire enemy naval fleet!" : "All your naval ships have been sunk.", 160, 130);

    tft.fillRoundRect(100, 160, 120, 36, 6, 0xFDC0);
    tft.setTextColor(TFT_BLACK, 0xFDC0);
    tft.drawString("PLAY AGAIN", 160, 178);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[CYD-33] CYD Naval Battleship Initializing...");

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

  autoDeployShips();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_now_init();
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  renderScreen();

  playBeep(1200, 70);
  delay(90);
  playBeep(1800, 90);
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    if (currentPhase == PHASE_PLACEMENT) {
      // Auto Deploy (204, 60, 106, 38)
      if (x >= 204 && x <= 310 && y >= 60 && y <= 98) {
        autoDeployShips();
        playBeep(1600, 30);
        renderScreen();
        delay(200);
      }
      // Lock & Ready (204, 116, 106, 42)
      else if (x >= 204 && x <= 310 && y >= 116 && y <= 158) {
        sendBattlePacket(BATTLE_PKT_READY, 0, 0, 0, myShipHitsLeft);
        currentPhase = peerReady ? PHASE_MY_TURN : PHASE_WAIT_PEER;
        playBeep(2200, 60);
        renderScreen();
        delay(200);
      }
    } else if (currentPhase == PHASE_MY_TURN) {
      // Tap on Enemy Radar Grid (x: 12 to 188, y: 46 to 222)
      if (x >= 12 && x < 12 + (GRID_SIZE * CELL_SIZE) && y >= 46 && y < 46 + (GRID_SIZE * CELL_SIZE)) {
        int c = (x - 12) / CELL_SIZE;
        int r = (y - 46) / CELL_SIZE;

        if (enemyRadarGrid[r][c] == CELL_EMPTY) {
          lastFiredR = r;
          lastFiredC = c;
          sendBattlePacket(BATTLE_PKT_FIRE, r, c, 0, myShipHitsLeft);
          playBeep(1500, 40);
          delay(200);
        }
      }
    } else if (currentPhase == PHASE_VICTORY || currentPhase == PHASE_DEFEAT) {
      // Play Again (100, 160, 120, 36)
      if (x >= 100 && x <= 220 && y >= 160 && y <= 196) {
        memset(enemyRadarGrid, CELL_EMPTY, sizeof(enemyRadarGrid));
        autoDeployShips();
        peerReady = false;
        currentPhase = PHASE_PLACEMENT;
        renderScreen();
        delay(200);
      }
    }
  }

  delay(20);
}
