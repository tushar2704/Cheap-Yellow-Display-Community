/**
 * @file main.cpp
 * @brief 30 - CYD-to-CYD Cross-Screen Ball Physics & Air Pong
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Community Channels:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - Two ESP32-2432S028R Cheap Yellow Displays
 * - Wireless Protocol: ESP-NOW (Sub-5ms latency, broadcast pairing)
 * - Speakers: FM8002A (GPIO 26)
 * - RGB LEDs: GPIO 4, 16, 17
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

// Roles: Left CYD (Player 1) or Right CYD (Player 2)
enum ScreenRole {
  ROLE_LEFT_P1 = 0,
  ROLE_RIGHT_P2 = 1
};
ScreenRole myRole = ROLE_LEFT_P1;

// ESP-NOW Packet
#define PKT_BALL_CROSS 0x01
#define PKT_SCORE_SYNC 0x02
#define PKT_HEARTBEAT  0x03

struct __attribute__((packed)) PongPacket {
  uint8_t msgType;
  float y;
  float vx;
  float vy;
  uint16_t scoreP1;
  uint16_t scoreP2;
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Game State
bool hasBall = true;
float ballX = 160.0f;
float ballY = 120.0f;
float ballVx = 3.8f;
float ballVy = 2.4f;
float prevBallX = 160.0f;
float prevBallY = 120.0f;
const int ballRadius = 5;

// Paddle State
float paddleY = 120.0f;
float prevPaddleY = 120.0f;
const int paddleWidth = 8;
const int paddleHeight = 46;

// Scores
uint16_t scoreP1 = 0;
uint16_t scoreP2 = 0;
bool soloPracticeMode = false;
unsigned long lastPeerHeartbeat = 0;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int dur) {
  tone(SPEAKER_PIN, freq, dur);
}

// Send ESP-NOW Packet
void sendPongPacket(uint8_t type, float y, float vx, float vy) {
  PongPacket pkt;
  pkt.msgType = type;
  pkt.y = y;
  pkt.vx = vx;
  pkt.vy = vy;
  pkt.scoreP1 = scoreP1;
  pkt.scoreP2 = scoreP2;
  esp_now_send(broadcastMac, (uint8_t *)&pkt, sizeof(pkt));
}

// ESP-NOW Receive Callback
void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len == sizeof(PongPacket)) {
    PongPacket pkt;
    memcpy(&pkt, data, sizeof(pkt));
    lastPeerHeartbeat = millis();

    if (pkt.msgType == PKT_BALL_CROSS) {
      scoreP1 = pkt.scoreP1;
      scoreP2 = pkt.scoreP2;

      // Ball entered our screen
      if (myRole == ROLE_LEFT_P1) {
        // Entering from right edge
        ballX = 310.0f;
        ballY = pkt.y;
        ballVx = -abs(pkt.vx); // moving left
        ballVy = pkt.vy;
      } else {
        // Entering from left edge
        ballX = 8.0f;
        ballY = pkt.y;
        ballVx = abs(pkt.vx);  // moving right
        ballVy = pkt.vy;
      }
      hasBall = true;
      playBeep(900, 30);
    } else if (pkt.msgType == PKT_SCORE_SYNC) {
      scoreP1 = pkt.scoreP1;
      scoreP2 = pkt.scoreP2;
    }
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, 0x1082); // Dark Navy Bar

  // Role Toggle Button
  tft.fillRoundRect(4, 2, 86, 20, 3, 0xFDC0);
  tft.setTextColor(TFT_BLACK, 0xFDC0);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(myRole == ROLE_LEFT_P1 ? "ROLE: P1 [L]" : "ROLE: P2 [R]", 47, 12);

  // Scoreboard
  char scoreStr[32];
  snprintf(scoreStr, sizeof(scoreStr), "P1: %d  |  P2: %d", scoreP1, scoreP2);
  tft.setTextColor(TFT_WHITE, 0x1082);
  tft.setTextSize(1);
  tft.drawString(scoreStr, 170, 12);

  // Mode Indicator
  bool partnerOnline = (millis() - lastPeerHeartbeat < 4000);
  tft.fillRoundRect(244, 2, 72, 20, 3, partnerOnline ? 0x07E0 : 0x2104);
  tft.setTextColor(partnerOnline ? TFT_BLACK : TFT_LIGHTGREY, partnerOnline ? 0x07E0 : 0x2104);
  tft.drawString(partnerOnline ? "LINKED" : "SOLO / WAIT", 280, 12);
}

void resetBall() {
  ballX = (myRole == ROLE_LEFT_P1) ? 60.0f : 260.0f;
  ballY = 120.0f;
  ballVx = (myRole == ROLE_LEFT_P1) ? 3.5f : -3.5f;
  ballVy = 2.0f;
  hasBall = true;
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[CYD-30] CYD-to-CYD Cross-Screen Pong Initializing...");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  // Initialize Display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  // Initialize Touchscreen
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Initialize Wi-Fi in Station Mode & ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
  }
  esp_now_register_recv_cb(onDataRecv);

  // Register Broadcast Peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Court Boundaries
  tft.drawFastHLine(0, 24, 320, 0x39C7);
  tft.drawFastHLine(0, 238, 320, 0x39C7);

  drawHeader();

  // Startup Chime
  setRgb(true, true, false);
  playBeep(1400, 60);
  delay(80);
  playBeep(2000, 80);
  delay(100);
  setRgb(false, false, false);
}

unsigned long lastFrame = 0;
unsigned long lastHeartbeatSend = 0;

void loop() {
  // Touch Screen Input (Paddle Drag & Role Switch)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Role Switch (x <= 90, y <= 24)
    if (y <= 24 && x <= 90) {
      myRole = (myRole == ROLE_LEFT_P1) ? ROLE_RIGHT_P2 : ROLE_LEFT_P1;
      playBeep(1800, 30);
      drawHeader();
      delay(200);
    } else {
      // Paddle Dragging
      paddleY = constrain((float)y, 25.0f + (paddleHeight / 2), 237.0f - (paddleHeight / 2));
    }
  }

  // Periodic Heartbeat
  if (millis() - lastHeartbeatSend > 1500) {
    lastHeartbeatSend = millis();
    sendPongPacket(PKT_HEARTBEAT, 0, 0, 0);
    drawHeader();
  }

  // 60 FPS Physics Loop
  if (millis() - lastFrame >= 16) {
    lastFrame = millis();

    // 1. Erase Previous Paddle
    int paddleX = (myRole == ROLE_LEFT_P1) ? 12 : (320 - 12 - paddleWidth);
    if (abs(paddleY - prevPaddleY) > 0.5f) {
      tft.fillRect(paddleX, (int)(prevPaddleY - paddleHeight / 2), paddleWidth, paddleHeight, 0x0821);
      prevPaddleY = paddleY;
    }

    // 2. Draw Current Paddle
    uint16_t paddleCol = (myRole == ROLE_LEFT_P1) ? 0xFDC0 : 0x07FF;
    tft.fillRoundRect(paddleX, (int)(paddleY - paddleHeight / 2), paddleWidth, paddleHeight, 3, paddleCol);

    // 3. Ball Physics (if ball is on this display)
    if (hasBall) {
      // Erase Old Ball
      tft.fillCircle((int)prevBallX, (int)prevBallY, ballRadius, 0x0821);

      // Move Ball
      ballX += ballVx;
      ballY += ballVy;

      // Top & Bottom Wall Bounces
      if (ballY <= (25 + ballRadius)) {
        ballY = 25 + ballRadius;
        ballVy = -ballVy;
        playBeep(1200, 15);
      } else if (ballY >= (237 - ballRadius)) {
        ballY = 237 - ballRadius;
        ballVy = -ballVy;
        playBeep(1200, 15);
      }

      // Paddle Collision & Open Edge Crossing
      if (myRole == ROLE_LEFT_P1) {
        // Paddle is on Left: x in [12, 20]
        if (ballX - ballRadius <= (paddleX + paddleWidth) && ballX + ballRadius >= paddleX) {
          if (ballY >= (paddleY - paddleHeight / 2 - 4) && ballY <= (paddleY + paddleHeight / 2 + 4)) {
            ballX = paddleX + paddleWidth + ballRadius;
            ballVx = abs(ballVx) * 1.05f; // speed up
            // Alter vertical velocity based on hit offset from paddle center
            float offset = (ballY - paddleY) / (paddleHeight / 2);
            ballVy = offset * 4.0f;
            playBeep(1600, 20);
            setRgb(false, true, false);
            delay(10);
            setRgb(false, false, false);
          }
        }

        // Ball exits Right Screen Edge -> Send to Partner Screen!
        if (ballX >= (320 - ballRadius)) {
          sendPongPacket(PKT_BALL_CROSS, ballY, ballVx, ballVy);
          hasBall = false;
          tft.fillCircle((int)prevBallX, (int)prevBallY, ballRadius, 0x0821);
          playBeep(800, 20);
        }

        // Missed Ball (Left Border Goal)
        if (ballX <= ballRadius) {
          scoreP2++;
          sendPongPacket(PKT_SCORE_SYNC, 0, 0, 0);
          drawHeader();
          setRgb(true, false, false);
          playBeep(400, 250);
          setRgb(false, false, false);
          resetBall();
        }
      } else {
        // ROLE_RIGHT_P2: Paddle is on Right: x in [300, 308]
        if (ballX + ballRadius >= paddleX && ballX - ballRadius <= (paddleX + paddleWidth)) {
          if (ballY >= (paddleY - paddleHeight / 2 - 4) && ballY <= (paddleY + paddleHeight / 2 + 4)) {
            ballX = paddleX - ballRadius;
            ballVx = -abs(ballVx) * 1.05f;
            float offset = (ballY - paddleY) / (paddleHeight / 2);
            ballVy = offset * 4.0f;
            playBeep(1600, 20);
            setRgb(false, true, false);
            delay(10);
            setRgb(false, false, false);
          }
        }

        // Ball exits Left Screen Edge -> Send to Partner Screen!
        if (ballX <= ballRadius) {
          sendPongPacket(PKT_BALL_CROSS, ballY, ballVx, ballVy);
          hasBall = false;
          tft.fillCircle((int)prevBallX, (int)prevBallY, ballRadius, 0x0821);
          playBeep(800, 20);
        }

        // Missed Ball (Right Border Goal)
        if (ballX >= (320 - ballRadius)) {
          scoreP1++;
          sendPongPacket(PKT_SCORE_SYNC, 0, 0, 0);
          drawHeader();
          setRgb(true, false, false);
          playBeep(400, 250);
          setRgb(false, false, false);
          resetBall();
        }
      }

      // Draw New Ball
      if (hasBall) {
        tft.fillCircle((int)ballX, (int)ballY, ballRadius, TFT_WHITE);
        prevBallX = ballX;
        prevBallY = ballY;
      }
    }
  }
}
