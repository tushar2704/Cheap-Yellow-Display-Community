/**
 * @file 08-nes-game-emulator.ino
 * @brief NES 8-Bit Retro Gaming Arcade Console for Cheap Yellow Display 2.8" (ESP32-2432S028R)
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

// Arcade Engine Constants
#define GAME_W 220
#define GAME_H 180
#define GAME_X 8
#define GAME_Y 34

// Player Spaceship
int playerX = GAME_X + (GAME_W / 2) - 8;
const int playerY = GAME_Y + GAME_H - 14;
int playerLives = 3;
int score = 0;
int highScore = 2840;

// Laser Projectiles
#define MAX_BULLETS 4
struct Bullet {
  int x, y;
  bool active;
};
Bullet bullets[MAX_BULLETS];

// Alien Invaders
#define ALIEN_COLS 6
#define ALIEN_ROWS 3
struct Alien {
  int x, y;
  bool alive;
  uint16_t color;
};
Alien aliens[ALIEN_COLS * ALIEN_ROWS];
int alienDirection = 1;
int alienStepTimer = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void soundLaser() {
  tone(SPEAKER_PIN, 1800, 20);
  delay(15);
  tone(SPEAKER_PIN, 1200, 20);
}

void soundExplosion() {
  setRgb(true, false, false);
  for (int f = 300; f > 100; f -= 40) {
    tone(SPEAKER_PIN, f, 25);
    delay(20);
  }
  setRgb(false, true, false);
}

void soundBonus() {
  setRgb(false, true, true);
  tone(SPEAKER_PIN, 1200, 50);
  delay(55);
  tone(SPEAKER_PIN, 1800, 60);
  delay(65);
  tone(SPEAKER_PIN, 2400, 80);
}

void initGame() {
  playerX = GAME_X + (GAME_W / 2) - 8;
  playerLives = 3;
  score = 0;

  for (int i = 0; i < MAX_BULLETS; i++) {
    bullets[i].active = false;
  }

  // Spawn Alien Matrix
  int idx = 0;
  for (int r = 0; r < ALIEN_ROWS; r++) {
    for (int c = 0; c < ALIEN_COLS; c++) {
      aliens[idx].x = GAME_X + 18 + (c * 28);
      aliens[idx].y = GAME_Y + 16 + (r * 20);
      aliens[idx].alive = true;
      aliens[idx].color = (r == 0) ? 0xF800 : (r == 1) ? 0xFDC0 : 0x07E0;
      idx++;
    }
  }
}

void drawArcadeBezel() {
  tft.fillScreen(0x0000);

  // Top Arcade Marquee
  tft.fillRect(0, 0, 320, 30, 0x18A2);
  tft.drawFastHLine(0, 30, 320, 0xFDC0);

  tft.setTextColor(0xFDC0, 0x18A2);
  tft.setTextSize(1);
  tft.setCursor(8, 10);
  tft.print("🕹️ NES CYD ARCADE — 1985 EDITION");

  // CRT Screen Border
  tft.drawRoundRect(GAME_X - 2, GAME_Y - 2, GAME_W + 4, GAME_H + 4, 4, 0x4208);

  // Right Side Touch Controller Pad
  // D-PAD LEFT Button (235, 42, 38, 38)
  tft.fillRoundRect(235, 42, 38, 38, 6, 0x2124);
  tft.drawRoundRect(235, 42, 38, 38, 6, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, 0x2124);
  tft.setTextSize(2);
  tft.setCursor(248, 52);
  tft.print("<");

  // D-PAD RIGHT Button (278, 42, 38, 38)
  tft.fillRoundRect(278, 42, 38, 38, 6, 0x2124);
  tft.drawRoundRect(278, 42, 38, 38, 6, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, 0x2124);
  tft.setCursor(291, 52);
  tft.print(">");

  // FIRE BUTTON A (245, 96, 62, 50)
  tft.fillRoundRect(245, 96, 62, 50, 8, 0xF800); // NES Red
  tft.drawRoundRect(245, 96, 62, 50, 8, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, 0xF800);
  tft.setTextSize(2);
  tft.setCursor(264, 110);
  tft.print("A");
  tft.setTextSize(1);
  tft.setCursor(256, 130);
  tft.print("FIRE");

  // RESET / RESTART BUTTON B (245, 156, 62, 36)
  tft.fillRoundRect(245, 156, 62, 36, 6, 0x0841);
  tft.drawRoundRect(245, 156, 62, 36, 6, 0x3186);
  tft.setTextColor(0x07E0, 0x0841);
  tft.setTextSize(1);
  tft.setCursor(254, 168);
  tft.print("RESET");

  // Score HUD
  tft.fillRect(GAME_X, GAME_Y - 14, GAME_W, 12, 0x0000);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(GAME_X + 4, GAME_Y - 12);
  tft.printf("1UP: %05d  HI: %05d  LIVES: %d", score, highScore, playerLives);

  // Footer Channel Bar
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
}

void fireBullet() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x = playerX + 6;
      bullets[i].y = playerY - 4;
      bullets[i].active = true;
      soundLaser();
      break;
    }
  }
}

void renderGame() {
  // Clear Active Play Area
  tft.fillRect(GAME_X, GAME_Y, GAME_W, GAME_H, 0x0000);

  // Draw Stars (Subtle Retro Background)
  tft.drawPixel(GAME_X + 24, GAME_Y + 45, TFT_DARKGREY);
  tft.drawPixel(GAME_X + 90, GAME_Y + 110, TFT_DARKGREY);
  tft.drawPixel(GAME_X + 175, GAME_Y + 30, TFT_DARKGREY);
  tft.drawPixel(GAME_X + 140, GAME_Y + 140, TFT_DARKGREY);

  // Draw Player Spaceship (14x8 pixel retro sprite)
  tft.fillRoundRect(playerX, playerY + 4, 14, 5, 2, 0x07FF);
  tft.fillRect(playerX + 5, playerY, 4, 4, 0xFDC0);

  // Draw Bullets
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      tft.fillRect(bullets[i].x, bullets[i].y, 2, 5, 0xFDC0);
    }
  }

  // Draw Invaders
  for (int i = 0; i < ALIEN_COLS * ALIEN_ROWS; i++) {
    if (aliens[i].alive) {
      int ax = aliens[i].x;
      int ay = aliens[i].y;
      uint16_t col = aliens[i].color;
      // Retro 10x8 Invader Glyph
      tft.fillRoundRect(ax, ay, 12, 8, 2, col);
      tft.drawPixel(ax + 3, ay + 2, TFT_BLACK);
      tft.drawPixel(ax + 8, ay + 2, TFT_BLACK);
    }
  }
}

void updateGamePhysics() {
  // Update Bullets
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      bullets[i].y -= 6;
      if (bullets[i].y < GAME_Y) {
        bullets[i].active = false;
      } else {
        // Bullet vs Alien Collision Check
        for (int a = 0; a < ALIEN_COLS * ALIEN_ROWS; a++) {
          if (aliens[a].alive) {
            if (bullets[i].x >= aliens[a].x && bullets[i].x <= aliens[a].x + 12 &&
                bullets[i].y >= aliens[a].y && bullets[i].y <= aliens[a].y + 8) {
              aliens[a].alive = false;
              bullets[i].active = false;
              score += 100;
              if (score > highScore) highScore = score;
              soundExplosion();

              // Update Score HUD
              tft.fillRect(GAME_X, GAME_Y - 14, GAME_W, 12, 0x0000);
              tft.setTextColor(TFT_WHITE, 0x0000);
              tft.setTextSize(1);
              tft.setCursor(GAME_X + 4, GAME_Y - 12);
              tft.printf("1UP: %05d  HI: %05d  LIVES: %d", score, highScore, playerLives);
              break;
            }
          }
        }
      }
    }
  }

  // Update Alien March
  alienStepTimer++;
  if (alienStepTimer > 12) {
    alienStepTimer = 0;
    bool changeDir = false;
    for (int i = 0; i < ALIEN_COLS * ALIEN_ROWS; i++) {
      if (aliens[i].alive) {
        if ((alienDirection == 1 && aliens[i].x >= GAME_X + GAME_W - 20) ||
            (alienDirection == -1 && aliens[i].x <= GAME_X + 6)) {
          changeDir = true;
          break;
        }
      }
    }

    if (changeDir) {
      alienDirection = -alienDirection;
      for (int i = 0; i < ALIEN_COLS * ALIEN_ROWS; i++) {
        aliens[i].y += 4;
      }
    } else {
      for (int i = 0; i < ALIEN_COLS * ALIEN_ROWS; i++) {
        aliens[i].x += (alienDirection * 4);
      }
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

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  initGame();
  drawArcadeBezel();

  soundBonus();
  setRgb(false, true, false);
}

void loop() {
  // Touch Controller Read
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // D-PAD LEFT (235, 42, 38, 38)
    if (x >= 235 && x <= 273 && y >= 42 && y <= 80) {
      playerX = max(GAME_X + 2, playerX - 6);
    }
    // D-PAD RIGHT (278, 42, 38, 38)
    else if (x >= 278 && x <= 316 && y >= 42 && y <= 80) {
      playerX = min(GAME_X + GAME_W - 16, playerX + 6);
    }
    // FIRE A (245, 96, 62, 50)
    else if (x >= 245 && x <= 307 && y >= 96 && y <= 146) {
      fireBullet();
    }
    // RESET B (245, 156, 62, 36)
    else if (x >= 245 && x <= 307 && y >= 156 && y <= 192) {
      initGame();
      drawArcadeBezel();
      soundBonus();
      delay(200);
    }
  }

  updateGamePhysics();
  renderGame();
  delay(30);
}
