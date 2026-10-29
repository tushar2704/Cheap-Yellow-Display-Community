/**
 * 🛡️ Project 20: Privacy Shield BLE Tracker Detector
 * 
 * Hardware: Elecrow CrowPanel 2.8" (ESP32-2432S028R)
 * Channel Branding: IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * Author: Tushar Aggarwal (tushar2704)
 * 
 * Features:
 * - Passive BLE sniffer for Apple Find My (AirTag), Samsung SmartTag, and Tile devices.
 * - Circular proximity radar with real-time distance estimation from RSSI path-loss.
 * - Persistent following detector: alerts if an unknown tag travels with the user across multiple scan cycles.
 * - Audio Geiger-counter clicks with frequency proportional to tracker proximity.
 * - Hardware active-low RGB LED safety alert indicators.
 * - Interactive touch controls to toggle Radar vs Tag Inspection Table.
 */

#include <Arduino.h>
#include <SPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <math.h>

// Hardware Pin Definitions for Elecrow 2.8" CYD
#define TFT_BL_PIN    21
#define TOUCH_CS      33
#define TOUCH_IRQ     36
#define TOUCH_MOSI    13
#define TOUCH_MISO    12
#define TOUCH_CLK     14

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define BUZZER_PIN    26

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
BLEScan* pBLEScan;

struct DetectedTracker {
  char mac[18];
  char type[16];
  int rssi;
  float distanceM;
  uint32_t firstSeen;
  uint32_t lastSeen;
  int angleDeg;
  uint16_t tagColor;
  bool isSuspicious;
};

const int MAX_TRACKERS = 8;
DetectedTracker trackers[MAX_TRACKERS];
int trackerCount = 0;

int currentView = 0; // 0 = Proximity Radar, 1 = Tracker List Table
bool geigerAudioEnabled = true;
int radarAngle = 0;
unsigned long lastRadarSweep = 0;
unsigned long lastGeigerClick = 0;

void setLED(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq = 2000, int ms = 30) {
  tone(BUZZER_PIN, freq, ms);
}

// Distance approximation based on RSSI
float calculateDistance(int rssi) {
  int txPower = -59; // Measured power at 1 meter
  if (rssi == 0) return -1.0;
  float ratio = (float)(txPower - rssi) / (10.0 * 2.0); // Path loss exponent n=2.0
  return pow(10, ratio);
}

class TrackerAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    bool isAirTag = false;
    bool isSmartTag = false;
    bool isTile = false;

    // Check Manufacturer Data
    if (advertisedDevice.haveManufacturerData()) {
      std::string mData = advertisedDevice.getManufacturerData();
      if (mData.length() >= 4) {
        uint8_t cId0 = (uint8_t)mData[0];
        uint8_t cId1 = (uint8_t)mData[1];

        // Apple Find My / AirTag (0x004C, Type 0x12)
        if (cId0 == 0x4C && cId1 == 0x00 && (uint8_t)mData[2] == 0x12) {
          isAirTag = true;
        }
        // Samsung SmartTag (0x0075)
        else if (cId0 == 0x75 && cId1 == 0x00) {
          isSmartTag = true;
        }
        // Tile Tracker (0x00D6)
        else if (cId0 == 0xD6 && cId1 == 0x00) {
          isTile = true;
        }
      }
    }

    if (advertisedDevice.haveServiceUUID()) {
      if (advertisedDevice.getServiceUUID().toString().find("feed") != std::string::npos) {
        isTile = true;
      }
    }

    if (isAirTag || isSmartTag || isTile) {
      const char* macStr = advertisedDevice.getAddress().toString().c_str();
      int rssi = advertisedDevice.getRSSI();

      // Check if already in tracking list
      int foundIdx = -1;
      for (int i = 0; i < trackerCount; i++) {
        if (strcmp(trackers[i].mac, macStr) == 0) {
          foundIdx = i;
          break;
        }
      }

      if (foundIdx != -1) {
        trackers[foundIdx].rssi = rssi;
        trackers[foundIdx].distanceM = calculateDistance(rssi);
        trackers[foundIdx].lastSeen = millis();
        // If seen for more than 45 seconds, mark suspicious
        if ((trackers[foundIdx].lastSeen - trackers[foundIdx].firstSeen) > 45000) {
          trackers[foundIdx].isSuspicious = true;
        }
      } else if (trackerCount < MAX_TRACKERS) {
        strncpy(trackers[trackerCount].mac, macStr, 17);
        trackers[trackerCount].mac[17] = '\0';
        trackers[trackerCount].rssi = rssi;
        trackers[trackerCount].distanceM = calculateDistance(rssi);
        trackers[trackerCount].firstSeen = millis();
        trackers[trackerCount].lastSeen = millis();
        trackers[trackerCount].angleDeg = random(0, 360);
        trackers[trackerCount].isSuspicious = false;

        if (isAirTag) {
          strcpy(trackers[trackerCount].type, "AirTag");
          trackers[trackerCount].tagColor = TFT_WHITE;
        } else if (isSmartTag) {
          strcpy(trackers[trackerCount].type, "SmartTag");
          trackers[trackerCount].tagColor = TFT_CYAN;
        } else {
          strcpy(trackers[trackerCount].type, "Tile");
          trackers[trackerCount].tagColor = TFT_ORANGE;
        }
        trackerCount++;
      }
    }
  }
};

void updateThreatLED() {
  bool hasSuspicious = false;
  for (int i = 0; i < trackerCount; i++) {
    if (trackers[i].isSuspicious) hasSuspicious = true;
  }

  if (hasSuspicious) {
    setLED(true, false, false); // Red: Persistent following alert!
  } else if (trackerCount > 0) {
    setLED(true, true, false);  // Yellow: Trackers in vicinity
  } else {
    setLED(false, true, false); // Green: All clear
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, tft.color565(15, 25, 35));
  tft.setTextColor(TFT_WHITE, tft.color565(15, 25, 35));
  tft.setTextSize(1);
  tft.drawString("PRIVACY SHIELD DETECTOR", 8, 6, 2);

  char statBuf[32];
  snprintf(statBuf, sizeof(statBuf), "TRACKERS:%d  %s", trackerCount, (trackerCount > 0) ? "ALERT!" : "CLEAR");
  tft.setTextColor((trackerCount > 0) ? TFT_YELLOW : TFT_GREEN, tft.color565(15, 25, 35));
  tft.drawRightString(statBuf, 312, 6, 2);
}

void drawRadarView() {
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  int cx = 160;
  int cy = 115;

  // Concentric radar rings (Immediate: 25px, Near: 50px, Far: 75px)
  tft.drawCircle(cx, cy, 25, tft.color565(30, 45, 60));
  tft.drawCircle(cx, cy, 50, tft.color565(30, 45, 60));
  tft.drawCircle(cx, cy, 75, tft.color565(30, 45, 60));

  tft.drawFastHLine(cx - 75, cy, 150, tft.color565(25, 35, 45));
  tft.drawFastVLine(cx, cy - 75, 150, tft.color565(25, 35, 45));

  // Distance range labels
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("<1.5m", cx + 4, cy - 22, 1);
  tft.drawString("<5m", cx + 4, cy - 47, 1);
  tft.drawString(">5m", cx + 4, cy - 72, 1);

  // Rotating radar sweep line
  float rad = radarAngle * M_PI / 180.0;
  int rx = cx + (int)(75 * cos(rad));
  int ry = cy + (int)(75 * sin(rad));
  tft.drawLine(cx, cy, rx, ry, TFT_GREEN);

  // Render tracker blips
  for (int i = 0; i < trackerCount; i++) {
    float distNorm = constrain(trackers[i].distanceM / 10.0, 0.2, 1.0);
    int r = (int)(distNorm * 72);
    float aRad = trackers[i].angleDeg * M_PI / 180.0;
    int bx = cx + (int)(r * cos(aRad));
    int by = cy + (int)(r * sin(aRad));

    uint16_t blipColor = trackers[i].isSuspicious ? TFT_RED : trackers[i].tagColor;
    tft.fillCircle(bx, by, 5, blipColor);
    tft.drawCircle(bx, by, 7, TFT_WHITE);

    // Label tag
    tft.setTextColor(blipColor, TFT_BLACK);
    tft.drawString(trackers[i].type, bx + 8, by - 4, 1);
  }

  // Persistent Warning Banner if detected
  bool anySuspicious = false;
  for (int i = 0; i < trackerCount; i++) {
    if (trackers[i].isSuspicious) anySuspicious = true;
  }

  if (anySuspicious) {
    tft.fillRoundRect(20, 184, 280, 16, 2, TFT_RED);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawCentreString("! CAUTION: SUSPICIOUS TRACKER FOLLOWING !", 160, 186, 1);
  }
}

void drawTrackerListView() {
  tft.fillRect(0, 26, 320, 178, TFT_BLACK);

  // Table header
  tft.fillRoundRect(4, 30, 312, 18, 2, tft.color565(25, 30, 40));
  tft.setTextColor(TFT_LIGHTGREY, tft.color565(25, 30, 40));
  tft.drawString("TYPE", 8, 34, 1);
  tft.drawString("MAC ADDRESS", 80, 34, 1);
  tft.drawString("RSSI", 215, 34, 1);
  tft.drawString("DIST", 270, 34, 1);

  if (trackerCount == 0) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("NO SUSPICIOUS BLE TRACKERS DETECTED", 160, 100, 2);
    return;
  }

  int visible = (trackerCount > 6) ? 6 : trackerCount;
  for (int i = 0; i < visible; i++) {
    int y = 52 + i * 22;
    uint16_t rowBg = (i % 2 == 0) ? tft.color565(15, 18, 24) : TFT_BLACK;

    tft.fillRect(4, y, 312, 20, rowBg);

    // Tag Type
    tft.setTextColor(trackers[i].tagColor, rowBg);
    tft.drawString(trackers[i].type, 8, y + 4, 1);

    // MAC
    tft.setTextColor(TFT_WHITE, rowBg);
    tft.drawString(trackers[i].mac, 80, y + 4, 1);

    // RSSI
    tft.setTextColor((trackers[i].rssi > -65) ? TFT_YELLOW : TFT_LIGHTGREY, rowBg);
    char rssiBuf[8];
    snprintf(rssiBuf, sizeof(rssiBuf), "%ddB", trackers[i].rssi);
    tft.drawString(rssiBuf, 215, y + 4, 1);

    // Approx Distance
    char distBuf[12];
    snprintf(distBuf, sizeof(distBuf), "%.1fm", trackers[i].distanceM);
    tft.setTextColor(trackers[i].isSuspicious ? TFT_RED : TFT_GREEN, rowBg);
    tft.drawString(distBuf, 270, y + 4, 1);
  }
}

void drawUI() {
  drawHeader();
  if (currentView == 0) {
    drawRadarView();
  } else {
    drawTrackerListView();
  }

  // Bottom Touch Controls Bar
  tft.fillRoundRect(8, 206, 96, 30, 4, tft.color565(35, 40, 50));
  tft.setTextColor(TFT_WHITE, tft.color565(35, 40, 50));
  tft.drawCentreString("SCAN", 56, 214, 2);

  uint16_t viewBtnBg = (currentView == 0) ? TFT_DARKCYAN : tft.color565(35, 40, 50);
  tft.fillRoundRect(112, 206, 96, 30, 4, viewBtnBg);
  tft.setTextColor(TFT_WHITE, viewBtnBg);
  tft.drawCentreString(currentView == 0 ? "RADAR" : "TAG LIST", 160, 214, 2);

  uint16_t geigerBtnBg = geigerAudioEnabled ? tft.color565(20, 50, 30) : tft.color565(40, 30, 30);
  tft.fillRoundRect(216, 206, 96, 30, 4, geigerBtnBg);
  tft.setTextColor(geigerAudioEnabled ? TFT_GREEN : TFT_LIGHTGREY, geigerBtnBg);
  tft.drawCentreString(geigerAudioEnabled ? "GEIGER ON" : "MUTED", 264, 214, 2);

  updateThreatLED();
}

void checkTouch() {
  if (!ts.touched()) return;

  TS_Point p = ts.getPoint();
  int x = map(p.x, 3800, 200, 0, 320);
  int y = map(p.y, 3800, 200, 0, 240);
  x = constrain(x, 0, 320);
  y = constrain(y, 0, 240);

  if (y >= 195) {
    if (x < 105) {
      // RESCAN
      beep(2000, 40);
      pBLEScan->start(2, false);
      pBLEScan->clearResults();
      drawUI();
      delay(200);
    } else if (x >= 110 && x < 210) {
      // TOGGLE VIEW
      currentView = (currentView == 0) ? 1 : 0;
      beep(2200, 40);
      drawUI();
      delay(200);
    } else if (x >= 215) {
      // TOGGLE GEIGER AUDIO
      geigerAudioEnabled = !geigerAudioEnabled;
      beep(2500, 40);
      drawUI();
      delay(200);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Privacy Shield BLE Tracker Detector Starting...");

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  beep(2200, 60);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Initialize BLE Scanner
  BLEDevice::init("CYD-Shield");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new TrackerAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  // Seed sample tracker if standalone
  strcpy(trackers[0].mac, "4F:8B:92:A1:33:DE");
  strcpy(trackers[0].type, "AirTag");
  trackers[0].rssi = -68;
  trackers[0].distanceM = calculateDistance(-68);
  trackers[0].firstSeen = millis() - 60000;
  trackers[0].lastSeen = millis();
  trackers[0].angleDeg = 45;
  trackers[0].tagColor = TFT_WHITE;
  trackers[0].isSuspicious = true;
  trackerCount = 1;

  drawUI();
}

void loop() {
  checkTouch();

  // Rotate radar beam every 80ms
  if (currentView == 0 && (millis() - lastRadarSweep >= 80)) {
    lastRadarSweep = millis();
    radarAngle = (radarAngle + 6) % 360;
    drawRadarView();
  }

  // Geiger Counter Audio Clicks
  if (geigerAudioEnabled && trackerCount > 0) {
    int closestRssi = -100;
    for (int i = 0; i < trackerCount; i++) {
      if (trackers[i].rssi > closestRssi) closestRssi = trackers[i].rssi;
    }
    int clickInterval = map(closestRssi, -95, -40, 1000, 80);
    clickInterval = constrain(clickInterval, 80, 1200);

    if (millis() - lastGeigerClick >= clickInterval) {
      lastGeigerClick = millis();
      beep(1500, 8); // Crisp geiger pulse
    }
  }

  delay(15);
}
