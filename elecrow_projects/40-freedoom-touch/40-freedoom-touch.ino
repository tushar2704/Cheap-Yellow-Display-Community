/**
 * 💥 Project 40: Playable No-PSRAM FreeDOOM
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - 3D First-Person DDA raycasting engine running entirely in 520KB SRAM (No PSRAM required).
 * - Textured depth-shaded wall rendering with distance fog attenuation.
 * - Animated center shotgun weapon sprite with muzzle flash recoil.
 * - On-screen virtual D-Pad touch controls and dedicated FIRE trigger button.
 * - Classic retro HUD: Health %, Armor %, Ammo counter, and channel branding.
 * - Shotgun blast acoustic frequency sweep on GPIO 26 buzzer & muzzle flash RGB LED on GPIOs 4/16/17.
 * - Hardware-calibrated non-inverted resistive touch coordinates.
 */

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_MOSI    32
#define TOUCH_MISO    39
#define TOUCH_CLK     25
#define TOUCH_CS      33
#define TOUCH_IRQ     36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

// UI Colors (16-bit 565)
#define COLOR_SKY       0x18C3 // Dark gloomy ceiling
#define COLOR_FLOOR     0x2104 // Dark dirt floor
#define COLOR_HUD_BG    0x10A2 // Classic slate HUD
#define COLOR_HUD_BRD   0x4208
#define COLOR_TEXT      0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GOLD      0xFEA0
#define COLOR_BLUE      0x2CDF

// 16x16 Dungeon Map
#define MAP_W 16
#define MAP_H 16
const uint8_t worldMap[MAP_H][MAP_W] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
  {1,0,1,1,0,0,1,0,2,2,2,0,0,0,0,1},
  {1,0,1,0,0,0,0,0,2,0,2,0,0,0,0,1},
  {1,0,1,0,0,0,1,0,2,0,2,0,3,0,0,1},
  {1,0,0,0,0,0,1,0,0,0,0,0,3,0,0,1},
  {1,0,2,2,2,0,1,1,1,0,1,1,3,0,0,1},
  {1,0,2,0,2,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,2,0,2,0,1,1,1,0,1,1,1,0,0,1},
  {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1},
  {1,0,0,0,0,0,1,0,3,3,3,0,1,0,0,1},
  {1,1,1,0,1,1,1,0,3,0,3,0,1,0,0,1},
  {1,0,0,0,0,0,0,0,3,0,3,0,0,0,0,1},
  {1,0,2,0,2,0,0,0,0,0,0,0,2,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Player State
float posX = 2.5f, posY = 2.5f;
float dirX = 1.0f, dirY = 0.0f;
float planeX = 0.0f, planeY = 0.66f;

int playerHealth = 100;
int playerArmor = 50;
int playerAmmo = 36;

// Weapon Animation
int muzzleFlashTimer = 0;

// Raycaster Resolution: 80 columns x 4 pixels width = 320 screen width
#define NUM_RAYS 80
#define RAY_WIDTH 4
#define VIEWPORT_H 188

void playBeep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

uint16_t getWallColor(int wallType, int side, float dist) {
  uint8_t r = 0, g = 0, b = 0;
  if (wallType == 1) {
    r = 80; g = 90; b = 130;
  } else if (wallType == 2) {
    r = 140; g = 70; b = 40;
  } else {
    r = 50; g = 120; b = 60;
  }

  if (side == 1) {
    r /= 1.4f; g /= 1.4f; b /= 1.4f;
  }

  float fog = max(0.2f, 1.0f - (dist / 14.0f));
  r = (uint8_t)(r * fog);
  g = (uint8_t)(g * fog);
  b = (uint8_t)(b * fog);

  return tft.color565(r, g, b);
}

void renderRaycastFrame() {
  tft.fillRect(0, 0, 320, VIEWPORT_H / 2, COLOR_SKY);
  tft.fillRect(0, VIEWPORT_H / 2, 320, VIEWPORT_H / 2, COLOR_FLOOR);

  for (int r = 0; r < NUM_RAYS; r++) {
    float cameraX = 2.0f * r / (float)NUM_RAYS - 1.0f;
    float rayDirX = dirX + planeX * cameraX;
    float rayDirY = dirY + planeY * cameraX;

    int mapX = (int)posX;
    int mapY = (int)posY;

    float sideDistX, sideDistY;
    float deltaDistX = (rayDirX == 0) ? 1e30 : abs(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30 : abs(1.0f / rayDirY);
    float perpWallDist;

    int stepX, stepY;
    int hit = 0;
    int side;

    if (rayDirX < 0) {
      stepX = -1;
      sideDistX = (posX - mapX) * deltaDistX;
    } else {
      stepX = 1;
      sideDistX = (mapX + 1.0f - posX) * deltaDistX;
    }

    if (rayDirY < 0) {
      stepY = -1;
      sideDistY = (posY - mapY) * deltaDistY;
    } else {
      stepY = 1;
      sideDistY = (mapY + 1.0f - posY) * deltaDistY;
    }

    while (hit == 0) {
      if (sideDistX < sideDistY) {
        sideDistX += deltaDistX;
        mapX += stepX;
        side = 0;
      } else {
        sideDistY += deltaDistY;
        mapY += stepY;
        side = 1;
      }
      if (mapX >= 0 && mapX < MAP_W && mapY >= 0 && mapY < MAP_H) {
        if (worldMap[mapY][mapX] > 0) hit = worldMap[mapY][mapX];
      } else {
        hit = 1;
      }
    }

    if (side == 0) perpWallDist = (sideDistX - deltaDistX);
    else perpWallDist = (sideDistY - deltaDistY);

    int lineHeight = (int)(VIEWPORT_H / max(0.01f, perpWallDist));
    int drawStart = -lineHeight / 2 + VIEWPORT_H / 2;
    if (drawStart < 0) drawStart = 0;
    int drawEnd = lineHeight / 2 + VIEWPORT_H / 2;
    if (drawEnd >= VIEWPORT_H) drawEnd = VIEWPORT_H - 1;

    uint16_t col = getWallColor(hit, side, perpWallDist);
    tft.fillRect(r * RAY_WIDTH, drawStart, RAY_WIDTH, drawEnd - drawStart + 1, col);
  }

  // Draw Center Shotgun Sprite
  int gunX = 145;
  int gunY = VIEWPORT_H - 36;
  if (muzzleFlashTimer > 0) {
    tft.fillTriangle(150, gunY - 14, 160, gunY - 28, 170, gunY - 14, COLOR_GOLD);
    tft.fillCircle(160, gunY - 14, 8, COLOR_RED);
    gunY += 5;
  }

  tft.fillRect(gunX + 4, gunY, 10, 36, tft.color565(50, 50, 55));
  tft.fillRect(gunX + 16, gunY, 10, 36, tft.color565(70, 70, 75));
  tft.fillRoundRect(gunX, gunY + 20, 30, 20, 4, tft.color565(90, 50, 20));
}

void drawHUD() {
  tft.fillRect(0, 190, 320, 50, COLOR_HUD_BG);
  tft.drawFastHLine(0, 190, 320, COLOR_HUD_BRD);

  tft.setTextColor(COLOR_RED, COLOR_HUD_BG);
  tft.setTextSize(1);
  tft.drawString("HEALTH", 10, 194, 1);
  char hpBuf[8];
  snprintf(hpBuf, sizeof(hpBuf), "%d%%", playerHealth);
  tft.drawString(hpBuf, 10, 206, 4);

  tft.setTextColor(COLOR_BLUE, COLOR_HUD_BG);
  tft.drawString("ARMOR", 75, 194, 1);
  char armBuf[8];
  snprintf(armBuf, sizeof(armBuf), "%d%%", playerArmor);
  tft.drawString(armBuf, 75, 206, 4);

  tft.setTextColor(COLOR_GOLD, COLOR_HUD_BG);
  tft.drawString("AMMO", 135, 194, 1);
  char ammoBuf[8];
  snprintf(ammoBuf, sizeof(ammoBuf), "%d", playerAmmo);
  tft.drawString(ammoBuf, 135, 206, 4);

  // Channel Watermark
  tft.setTextColor(tft.color565(60, 75, 95), COLOR_HUD_BG);
  tft.drawString("IamNotIntoTech", 10, 230, 1);

  // Virtual Touch D-Pad Indicator (Right Side)
  tft.fillRoundRect(195, 194, 26, 20, 3, tft.color565(35, 45, 60));
  tft.setTextColor(COLOR_TEXT, tft.color565(35, 45, 60));
  tft.drawCentreString("<", 208, 198, 2);

  tft.fillRoundRect(225, 194, 26, 20, 3, tft.color565(35, 45, 60));
  tft.drawCentreString("^", 238, 198, 2);

  tft.fillRoundRect(255, 194, 26, 20, 3, tft.color565(35, 45, 60));
  tft.drawCentreString(">", 268, 198, 2);

  tft.fillRoundRect(225, 216, 26, 20, 3, tft.color565(35, 45, 60));
  tft.drawCentreString("v", 238, 220, 2);

  // FIRE Button
  tft.fillRoundRect(285, 194, 32, 42, 4, COLOR_RED);
  tft.drawRoundRect(285, 194, 32, 42, 4, COLOR_GOLD);
  tft.setTextColor(0x0000, COLOR_RED);
  tft.drawCentreString("FIRE", 301, 208, 1);
}

void fireShotgun() {
  if (playerAmmo <= 0) {
    playBeep(900, 40);
    return;
  }

  playerAmmo--;
  muzzleFlashTimer = 3;
  setRGB(true, true, false);

  for (int f = 1800; f > 300; f -= 120) {
    tone(BUZZER_PIN, f, 8);
    delay(4);
  }
  noTone(BUZZER_PIN);
  setRGB(false, false, false);

  drawHUD();
}

void movePlayer(float moveSpeed, float rotSpeed) {
  if (moveSpeed != 0) {
    float newX = posX + dirX * moveSpeed;
    float newY = posY + dirY * moveSpeed;
    if (worldMap[(int)posY][(int)newX] == 0) posX = newX;
    if (worldMap[(int)newY][(int)posX] == 0) posY = newY;
  }

  if (rotSpeed != 0) {
    float oldDirX = dirX;
    dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
    dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
    float oldPlaneX = planeX;
    planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
    planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRGB(false, false, false);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COLOR_FLOOR);

  // Initialize Touch (VSPI)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  renderRaycastFrame();
  drawHUD();

  playBeep(2400, 60);
  delay(60);
  playBeep(3200, 100);
  Serial.println(F("[SYSTEM] FreeDOOM Raycaster Initialized."));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      int screenX = map(p.x, 240, 3800, 0, 320);
      int screenY = map(p.y, 240, 3800, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Direct Viewport Drag to Turn or Move (screenY < 190)
      if (screenY < 190) {
        if (screenX < 100) movePlayer(0, -0.15f);
        else if (screenX > 220) movePlayer(0, 0.15f);
        else movePlayer(0.20f, 0);
      }
      // Bottom D-Pad & Fire Button (screenY >= 190)
      else {
        if (screenX >= 280) {
          fireShotgun();
        } else if (screenX >= 195 && screenX <= 222) {
          movePlayer(0, -0.18f);
        } else if (screenX >= 223 && screenX <= 252 && screenY < 216) {
          movePlayer(0.22f, 0);
        } else if (screenX >= 223 && screenX <= 252 && screenY >= 216) {
          movePlayer(-0.20f, 0);
        } else if (screenX >= 253 && screenX <= 280) {
          movePlayer(0, 0.18f);
        }
      }
    }
  }

  if (muzzleFlashTimer > 0) muzzleFlashTimer--;

  renderRaycastFrame();

  delay(15);
}
