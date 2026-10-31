/**
 * 🛡️ Project 25: Wi-Fi Deauth Defense Sentinel
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Promiscuous 802.11 management frame sniffer detecting deauthentication & disassociation floods.
 * - Real-time packet rate counter (deauth frames per second) with peak hold meter.
 * - Attack event log capturing attacker BSSID, victim client MAC, and signal strength.
 * - Channel hopping mode (Channels 1-13) or locked single-channel monitoring.
 * - Hardware active-low RGB LED flashing red during active RF jamming attacks.
 * - Onboard acoustic siren alarm alert for smart security camera defense.
 * - Calibrated non-inverted resistive touch controls.
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_wifi.h>
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

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);

struct AttackEvent {
  char attackerMac[18];
  char victimMac[18];
  int channel;
  int rssi;
  const char* type;
};

const int MAX_EVENTS = 6;
AttackEvent attackLogs[MAX_EVENTS];
int logCount = 0;

volatile uint32_t deauthCount = 0;
uint32_t deauthRate = 0;
uint32_t peakRate = 0;
bool underAttack = false;
bool sirenMuted = false;
bool channelHopping = true;
int currentChannel = 1;
unsigned long lastRateCalc = 0;
unsigned long lastHopTime = 0;
unsigned long lastTouchTime = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

// 802.11 Promiscuous Sniffer Callback
void snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT) return;

  const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  const uint8_t* payload = pkt->payload;
  int len = pkt->rx_ctrl.sig_len;

  if (len < 24) return;

  uint8_t frameControl = payload[0];
  uint8_t frameSubtype = (frameControl & 0xF0) >> 4;

  // Subtype 12 (0x0C) = Deauthentication, Subtype 10 (0x0A) = Disassociation
  if (frameSubtype == 12 || frameSubtype == 10) {
    deauthCount++;

    // Format MACs: Target = payload[4..9], Source = payload[10..15]
    if (logCount < MAX_EVENTS) {
      snprintf(attackLogs[logCount].victimMac, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
               payload[4], payload[5], payload[6], payload[7], payload[8], payload[9]);
      snprintf(attackLogs[logCount].attackerMac, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
               payload[10], payload[11], payload[12], payload[13], payload[14], payload[15]);
      attackLogs[logCount].channel = pkt->rx_ctrl.channel;
      attackLogs[logCount].rssi = pkt->rx_ctrl.rssi;
      attackLogs[logCount].type = (frameSubtype == 12) ? "DEAUTH" : "DISASSOC";
      logCount++;
    }
  }
}

void drawHeader() {
  uint16_t bg = underAttack ? TFT_RED : tft.color565(15, 25, 35);

  tft.fillRect(0, 0, 320, 26, bg);
  tft.setTextColor(TFT_WHITE, bg);
  tft.setTextSize(1);
  tft.drawString("WIFI DEAUTH SENTINEL", 8, 6, 2);

  char statBuf[32];
  snprintf(statBuf, sizeof(statBuf), "CH:%d %s %s", currentChannel, channelHopping ? "HOP" : "LOCK", underAttack ? "! ATTACK !" : "SECURE");
  tft.setTextColor(underAttack ? TFT_YELLOW : TFT_GREEN, bg);
  tft.drawRightString(statBuf, 312, 6, 2);
}

void drawDashboard() {
  drawHeader();

  // Rate Meter Card
  tft.fillRoundRect(8, 30, 304, 55, 4, tft.color565(15, 20, 28));
  tft.drawRoundRect(8, 30, 304, 55, 4, underAttack ? TFT_RED : TFT_CYAN);

  tft.setTextColor(underAttack ? TFT_RED : TFT_CYAN, tft.color565(15, 20, 28));
  tft.drawString("DEAUTHENTICATION FRAME VELOCITY", 16, 36, 1);

  // Rate bar
  int barW = map(deauthRate, 0, 40, 0, 280);
  barW = constrain(barW, 0, 280);
  uint16_t barCol = underAttack ? TFT_RED : ((deauthRate > 5) ? TFT_YELLOW : TFT_GREEN);

  tft.fillRect(16, 50, 280, 14, tft.color565(30, 35, 45));
  tft.fillRect(16, 50, barW, 14, barCol);

  char rateBuf[48];
  snprintf(rateBuf, sizeof(rateBuf), "RATE: %d frames/s   PEAK: %d frames/s", deauthRate, peakRate);
  tft.setTextColor(TFT_WHITE, tft.color565(15, 20, 28));
  tft.drawString(rateBuf, 16, 68, 1);

  // Attack Event Table
  tft.fillRoundRect(8, 90, 304, 108, 4, tft.color565(15, 18, 24));
  tft.drawRoundRect(8, 90, 304, 108, 4, TFT_DARKGREY);

  tft.fillRoundRect(10, 92, 300, 16, 2, tft.color565(25, 30, 40));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 40));
  tft.drawString("TYPE", 14, 96, 1);
  tft.drawString("ATTACKER MAC", 70, 96, 1);
  tft.drawString("VICTIM MAC", 185, 96, 1);
  tft.drawString("CH", 295, 96, 1);

  if (logCount == 0) {
    tft.setTextColor(TFT_GREEN, tft.color565(15, 18, 24));
    tft.drawCentreString("ZERO INTRUSIONS DETECTED - AIRSPACE CLEAN", 160, 145, 1);
  } else {
    for (int i = 0; i < logCount; i++) {
      int y = 112 + i * 14;
      tft.setTextColor(TFT_RED, tft.color565(15, 18, 24));
      tft.drawString(attackLogs[i].type, 14, y, 1);

      tft.setTextColor(TFT_WHITE, tft.color565(15, 18, 24));
      tft.drawString(attackLogs[i].attackerMac, 70, y, 1);
      tft.drawString(attackLogs[i].victimMac, 185, y, 1);

      tft.setTextColor(TFT_YELLOW, tft.color565(15, 18, 24));
      tft.drawNumber(attackLogs[i].channel, 295, y, 1);
    }
  }

  // Bottom Controls (Y: 204 to 240)
  uint16_t hopBg = channelHopping ? TFT_DARKCYAN : tft.color565(35, 40, 50);
  tft.fillRoundRect(8, 204, 96, 32, 4, hopBg);
  tft.drawRoundRect(8, 204, 96, 32, 4, channelHopping ? TFT_CYAN : TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, hopBg);
  tft.setTextSize(1);
  tft.drawCentreString(channelHopping ? "HOP 1-13" : "LOCK CH", 56, 212, 2);

  uint16_t muteBg = sirenMuted ? tft.color565(50, 40, 30) : tft.color565(20, 50, 30);
  tft.fillRoundRect(112, 204, 96, 32, 4, muteBg);
  tft.drawRoundRect(112, 204, 96, 32, 4, sirenMuted ? TFT_YELLOW : TFT_GREEN);
  tft.setTextColor(sirenMuted ? TFT_YELLOW : TFT_GREEN, muteBg);
  tft.drawCentreString(sirenMuted ? "MUTED" : "SIREN ON", 160, 212, 2);

  tft.fillRoundRect(216, 204, 96, 32, 4, tft.color565(40, 20, 25));
  tft.drawRoundRect(216, 204, 96, 32, 4, TFT_RED);
  tft.setTextColor(TFT_WHITE, tft.color565(40, 20, 25));
  tft.drawCentreString("CLEAR LOGS", 264, 212, 2);
}

void checkTouch() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 180) {
        lastTouchTime = millis();
        // Calibrated non-inverted coordinate mapping for 2.8" CYD
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        if (screenY >= 195) {
          if (screenX < 105) {
            // Toggle Hopping / Lock
            channelHopping = !channelHopping;
            beep(1800, 40);
            drawDashboard();
          } else if (screenX >= 110 && screenX < 210) {
            // Toggle Siren Mute
            sirenMuted = !sirenMuted;
            beep(sirenMuted ? 1400 : 2600, 50);
            drawDashboard();
          } else if (screenX >= 215) {
            // Clear logs
            logCount = 0;
            peakRate = 0;
            beep(2000, 60);
            drawDashboard();
          }
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Wi-Fi Deauth Defense Sentinel Initializing for @IamNotIntoTech...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setLED(false, true, false); // Green: Secure
  beep(2200, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Initialize VSPI Touch Controller
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Initialize Wi-Fi in Station Mode & enable Promiscuous Management Frame Sniffing
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  wifi_promiscuous_filter_t filter = { .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT };
  esp_wifi_set_promiscuous_filter(&filter);
  esp_wifi_set_promiscuous_rx_cb(&snifferCallback);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);

  drawDashboard();
}

void loop() {
  checkTouch();

  // 1-second Rate Velocity Calculation
  if (millis() - lastRateCalc >= 1000) {
    lastRateCalc = millis();
    deauthRate = deauthCount;
    deauthCount = 0;

    if (deauthRate > peakRate) peakRate = deauthRate;
    underAttack = (deauthRate > 10);

    if (underAttack) {
      setLED(true, false, false); // Flashing Red on attack
      if (!sirenMuted) {
        beep(1200, 60);
        delay(60);
        beep(2400, 80);
      }
    } else if (deauthRate > 2) {
      setLED(true, true, false);  // Yellow warning
    } else {
      setLED(false, true, false); // Green: Secure
    }

    drawDashboard();
  }

  // Channel hopping every 300ms if enabled
  if (channelHopping && (millis() - lastHopTime >= 300)) {
    lastHopTime = millis();
    currentChannel = (currentChannel % 13) + 1;
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  }

  delay(15);
}

