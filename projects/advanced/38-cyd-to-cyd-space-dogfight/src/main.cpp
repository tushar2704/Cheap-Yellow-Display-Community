/**
 * @file main.cpp
 * @brief CYD-to-CYD Space Dogfight Duel (Retro Arcade Head-to-Head Duel)
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Hardware:
 * - 2× Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Radio: ESP-NOW 2.4GHz Peer-to-Peer (sub-5ms cross-screen projectile physics)
 * - Audio: GPIO 26 Speaker / Piezo buzzer (laser zaps, shield hum, explosion rumble)
 * - RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) [Active LOW]
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

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define SPEAKER_PIN   26

// ---------------- Display & Touch ----------------
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
Preferences prefs;

// ---------------- Color Palette ----------------
#define COL_SPACE      0x0000  // Pure Black
#define COL_HEADER     0x1084  // Dark Space Gray
#define COL_P1_BLUE    0x05DF  // Cyan Blue Falcon
#define COL_P2_RED     0xF986  // Red Phoenix
#define COL_LASER_P1   0x07FF  // Plasma Cyan
#define COL_LASER_P2   0xF800  // Plasma Red
#define COL_SHIELD     0x07E0  // Emerald Shield
#define COL_EMP        0xFFE0  // Electric Yellow
#define COL_WHITE      0xFFFF  // White
#define COL_GRAY       0x7BEF  // Dim Gray

// ---------------- Packet Protocol ----------------
#define PKT_LASER      0x30
#define PKT_EMP        0x31
#define PKT_HIT        0x32
#define PKT_GAME_OVER  0x33
#define PKT_RESTART    0x34

struct __attribute__((packed)) DogfightPacket {
  uint8_t  type;
  uint8_t  playerId; // 1 or 2
  int16_t  x;        // Cross-screen coordinate
  int8_t   vx;       // Horizontal deflection velocity
  uint8_t  param;
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------- Game State ----------------
enum PlayerRole {
  PLAYER_1 = 1,
  PLAYER_2 = 2
};

PlayerRole myPlayerId = PLAYER_1;
enum GameState {
  STATE_PLAYING,
  STATE_VICTORY,
  STATE_DEFEATED
};

GameState currentGameState = STATE_PLAYING;

// Ship variables
int shipX = 160;
int prevShipX = 160;
const int shipY = 164;
const int shipW = 24;
const int shipH = 14;

int playerHp = 100;
int opponentHp = 100;
bool shieldActive = false;
unsigned long shieldEndTime = 0;
unsigned long shieldCooldownEnd = 0;
bool empStunned = false;
unsigned long empStunEndTime = 0;
unsigned long empCooldownEnd = 0;
unsigned long lastFireTime = 0;

// Starfield particles
#define STAR_COUNT 24
struct Star {
  int x, y;
  uint8_t speed;
};
Star stars[STAR_COUNT];

// Laser Projectiles
#define MAX_MY_LASERS 4
struct MyLaser {
  bool active;
  int x, y;
  int prevX, prevY;
};
MyLaser myLasers[MAX_MY_LASERS];

#define MAX_ENEMY_LASERS 6
struct EnemyLaser {
  bool active;
  bool isEmp;
  int x, y;
  int prevX, prevY;
};
EnemyLaser enemyLasers[MAX_ENEMY_LASERS];

unsigned long lastTouchTime = 0;

// ---------------- Audio & LED Effects ----------------
void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playLaserSound() {
  for (int f = 1900; f > 500; f -= 140) {
    tone(SPEAKER_PIN, f, 8);
    delay(9);
  }
}

void playHitSound() {
  tone(SPEAKER_PIN, 200, 80);
  delay(85);
  tone(SPEAKER_PIN, 120, 100);
}

void playShieldSound() {
  tone(SPEAKER_PIN, 900, 40);
  delay(45);
  tone(SPEAKER_PIN, 1400, 50);
}

void playEmpSound() {
  for (int i = 0; i < 4; i++) {
    tone(SPEAKER_PIN, 3000, 15);
    delay(20);
    tone(SPEAKER_PIN, 2200, 15);
    delay(20);
  }
}

void playExplosionSound() {
  for (int f = 400; f > 60; f -= 25) {
    tone(SPEAKER_PIN, f, 25);
    delay(28);
  }
}

void playClickSound() {
  tone(SPEAKER_PIN, 1500, 12);
}

// ---------------- ESP-NOW Packet Transmit ----------------
void sendDogfightPacket(uint8_t type, int16_t x = 0, int8_t vx = 0, uint8_t param = 0) {
  DogfightPacket pkt;
  pkt.type = type;
  pkt.playerId = (uint8_t)myPlayerId;
  pkt.x = x;
  pkt.vx = vx;
  pkt.param = param;
  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
}

// ---------------- ESP-NOW Receive Callback ----------------
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(DogfightPacket)) return;

  DogfightPacket pkt;
  memcpy(&pkt, incomingData, sizeof(pkt));

  // Ignore packets from self
  if (pkt.playerId == (uint8_t)myPlayerId) return;

  if (pkt.type == PKT_LASER) {
    // Laser shot by opponent exits their top screen and enters our top screen
    for (int i = 0; i < MAX_ENEMY_LASERS; i++) {
      if (!enemyLasers[i].active) {
        enemyLasers[i].active = true;
        enemyLasers[i].isEmp = false;
        // Invert X coordinate so relative aim matches physical head-to-head facing
        enemyLasers[i].x = constrain(320 - pkt.x, 10, 310);
        enemyLasers[i].y = 28;
        enemyLasers[i].prevX = enemyLasers[i].x;
        enemyLasers[i].prevY = enemyLasers[i].y;
        break;
      }
    }
  } else if (pkt.type == PKT_EMP) {
    // Incoming EMP projectile
    for (int i = 0; i < MAX_ENEMY_LASERS; i++) {
      if (!enemyLasers[i].active) {
        enemyLasers[i].active = true;
        enemyLasers[i].isEmp = true;
        enemyLasers[i].x = constrain(320 - pkt.x, 10, 310);
        enemyLasers[i].y = 28;
        enemyLasers[i].prevX = enemyLasers[i].x;
        enemyLasers[i].prevY = enemyLasers[i].y;
        break;
      }
    }
  } else if (pkt.type == PKT_HIT) {
    // Opponent tells us they took damage -> Update opponent HP HUD
    opponentHp = pkt.param;
    if (opponentHp == 0) {
      currentGameState = STATE_VICTORY;
      setRgbLed(false, true, false); // Green
      playExplosionSound();
    }
  } else if (pkt.type == PKT_GAME_OVER) {
    // Opponent signaled game over
    currentGameState = STATE_VICTORY;
    setRgbLed(false, true, false);
    playExplosionSound();
  } else if (pkt.type == PKT_RESTART) {
    // Reset duel
    playerHp = 100;
    opponentHp = 100;
    currentGameState = STATE_PLAYING;
    tft.fillScreen(COL_SPACE);
  }
}

// ---------------- GUI Drawing ----------------
void drawHeader() {
  tft.fillRect(0, 0, 320, 24, COL_HEADER);
  tft.drawFastHLine(0, 24, 320, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);

  // Player Role & Name
  tft.setTextSize(1);
  if (myPlayerId == PLAYER_1) {
    tft.setTextColor(COL_P1_BLUE, COL_HEADER);
    tft.setCursor(6, 8);
    tft.print("[P1: BLUE FALCON]");
  } else {
    tft.setTextColor(COL_P2_RED, COL_HEADER);
    tft.setCursor(6, 8);
    tft.print("[P2: RED PHOENIX]");
  }

  // Health Bars: My HP (Left) vs Enemy HP (Right)
  tft.setTextColor(COL_WHITE, COL_HEADER);
  tft.setCursor(120, 8);
  tft.printf("HP:%3d%%", playerHp);

  // My HP Bar
  tft.drawRect(162, 7, 50, 10, COL_WHITE);
  tft.fillRect(163, 8, 48, 8, COL_SPACE);
  int myBarW = map(playerHp, 0, 100, 0, 48);
  tft.fillRect(163, 8, constrain(myBarW, 0, 48), 8, (playerHp > 30) ? COL_SHIELD : COL_P2_RED);

  // Enemy HP Bar
  tft.setCursor(218, 8);
  tft.printf("FOE:%3d%%", opponentHp);
  tft.drawRect(268, 7, 48, 10, COL_WHITE);
  tft.fillRect(269, 8, 46, 8, COL_SPACE);
  int enemyBarW = map(opponentHp, 0, 100, 0, 46);
  tft.fillRect(269, 8, constrain(enemyBarW, 0, 46), 8, (opponentHp > 30) ? COL_SHIELD : COL_P2_RED);
}

void drawControls() {
  // Steering Control Lane (Y: 184 to 239)
  tft.fillRect(0, 184, 320, 56, 0x0821);
  tft.drawFastHLine(0, 184, 320, COL_HEADER);

  // Left Arrow
  tft.fillRoundRect(6, 188, 52, 46, 4, 0x18E3);
  tft.drawRoundRect(6, 188, 52, 46, 4, COL_WHITE);
  tft.setTextColor(COL_WHITE, 0x18E3);
  tft.setTextSize(2);
  tft.setCursor(24, 202);
  tft.print("<");

  // Right Arrow
  tft.fillRoundRect(64, 188, 52, 46, 4, 0x18E3);
  tft.drawRoundRect(64, 188, 52, 46, 4, COL_WHITE);
  tft.setTextColor(COL_WHITE, 0x18E3);
  tft.setTextSize(2);
  tft.setCursor(82, 202);
  tft.print(">");

  // SHIELD Button
  bool shieldReady = (millis() >= shieldCooldownEnd);
  tft.fillRoundRect(122, 188, 58, 46, 4, shieldReady ? 0x03E0 : 0x2104);
  tft.drawRoundRect(122, 188, 58, 46, 4, COL_WHITE);
  tft.setTextColor(COL_WHITE, shieldReady ? 0x03E0 : 0x2104);
  tft.setTextSize(1);
  tft.setCursor(132, 200);
  tft.print("SHIELD");
  tft.setCursor(134, 214);
  tft.print(shieldActive ? "ACTIVE" : (shieldReady ? "READY" : "WAIT"));

  // EMP Button
  bool empReady = (millis() >= empCooldownEnd);
  tft.fillRoundRect(186, 188, 58, 46, 4, empReady ? 0x94A0 : 0x2104);
  tft.drawRoundRect(186, 188, 58, 46, 4, COL_WHITE);
  tft.setTextColor(COL_WHITE, empReady ? 0x94A0 : 0x2104);
  tft.setTextSize(1);
  tft.setCursor(200, 200);
  tft.print("EMP");
  tft.setCursor(196, 214);
  tft.print(empReady ? "READY" : "WAIT");

  // FIRE Button
  tft.fillRoundRect(250, 188, 64, 46, 4, (myPlayerId == PLAYER_1) ? 0x041F : 0x8800);
  tft.drawRoundRect(250, 188, 64, 46, 4, COL_WHITE);
  tft.setTextColor(COL_WHITE, (myPlayerId == PLAYER_1) ? 0x041F : 0x8800);
  tft.setTextSize(2);
  tft.setCursor(264, 202);
  tft.print("FIRE");
}

void drawShip(int x, uint16_t color) {
  // Modern retro triangle vector fighter
  tft.fillTriangle(x, shipY - 10, x - shipW / 2, shipY + shipH / 2, x + shipW / 2, shipY + shipH / 2, color);
  tft.drawTriangle(x, shipY - 10, x - shipW / 2, shipY + shipH / 2, x + shipW / 2, shipY + shipH / 2, COL_WHITE);
  // Cockpit
  tft.fillCircle(x, shipY, 3, COL_WHITE);

  // If shield active, draw glowing bubble around ship
  if (shieldActive) {
    tft.drawCircle(x, shipY, shipW / 2 + 6, COL_SHIELD);
    tft.drawCircle(x, shipY, shipW / 2 + 7, COL_EMP);
  }
}

void eraseShip(int x) {
  tft.fillRect(x - shipW / 2 - 8, shipY - 12, shipW + 16, shipH + 16, COL_SPACE);
}

void initStars() {
  for (int i = 0; i < STAR_COUNT; i++) {
    stars[i].x = random(4, 316);
    stars[i].y = random(26, 180);
    stars[i].speed = random(1, 3);
  }
}

void updateStars() {
  for (int i = 0; i < STAR_COUNT; i++) {
    tft.drawPixel(stars[i].x, stars[i].y, COL_SPACE); // erase
    stars[i].y += stars[i].speed;
    if (stars[i].y >= 180) {
      stars[i].y = 26;
      stars[i].x = random(4, 316);
    }
    tft.drawPixel(stars[i].x, stars[i].y, (stars[i].speed == 2) ? COL_WHITE : COL_GRAY);
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=======================================================");
  Serial.println("  CYD-to-CYD Space Dogfight Duel - Project 38");
  Serial.println("  Retro Arcade Cross-Screen Wireless Combat");
  Serial.println("  Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("  Channels: @IamNotIntoTech | @Makerrdesigns");
  Serial.println("=======================================================\n");

  // Read saved player ID or default to P1
  prefs.begin("cyd_dogfight", false);
  myPlayerId = (PlayerRole)prefs.getUChar("player_id", PLAYER_1);

  // Pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(myPlayerId == PLAYER_2, false, myPlayerId == PLAYER_1);

  pinMode(SPEAKER_PIN, OUTPUT);

  // Display Init
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COL_SPACE);

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

  // Stars
  initStars();

  // Reset lasers
  for (int i = 0; i < MAX_MY_LASERS; i++) myLasers[i].active = false;
  for (int i = 0; i < MAX_ENEMY_LASERS; i++) enemyLasers[i].active = false;

  // Draw UI
  drawHeader();
  drawControls();
  drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);

  playLaserSound();
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // Handle EMP Stun cooldown
  if (empStunned && now >= empStunEndTime) {
    empStunned = false;
  }

  // Handle Shield timeout
  if (shieldActive && now >= shieldEndTime) {
    shieldActive = false;
    eraseShip(shipX);
    drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);
    drawControls();
  }

  // 1. TOUCH CONTROLS
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Top Bar Tap: Switch Player 1 <-> Player 2
      if (screenY <= 24 && screenX <= 110) {
        if (now - lastTouchTime > 400) {
          lastTouchTime = now;
          playClickSound();
          myPlayerId = (myPlayerId == PLAYER_1) ? PLAYER_2 : PLAYER_1;
          prefs.putUChar("player_id", (uint8_t)myPlayerId);
          setRgbLed(myPlayerId == PLAYER_2, false, myPlayerId == PLAYER_1);
          drawHeader();
          drawControls();
          eraseShip(shipX);
          drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);
          return;
        }
      }

      // If in VICTORY or DEFEATED state: tap anywhere to restart
      if (currentGameState != STATE_PLAYING) {
        if (now - lastTouchTime > 500) {
          lastTouchTime = now;
          sendDogfightPacket(PKT_RESTART);
          playerHp = 100;
          opponentHp = 100;
          currentGameState = STATE_PLAYING;
          tft.fillScreen(COL_SPACE);
          drawHeader();
          drawControls();
          drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);
          return;
        }
      }

      // Control Buttons (Y >= 184)
      if (currentGameState == STATE_PLAYING && !empStunned) {
        // Left Button
        if (screenX >= 6 && screenX <= 58 && screenY >= 188) {
          if (shipX > 24) {
            eraseShip(shipX);
            shipX -= 8;
            drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);
          }
        }
        // Right Button
        else if (screenX >= 64 && screenX <= 116 && screenY >= 188) {
          if (shipX < 296) {
            eraseShip(shipX);
            shipX += 8;
            drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);
          }
        }
        // Direct Touch Steering in Ship Lane
        else if (screenY >= 130 && screenY <= 180) {
          eraseShip(shipX);
          shipX = constrain(screenX, 24, 296);
          drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);
        }
        // Shield Button
        else if (screenX >= 122 && screenX <= 180 && screenY >= 188) {
          if (now >= shieldCooldownEnd && !shieldActive) {
            shieldActive = true;
            shieldEndTime = now + 3000;      // 3 seconds active
            shieldCooldownEnd = now + 9000;  // 9s cooldown
            playShieldSound();
            drawControls();
            drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);
          }
        }
        // EMP Button
        else if (screenX >= 186 && screenX <= 244 && screenY >= 188) {
          if (now >= empCooldownEnd) {
            empCooldownEnd = now + 12000; // 12s cooldown
            playEmpSound();
            drawControls();
            // Spawn outgoing EMP bolt
            for (int i = 0; i < MAX_MY_LASERS; i++) {
              if (!myLasers[i].active) {
                myLasers[i].active = true;
                myLasers[i].x = shipX;
                myLasers[i].y = shipY - 14;
                myLasers[i].prevX = myLasers[i].x;
                myLasers[i].prevY = myLasers[i].y;
                break;
              }
            }
          }
        }
        // Fire Button
        else if (screenX >= 250 && screenX <= 316 && screenY >= 188) {
          if (now - lastFireTime > 220) {
            lastFireTime = now;
            playLaserSound();
            for (int i = 0; i < MAX_MY_LASERS; i++) {
              if (!myLasers[i].active) {
                myLasers[i].active = true;
                myLasers[i].x = shipX;
                myLasers[i].y = shipY - 14;
                myLasers[i].prevX = myLasers[i].x;
                myLasers[i].prevY = myLasers[i].y;
                break;
              }
            }
          }
        }
      }
    }
  }

  // 2. GAME OVER OVERLAYS
  if (currentGameState == STATE_VICTORY) {
    tft.fillRect(40, 70, 240, 80, 0x03E0);
    tft.drawRect(40, 70, 240, 80, COL_WHITE);
    tft.setTextColor(COL_WHITE, 0x03E0);
    tft.setTextSize(2);
    tft.setCursor(68, 86);
    tft.print("VICTORY!!!");
    tft.setTextSize(1);
    tft.setCursor(60, 114);
    tft.print("ENEMY SHIP DESTROYED! [REMATCH]");
    delay(50);
    return;
  } else if (currentGameState == STATE_DEFEATED) {
    tft.fillRect(40, 70, 240, 80, 0x7800);
    tft.drawRect(40, 70, 240, 80, COL_WHITE);
    tft.setTextColor(COL_WHITE, 0x7800);
    tft.setTextSize(2);
    tft.setCursor(68, 86);
    tft.print("DEFEATED!");
    tft.setTextSize(1);
    tft.setCursor(60, 114);
    tft.print("YOUR HULL WAS DESTROYED! [RETRY]");
    delay(50);
    return;
  }

  // 3. ANIMATION: Starfield
  updateStars();

  // 4. ANIMATION: Move Outgoing Lasers (Traveling UP)
  for (int i = 0; i < MAX_MY_LASERS; i++) {
    if (myLasers[i].active) {
      // Erase previous position
      tft.fillRect(myLasers[i].prevX - 1, myLasers[i].prevY, 3, 8, COL_SPACE);

      myLasers[i].y -= 9; // Move up

      // Check if laser exits top of screen -> Transmit to opponent's CYD!
      if (myLasers[i].y <= 26) {
        myLasers[i].active = false;
        // Transmit cross-screen packet over ESP-NOW
        sendDogfightPacket(PKT_LASER, myLasers[i].x);
      } else {
        myLasers[i].prevX = myLasers[i].x;
        myLasers[i].prevY = myLasers[i].y;
        tft.fillRect(myLasers[i].x - 1, myLasers[i].y, 3, 8, (myPlayerId == PLAYER_1) ? COL_LASER_P1 : COL_LASER_P2);
      }
    }
  }

  // 5. ANIMATION: Move Incoming Enemy Lasers (Traveling DOWN)
  for (int i = 0; i < MAX_ENEMY_LASERS; i++) {
    if (enemyLasers[i].active) {
      // Erase previous position
      tft.fillRect(enemyLasers[i].prevX - 1, enemyLasers[i].prevY, 3, 8, COL_SPACE);

      enemyLasers[i].y += 7; // Move down

      // Check collision with Player Ship
      if (enemyLasers[i].y >= shipY - 10 && enemyLasers[i].y <= shipY + shipH / 2 &&
          abs(enemyLasers[i].x - shipX) <= shipW / 2 + 3) {
        enemyLasers[i].active = false;

        if (shieldActive) {
          // Shield absorbs the blow!
          shieldActive = false;
          playShieldSound();
          eraseShip(shipX);
          drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);
          drawControls();
        } else if (enemyLasers[i].isEmp) {
          // Stunned by EMP
          empStunned = true;
          empStunEndTime = now + 2500;
          playEmpSound();
        } else {
          // Direct Hit! Take 20 Damage
          playerHp = max(0, playerHp - 20);
          playHitSound();
          setRgbLed(true, false, false); // Flash Red

          // Notify opponent of our new HP
          sendDogfightPacket(PKT_HIT, 0, 0, (uint8_t)playerHp);
          drawHeader();

          // Screen shake & flash
          tft.fillRect(0, 25, 320, 158, 0x3800);
          delay(25);
          tft.fillRect(0, 25, 320, 158, COL_SPACE);
          drawShip(shipX, (myPlayerId == PLAYER_1) ? COL_P1_BLUE : COL_P2_RED);
          setRgbLed(myPlayerId == PLAYER_2, false, myPlayerId == PLAYER_1);

          if (playerHp == 0) {
            currentGameState = STATE_DEFEATED;
            sendDogfightPacket(PKT_GAME_OVER);
            playExplosionSound();
          }
        }
      } else if (enemyLasers[i].y >= 180) {
        // Laser missed and flew past bottom of combat zone
        enemyLasers[i].active = false;
      } else {
        enemyLasers[i].prevX = enemyLasers[i].x;
        enemyLasers[i].prevY = enemyLasers[i].y;
        uint16_t col = enemyLasers[i].isEmp ? COL_EMP : ((myPlayerId == PLAYER_1) ? COL_LASER_P2 : COL_LASER_P1);
        tft.fillRect(enemyLasers[i].x - 1, enemyLasers[i].y, 3, 8, col);
      }
    }
  }

  delay(14);
}

