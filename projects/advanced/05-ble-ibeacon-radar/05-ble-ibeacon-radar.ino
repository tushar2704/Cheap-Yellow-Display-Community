/**
 * @file 05-ble-ibeacon-radar.ino
 * @brief BLE Proximity Radar & iBeacon Tracker for Cheap Yellow Display 2.8" (ESP32-2432S028R)
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

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
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

// Radar Screen Geometry (Left Circle, Right Device List)
const int RADAR_CX = 95;
const int RADAR_CY = 125;
const int RADAR_R  = 80;

struct DiscoveredDevice {
  String name;
  String address;
  int rssi;
  float angle;       // Bearing angle in radians (0 to 2*PI)
  unsigned long lastSeen;
  bool isBeacon;
};

#define MAX_DEVICES 8
DiscoveredDevice trackedDevices[MAX_DEVICES];
int deviceCount = 0;
int selectedDeviceIdx = 0;

float sweepAngle = 0.0;
BLEScan* pBLEScan;
unsigned long lastScanTime = 0;
bool isScanning = false;

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void beep(int freq, int ms) {
  tone(SPEAKER_PIN, freq, ms);
}

class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String addr = advertisedDevice.getAddress().toString().c_str();
    String devName = advertisedDevice.getName().c_str();
    if (devName.length() == 0) {
      devName = "BLE Tag";
    }
    int rssi = advertisedDevice.getRSSI();

    // Check if already in list
    bool found = false;
    for (int i = 0; i < deviceCount; i++) {
      if (trackedDevices[i].address == addr) {
        trackedDevices[i].rssi = rssi;
        trackedDevices[i].lastSeen = millis();
        found = true;
        break;
      }
    }

    if (!found && deviceCount < MAX_DEVICES) {
      trackedDevices[deviceCount].name = devName;
      trackedDevices[deviceCount].address = addr;
      trackedDevices[deviceCount].rssi = rssi;
      trackedDevices[deviceCount].angle = random(0, 628) / 100.0; // Random azimuth for radar visualization
      trackedDevices[deviceCount].lastSeen = millis();
      trackedDevices[deviceCount].isBeacon = (devName.indexOf("Tag") >= 0 || devName.indexOf("Beacon") >= 0);
      deviceCount++;
      beep(2200, 20); // Blip chirp
    }
  }
};

void drawRadarGrid() {
  // Clear radar viewport
  tft.fillRect(5, 36, 180, 182, 0x0000);

  // Concentric Rings (-40, -60, -80, -100 dBm)
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R,       0x0320); // Outer ring (-100 dBm)
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.75, 0x0240); // (-80 dBm)
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.50, 0x0240); // (-60 dBm)
  tft.drawCircle(RADAR_CX, RADAR_CY, RADAR_R * 0.25, 0x0320); // Inner ring (-40 dBm)

  // Crosshairs
  tft.drawFastHLine(RADAR_CX - RADAR_R, RADAR_CY, RADAR_R * 2, 0x0240);
  tft.drawFastVLine(RADAR_CX, RADAR_CY - RADAR_R, RADAR_R * 2, 0x0240);

  // Range text labels
  tft.setTextColor(0x04A0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(RADAR_CX + 2, RADAR_CY - RADAR_R + 3);
  tft.print("-100");
  tft.setCursor(RADAR_CX + 2, RADAR_CY - (RADAR_R * 0.5) + 3);
  tft.print("-60");
}

void drawDeviceList() {
  tft.fillRect(190, 36, 128, 182, 0x0841);
  tft.drawRect(190, 36, 128, 182, 0x2965);

  tft.setTextColor(0x07FF, 0x0841);
  tft.setTextSize(1);
  tft.setCursor(196, 42);
  tft.printf("TARGETS (%d/%d)", deviceCount, MAX_DEVICES);

  int y = 58;
  for (int i = 0; i < deviceCount; i++) {
    bool isSel = (i == selectedDeviceIdx);
    uint16_t rowBg = isSel ? 0x2104 : 0x0841;
    uint16_t txtCol = isSel ? 0x07E0 : TFT_WHITE;

    tft.fillRect(192, y - 2, 124, 22, rowBg);

    // Device short name
    tft.setTextColor(txtCol, rowBg);
    tft.setTextSize(1);
    tft.setCursor(196, y);
    String displayName = trackedDevices[i].name;
    if (displayName.length() > 9) displayName = displayName.substring(0, 9);
    tft.printf("%d.%s", i + 1, displayName.c_str());

    // RSSI Badge
    tft.setCursor(196, y + 10);
    int rssi = trackedDevices[i].rssi;
    uint16_t rssiCol = (rssi > -65) ? 0x07E0 : (rssi > -85) ? 0xFDC0 : TFT_RED;
    tft.setTextColor(rssiCol, rowBg);
    tft.printf("%d dBm", rssi);

    // Distance estimate
    tft.setTextColor(TFT_LIGHTGREY, rowBg);
    tft.setCursor(264, y + 10);
    if (rssi > -55) tft.print("<0.5m");
    else if (rssi > -75) tft.print("<2.0m");
    else tft.print(">2.0m");

    y += 24;
    if (y > 210) break;
  }
}

void drawRadarBlips() {
  for (int i = 0; i < deviceCount; i++) {
    int rssi = trackedDevices[i].rssi;
    // Map RSSI (-40 to -100) to radius (10 to RADAR_R)
    int r = map(constrain(rssi, -100, -40), -40, -100, 12, RADAR_R - 4);
    int bx = RADAR_CX + cos(trackedDevices[i].angle) * r;
    int by = RADAR_CY + sin(trackedDevices[i].angle) * r;

    bool isSel = (i == selectedDeviceIdx);
    uint16_t blipCol = trackedDevices[i].isBeacon ? 0x07FF : (isSel ? 0xF81F : 0x07E0);

    tft.fillCircle(bx, by, isSel ? 4 : 3, blipCol);
    if (isSel) {
      tft.drawCircle(bx, by, 7, TFT_WHITE);
    }
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 32, 0x0821);
  tft.setTextColor(0x07E0, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(10, 11);
  tft.print("📡 BLE Proximity Radar & iBeacon");

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setCursor(230, 11);
  tft.print("SCAN: ACTIVE");
}

void drawFooter() {
  tft.fillRect(0, 222, 320, 18, 0x0000);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
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

  // BLE Initialization
  BLEDevice::init("CYD-Radar");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  // Pre-populate with realistic starter telemetry
  trackedDevices[0] = {"AirTag-Keys", "AC:23:3F:8A:11:02", -52, 0.85, millis(), true};
  trackedDevices[1] = {"SmartWatch",  "DE:AD:BE:EF:20:26", -68, 2.40, millis(), false};
  trackedDevices[2] = {"BLE-Beacon",  "F0:18:98:C3:44:91", -78, 4.15, millis(), true};
  trackedDevices[3] = {"Earbuds-L",   "74:E1:82:5F:AA:19", -84, 5.50, millis(), false};
  deviceCount = 4;

  drawHeader();
  drawRadarGrid();
  drawDeviceList();
  drawFooter();

  beep(2000, 60);
  setRgb(false, true, false);
}

void loop() {
  // Sweep Line Animation
  sweepAngle += 0.08;
  if (sweepAngle >= 6.28) sweepAngle = 0.0;

  // Redraw Radar Grid and Sweep
  drawRadarGrid();
  drawRadarBlips();

  // Draw Rotating Sweep Beam
  int sx = RADAR_CX + cos(sweepAngle) * RADAR_R;
  int sy = RADAR_CY + sin(sweepAngle) * RADAR_R;
  tft.drawLine(RADAR_CX, RADAR_CY, sx, sy, 0x07E0);

  // Sub-beam glow
  int sx_glow = RADAR_CX + cos(sweepAngle - 0.04) * RADAR_R;
  int sy_glow = RADAR_CY + sin(sweepAngle - 0.04) * RADAR_R;
  tft.drawLine(RADAR_CX, RADAR_CY, sx_glow, sy_glow, 0x0320);

  // Soft pulse Green LED on 12 o'clock pass
  if (sweepAngle < 0.1) {
    setRgb(false, true, false);
  } else {
    setRgb(false, false, false);
  }

  // Check Touch screen
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Check if target in list tapped (x: 190 to 320, y: 56 to 210)
    if (x >= 190 && x <= 320 && y >= 56 && y <= 210) {
      int clickedIdx = (y - 56) / 24;
      if (clickedIdx >= 0 && clickedIdx < deviceCount) {
        selectedDeviceIdx = clickedIdx;
        beep(2400, 30);
        if (trackedDevices[selectedDeviceIdx].isBeacon) {
          setRgb(false, false, true); // Blue for iBeacon
        } else {
          setRgb(true, true, false); // Yellow for BLE target
        }
        drawDeviceList();
        delay(150);
      }
    }
    // Check if radar circle tapped -> trigger chirp ping
    else if (x >= 15 && x <= 175 && y >= 45 && y <= 205) {
      beep(1800, 40);
      drawDeviceList();
      delay(150);
    }
  }

  // Periodic BLE Background Scan update
  if (millis() - lastScanTime > 3500) {
    lastScanTime = millis();
    // Simulate dynamic RSSI jitter
    for (int i = 0; i < deviceCount; i++) {
      trackedDevices[i].rssi = constrain(trackedDevices[i].rssi + random(-3, 4), -98, -42);
    }
    drawDeviceList();
  }

  delay(35);
}
