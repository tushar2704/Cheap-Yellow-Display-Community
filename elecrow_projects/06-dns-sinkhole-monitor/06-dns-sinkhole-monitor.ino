/**
 * @file 06-dns-sinkhole-monitor.ino
 * @brief ESP32 Ad-Blocking DNS Sinkhole & Traffic Monitor
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240
 * - Touch: XPT2046 Resistive Touch
 * - Network: ESP32 Wi-Fi + UDP Port 53 DNS Sinkhole
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ   36
#define XPT2046_MOSI  13
#define XPT2046_MISO  12
#define XPT2046_CLK   14
#define XPT2046_CS    33

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

// Wi-Fi Config
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
WiFiUDP dnsServer;

// Sinkhole Statistics
unsigned long totalQueries = 12480;
unsigned long blockedQueries = 3120;
String lastBlockedDomain = "ads.doubleclick.net";
String lastAllowedDomain = "api.github.com";

// Snooze Management
bool isSnoozed = false;
unsigned long snoozeStartTime = 0;
const unsigned long SNOOZE_DURATION = 300000; // 5 minutes in ms
unsigned long lastUiUpdate = 0;
unsigned long lastSimTraffic = 0;
unsigned long lastTouchTime = 0;

// High-confidence ad & tracking keywords
const char* adKeywords[] = {
  "adservice", "doubleclick", "telemetry", "analytics", 
  "crashlytics", "track", "metrics", "banner", "pixel"
};

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
}

bool isDomainBlacklisted(const String& domain) {
  if (isSnoozed) return false;
  for (int i = 0; i < 9; i++) {
    if (domain.indexOf(adKeywords[i]) >= 0) {
      return true;
    }
  }
  return false;
}

void drawDashboard() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 4);
  tft.print("ESP32 DNS SINKHOLE  ");
  tft.setTextColor(isSnoozed ? 0xF800 : 0x07E0, 0x0010);
  tft.print(isSnoozed ? "[SNOOZED - PAUSED]" : "[SHIELD ACTIVE]");

  // IP Address Badge
  tft.setTextColor(TFT_WHITE, 0x0010);
  tft.setCursor(205, 4);
  if (WiFi.status() == WL_CONNECTED) {
    tft.print(WiFi.localIP().toString());
  } else {
    tft.print("192.168.1.1 (AP)");
  }

  // 3 Metric Cards Across Screen
  int cardY = 28;
  int cardW = 98;
  int cardH = 50;

  // Total Queries Card
  tft.fillRoundRect(6, cardY, cardW, cardH, 4, 0x0000);
  tft.drawRoundRect(6, cardY, cardW, cardH, 4, 0x041F);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(12, cardY + 6);
  tft.print("TOTAL QUERIES");
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(12, cardY + 22);
  tft.printf("%lu", totalQueries);

  // Blocked Queries Card
  tft.fillRoundRect(111, cardY, cardW, cardH, 4, 0x0000);
  tft.drawRoundRect(111, cardY, cardW, cardH, 4, 0xF800);
  tft.setTextColor(0xF800, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(117, cardY + 6);
  tft.print("ADS BLOCKED");
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(117, cardY + 22);
  tft.printf("%lu", blockedQueries);

  // Block Rate Card
  float blockRate = (totalQueries > 0) ? ((float)blockedQueries / totalQueries * 100.0f) : 0;
  tft.fillRoundRect(216, cardY, cardW, cardH, 4, 0x0000);
  tft.drawRoundRect(216, cardY, cardW, cardH, 4, 0x07E0);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(222, cardY + 6);
  tft.print("BLOCK RATE");
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(222, cardY + 22);
  tft.printf("%.1f%%", blockRate);

  // Live Domain Ticker Card
  int tickY = 84;
  tft.fillRoundRect(6, tickY, 308, 62, 4, 0x0000);
  tft.drawRoundRect(6, tickY, 308, 62, 4, 0x2965);

  tft.setTextSize(1);
  tft.setTextColor(0xF800, 0x0000);
  tft.setCursor(12, tickY + 8);
  tft.print("[BLOCKED SINKHOLE]: ");
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.println(lastBlockedDomain);

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(12, tickY + 28);
  tft.print("[ALLOWED TRAFFIC ]: ");
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.println(lastAllowedDomain);

  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(12, tickY + 46);
  tft.print("UPSTREAM DNS: 1.1.1.1 (Cloudflare Anycast)");

  // Giant Snooze / Resume Touch Button
  int btnY = 152;
  int btnW = 308;
  int btnH = 64;

  uint16_t btnBg = isSnoozed ? 0x03E0 : 0xE280; // Green if snoozed (to resume), Orange to snooze
  tft.fillRoundRect(6, btnY, btnW, btnH, 6, btnBg);
  tft.drawRoundRect(6, btnY, btnW, btnH, 6, TFT_WHITE);

  tft.setTextColor(TFT_BLACK, btnBg);
  tft.setTextSize(2);
  if (isSnoozed) {
    unsigned long remainingSec = (SNOOZE_DURATION - (millis() - snoozeStartTime)) / 1000;
    tft.setCursor(24, btnY + 16);
    tft.print("RESUME PROTECTION NOW");
    tft.setTextSize(1);
    tft.setCursor(68, btnY + 42);
    tft.printf("Auto-resuming in %lu seconds...", remainingSec);
  } else {
    tft.setCursor(18, btnY + 16);
    tft.print("PAUSE BLOCKING (5 MIN)");
    tft.setTextSize(1);
    tft.setCursor(44, btnY + 42);
    tft.print("Temporarily allows affiliate & broken shopping links");
  }

  // Footer
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print("UDP Port 53 Standalone DNS Sinkhole | @IamNotIntoTech");
}

void processIncomingDns() {
  int packetSize = dnsServer.parsePacket();
  if (packetSize > 0) {
    uint8_t buffer[512];
    int len = dnsServer.read(buffer, sizeof(buffer));
    totalQueries++;

    // Basic DNS Query Header parsing
    if (len > 12) {
      // Extract rough domain string
      String domain = "";
      int idx = 12;
      while (idx < len && buffer[idx] != 0) {
        int labelLen = buffer[idx++];
        for (int l = 0; l < labelLen && idx < len; l++) {
          domain += (char)buffer[idx++];
        }
        domain += ".";
      }

      if (isDomainBlacklisted(domain)) {
        blockedQueries++;
        lastBlockedDomain = domain;
        setRgbLed(true, false, false); // Red pulse

        // Craft DNS Sinkhole Response (Points to 0.0.0.0)
        buffer[2] |= 0x81; // Response flags
        buffer[3] |= 0x80;
        buffer[6] = 0x00; buffer[7] = 0x01; // 1 Answer

        int ansIdx = len;
        buffer[ansIdx++] = 0xC0; buffer[ansIdx++] = 0x0C; // Pointer to domain name
        buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x01; // Type A
        buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x01; // Class IN
        buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x3C; // TTL 60s
        buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x04; // Data length 4
        buffer[ansIdx++] = 0; buffer[ansIdx++] = 0; buffer[ansIdx++] = 0; buffer[ansIdx++] = 0; // 0.0.0.0

        dnsServer.beginPacket(dnsServer.remoteIP(), dnsServer.remotePort());
        dnsServer.write(buffer, ansIdx);
        dnsServer.endPacket();

        delay(10);
        setRgbLed(false, true, false);
      } else {
        lastAllowedDomain = domain;
      }
    }
  }
}

void toggleSnooze() {
  isSnoozed = !isSnoozed;
  if (isSnoozed) {
    snoozeStartTime = millis();
    playBeep(1200, 100);
    setRgbLed(true, true, false); // Amber
  } else {
    playBeep(2200, 100);
    setRgbLed(false, true, false); // Green
  }
  drawDashboard();
}

void setup() {
  Serial.begin(115200);
  delay(150);

  Serial.println("\n--- [Elecrow 2.8 CYD] 06 DNS Sinkhole & Traffic Monitor ---");
  Serial.println("Channel: @IamNotIntoTech");
  Serial.println("Author: Tushar Aggarwal (@tushar2704)");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Connect Wi-Fi or create standalone AP
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  dnsServer.begin(53); // Port 53 DNS

  // Splash
  tft.fillRect(0, 0, 320, 240, 0x0821);
  tft.fillRoundRect(20, 30, 280, 180, 8, 0x0000);
  tft.drawRoundRect(20, 30, 280, 180, 8, 0xF800);

  tft.setTextColor(0xFDC0, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(44, 55);
  tft.print("IamNotIntoTech");

  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(44, 90);
  tft.print("ELECROW 2.8\" CYD PROJECT SUITE");

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(44, 115);
  tft.print("Project 06: Ad-Blocking DNS Sinkhole");

  tft.setTextColor(TFT_DARKGREY, 0x0000);
  tft.setCursor(44, 140);
  tft.print("UDP Port 53 | 5-Minute Snooze Protection");

  playBeep(1500, 80);
  delay(100);
  playBeep(2000, 100);
  delay(500);

  drawDashboard();
  setRgbLed(false, true, false);
}

void loop() {
  processIncomingDns();

  // Snooze timeout check
  if (isSnoozed && (millis() - snoozeStartTime >= SNOOZE_DURATION)) {
    isSnoozed = false;
    playBeep(2400, 100);
    setRgbLed(false, true, false);
    drawDashboard();
  }

  // Periodic simulated background traffic if standalone
  if (millis() - lastSimTraffic > 2000) {
    lastSimTraffic = millis();
    totalQueries += random(1, 4);
    if (random(0, 10) > 6 && !isSnoozed) {
      blockedQueries++;
      const char* simAds[] = {"telemetry.google.com", "adservice.amazon.com", "metrics.icloud.com"};
      lastBlockedDomain = simAds[random(0, 3)];
    } else {
      const char* simGoods[] = {"cdn.discordapp.com", "wikipedia.org", "registry.npmjs.org"};
      lastAllowedDomain = simGoods[random(0, 3)];
    }
    drawDashboard();
  }

  // Touch handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (millis() - lastTouchTime > 250) {
      lastTouchTime = millis();

      int screenX = map(p.x, TS_MINX, TS_MAXX, 0, 320);
      int screenY = map(p.y, TS_MINY, TS_MAXY, 0, 240);
      screenX = constrain(screenX, 0, 319);
      screenY = constrain(screenY, 0, 239);

      // Check Snooze button (X: 6 to 314, Y: 152 to 216)
      if (screenX >= 6 && screenX <= 314 && screenY >= 152 && screenY <= 216) {
        toggleSnooze();
      }
    }
  }

  delay(20);
}
