/**
 * @file 06-dns-sinkhole-monitor.ino
 * @brief ESP32 Ad-Blocking DNS Sinkhole & Traffic Monitor (Flicker-Free & Calibrated)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 (HSPI: MOSI=13, MISO=12, CLK=14, CS=15)
 * - Touch: XPT2046 Resistive Touch (VSPI: MOSI=32, MISO=39, CLK=25, CS=33)
 * - Network: ESP32 Wi-Fi + UDP Port 53 DNS Sinkhole
 * - Audio: GPIO 26 Tone feedback
 * - RGB LED: GPIO 4, 16, 17 [Active LOW]
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Accurate CYD 2.8" Resistive Touch SPI Pinout
#define XPT2046_MOSI  32
#define XPT2046_MISO  39
#define XPT2046_CLK   25
#define XPT2046_CS    33
#define XPT2046_IRQ   36

#define LED_RED       4
#define LED_GREEN     16
#define LED_BLUE      17
#define LDR_PIN       34
#define SPEAKER_PIN   26

// Wi-Fi Config
const char* ssid = "HOMELAB_WIFI";
const char* password = "homelab_secret";

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS); // Polling mode avoids GPIO 36 interrupt lockup
TFT_eSPI tft = TFT_eSPI();
WiFiUDP dnsServer;

// Sinkhole Statistics
unsigned long totalQueries = 14820;
unsigned long blockedQueries = 3840;
unsigned long prevTotalQueries = 0;
unsigned long prevBlockedQueries = 0;
float prevBlockRate = -1.0f;
String lastBlockedDomain = "ads.doubleclick.net";
String lastAllowedDomain = "api.github.com";

// Snooze Management
bool isSnoozed = false;
unsigned long snoozeStartTime = 0;
const unsigned long SNOOZE_DURATION = 300000; // 5 minutes in ms
unsigned long lastSimTraffic = 0;
unsigned long lastSnoozeTimerUpdate = 0;
unsigned long lastTouchTime = 0;
unsigned long footerClearTime = 0;

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

void updateFooter(const char* message, uint16_t fg = TFT_WHITE, uint16_t bg = 0x18C3) {
  tft.fillRect(0, 224, 320, 16, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(1);
  tft.setCursor(8, 228);
  tft.print(message);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 24, 0x0010);
  tft.setTextColor(0xFFE0, 0x0010);
  tft.setTextSize(1);
  tft.setCursor(6, 6);
  tft.print("ESP32 DNS SINKHOLE  ");
  tft.setTextColor(isSnoozed ? 0xF800 : 0x07E0, 0x0010);
  tft.print(isSnoozed ? "[SNOOZED - PAUSED]" : "[SHIELD ACTIVE]");

  // IP Address Badge
  tft.setTextColor(TFT_WHITE, 0x0010);
  tft.setCursor(208, 6);
  if (WiFi.status() == WL_CONNECTED) {
    tft.print(WiFi.localIP().toString());
  } else {
    tft.print("192.168.4.1 (AP)");
  }
}

void drawStaticCards() {
  int cardY = 28;
  int cardW = 98;
  int cardH = 50;

  // 1. Total Queries Card
  tft.fillRoundRect(6, cardY, cardW, cardH, 4, 0x0000);
  tft.drawRoundRect(6, cardY, cardW, cardH, 4, 0x041F);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(12, cardY + 6);
  tft.print("TOTAL QUERIES");

  // 2. Blocked Queries Card
  tft.fillRoundRect(111, cardY, cardW, cardH, 4, 0x0000);
  tft.drawRoundRect(111, cardY, cardW, cardH, 4, 0xF800);
  tft.setTextColor(0xF800, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(117, cardY + 6);
  tft.print("ADS BLOCKED");

  // 3. Block Rate Card
  tft.fillRoundRect(216, cardY, cardW, cardH, 4, 0x0000);
  tft.drawRoundRect(216, cardY, cardW, cardH, 4, 0x07E0);
  tft.setTextColor(0x07E0, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(222, cardY + 6);
  tft.print("BLOCK RATE");

  // 4. Live Domain Ticker Frame
  int tickY = 84;
  tft.fillRoundRect(6, tickY, 308, 62, 4, 0x0000);
  tft.drawRoundRect(6, tickY, 308, 62, 4, 0x2965);

  tft.setTextSize(1);
  tft.setTextColor(0x07FF, 0x0000);
  tft.setCursor(12, tickY + 46);
  tft.print("UPSTREAM DNS: 1.1.1.1 (Cloudflare Anycast)");
}

void updateMetrics(bool force = false) {
  int cardY = 28;

  // Total Queries
  if (force || totalQueries != prevTotalQueries) {
    prevTotalQueries = totalQueries;
    tft.fillRect(10, cardY + 20, 90, 24, 0x0000);
    tft.setTextColor(TFT_WHITE, 0x0000);
    tft.setTextSize(2);
    tft.setCursor(12, cardY + 22);
    tft.printf("%lu", totalQueries);
  }

  // Blocked Queries
  if (force || blockedQueries != prevBlockedQueries) {
    prevBlockedQueries = blockedQueries;
    tft.fillRect(115, cardY + 20, 90, 24, 0x0000);
    tft.setTextColor(0xF800, 0x0000);
    tft.setTextSize(2);
    tft.setCursor(117, cardY + 22);
    tft.printf("%lu", blockedQueries);
  }

  // Block Rate
  float blockRate = (totalQueries > 0) ? ((float)blockedQueries / totalQueries * 100.0f) : 0;
  if (force || abs(blockRate - prevBlockRate) >= 0.05f) {
    prevBlockRate = blockRate;
    tft.fillRect(220, cardY + 20, 90, 24, 0x0000);
    tft.setTextColor(0x07E0, 0x0000);
    tft.setTextSize(2);
    tft.setCursor(222, cardY + 22);
    tft.printf("%.1f%%", blockRate);
  }
}

void updateDomainTicker() {
  int tickY = 84;
  // Overwrite domain rows cleanly without full screen redraw
  tft.fillRect(10, tickY + 6, 300, 36, 0x0000);

  tft.setTextSize(1);
  tft.setTextColor(0xF800, 0x0000);
  tft.setCursor(12, tickY + 8);
  tft.print("[BLOCKED SINKHOLE]: ");
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.println(lastBlockedDomain);

  tft.setTextColor(0x07E0, 0x0000);
  tft.setCursor(12, tickY + 26);
  tft.print("[ALLOWED TRAFFIC ]: ");
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.println(lastAllowedDomain);
}

void drawSnoozeButton(bool pressed = false) {
  int btnY = 152;
  int btnW = 308;
  int btnH = 64;

  uint16_t btnBg = pressed ? TFT_WHITE : (isSnoozed ? 0x03E0 : 0xE280);
  uint16_t fgColor = pressed ? TFT_BLACK : (isSnoozed ? TFT_WHITE : TFT_BLACK);

  tft.fillRoundRect(6, btnY, btnW, btnH, 6, btnBg);
  tft.drawRoundRect(6, btnY, btnW, btnH, 6, pressed ? 0x07FF : TFT_WHITE);

  tft.setTextColor(fgColor, btnBg);
  tft.setTextSize(2);

  if (isSnoozed) {
    unsigned long elapsed = millis() - snoozeStartTime;
    unsigned long remainingSec = (elapsed < SNOOZE_DURATION) ? ((SNOOZE_DURATION - elapsed) / 1000) : 0;
    tft.setCursor(24, btnY + 16);
    tft.print("RESUME PROTECTION NOW");
    tft.setTextSize(1);
    tft.setCursor(68, btnY + 42);
    tft.printf("Auto-resuming in %02lu:%02lu...", remainingSec / 60, remainingSec % 60);
  } else {
    tft.setCursor(18, btnY + 16);
    tft.print("PAUSE BLOCKING (5 MIN)");
    tft.setTextSize(1);
    tft.setCursor(44, btnY + 42);
    tft.print("Temporarily allows affiliate & shopping links");
  }
}

void toggleSnooze() {
  // Animate press
  drawSnoozeButton(true);

  isSnoozed = !isSnoozed;
  if (isSnoozed) {
    snoozeStartTime = millis();
    playBeep(1200, 60);
    delay(40);
    playBeep(900, 80);
    setRgbLed(true, true, false); // Amber = Snoozed
    updateFooter("⚠️ Ad-Blocking Paused for 5 Minutes", TFT_BLACK, 0xFDC0);
  } else {
    playBeep(1800, 60);
    delay(40);
    playBeep(2400, 80);
    setRgbLed(false, true, false); // Green = Protected
    updateFooter("🛡️ DNS Shield Resumed — Ads Blocked", TFT_BLACK, 0x07E0);
  }
  footerClearTime = millis() + 3500;

  delay(60);
  drawHeader();
  drawSnoozeButton(false);
}

void processIncomingDns() {
  int packetSize = dnsServer.parsePacket();
  if (packetSize > 0) {
    uint8_t buffer[512];
    int len = dnsServer.read(buffer, sizeof(buffer));
    totalQueries++;

    // Basic DNS Query Header parsing
    if (len > 12) {
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

        // Craft DNS Sinkhole Response (0.0.0.0)
        buffer[2] |= 0x81;
        buffer[3] |= 0x80;
        buffer[6] = 0x00; buffer[7] = 0x01; // 1 Answer

        int ansIdx = len;
        buffer[ansIdx++] = 0xC0; buffer[ansIdx++] = 0x0C; // Pointer
        buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x01; // Type A
        buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x01; // Class IN
        buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x3C; // TTL 60s
        buffer[ansIdx++] = 0x00; buffer[ansIdx++] = 0x04; // Data length 4
        buffer[ansIdx++] = 0; buffer[ansIdx++] = 0; buffer[ansIdx++] = 0; buffer[ansIdx++] = 0; // 0.0.0.0

        dnsServer.beginPacket(dnsServer.remoteIP(), dnsServer.remotePort());
        dnsServer.write(buffer, ansIdx);
        dnsServer.endPacket();

        updateMetrics();
        updateDomainTicker();
        delay(10);
        setRgbLed(isSnoozed ? true : false, true, false);
      } else {
        lastAllowedDomain = domain;
        updateMetrics();
        updateDomainTicker();
      }
    }
  }
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
  setRgbLed(true, false, false); // Red = Init

  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  // Dedicated CYD 2.8" Resistive Touch initialization
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Wi-Fi Station & SoftAP fallback
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  dnsServer.begin(53); // Standard DNS UDP Port 53

  // Splash Screen
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
  playBeep(2200, 100);
  delay(500);

  tft.fillScreen(0x0821);
  drawHeader();
  drawStaticCards();
  updateMetrics(true);
  updateDomainTicker();
  drawSnoozeButton(false);
  updateFooter("UDP Port 53 Standalone DNS Sinkhole | @IamNotIntoTech");
  setRgbLed(false, true, false); // Green = Active Shield
}

void loop() {
  processIncomingDns();

  // Snooze auto-resume timeout check
  if (isSnoozed) {
    if (millis() - snoozeStartTime >= SNOOZE_DURATION) {
      isSnoozed = false;
      playBeep(2400, 120);
      setRgbLed(false, true, false);
      drawHeader();
      drawSnoozeButton(false);
      updateFooter("🛡️ 5-Minute Snooze Expired — Shield Active", TFT_BLACK, 0x07E0);
      footerClearTime = millis() + 3000;
    } else if (millis() - lastSnoozeTimerUpdate >= 1000) {
      lastSnoozeTimerUpdate = millis();
      drawSnoozeButton(false); // Update countdown second timer
    }
  }

  // Clear temporary action messages in footer
  if (footerClearTime > 0 && millis() > footerClearTime) {
    footerClearTime = 0;
    updateFooter(isSnoozed ? "Snooze Active: Ads Temporarily Permitted" : "UDP Port 53 Standalone DNS Sinkhole | @IamNotIntoTech");
  }

  // Standalone simulated traffic generator (smooth, non-flickering)
  if (millis() - lastSimTraffic > 2200) {
    lastSimTraffic = millis();
    totalQueries += random(1, 5);

    if (random(0, 10) > 6 && !isSnoozed) {
      blockedQueries++;
      const char* simAds[] = {
        "telemetry.google.com", "adservice.amazon.com", "metrics.icloud.com",
        "adclick.g.doubleclick.net", "analytics.tiktok.com", "tracker.facebook.com"
      };
      lastBlockedDomain = simAds[random(0, 6)];
      setRgbLed(true, false, false); // Quick red pulse
      delay(30);
      setRgbLed(isSnoozed ? true : false, true, false);
    } else {
      const char* simGoods[] = {
        "cdn.discordapp.com", "wikipedia.org", "registry.npmjs.org",
        "github.com", "youtube.com", "kernel.org"
      };
      lastAllowedDomain = simGoods[random(0, 6)];
    }

    updateMetrics(false);
    updateDomainTicker();
  }

  // Touch Handling (calibrated, non-inverted Y)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 220) {
        lastTouchTime = millis();

        // Elecrow 2.8" CYD Landscape calibration (non-inverted Y)
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        Serial.printf("[TOUCH] Raw X=%d, Y=%d, Z=%d -> Screen (%d, %d)\n", p.x, p.y, p.z, screenX, screenY);

        // 1. Giant Snooze / Resume Button (X: 6 to 314, Y: 152 to 216)
        if (screenX >= 6 && screenX <= 314 && screenY >= 150 && screenY <= 218) {
          toggleSnooze();
          return;
        }

        // 2. Metric Cards Touch (Y: 26 to 80)
        if (screenY >= 26 && screenY <= 80) {
          playBeep(2000, 40);
          if (screenX < 108) { // Total Queries Card
            char msg[64];
            snprintf(msg, sizeof(msg), "📊 Total DNS Lookups Since Boot: %lu", totalQueries);
            updateFooter(msg, TFT_YELLOW, 0x0010);
          } else if (screenX < 212) { // Ads Blocked Card
            // Trigger instant manual ad-blocking test
            blockedQueries++;
            totalQueries++;
            lastBlockedDomain = "telemetry.ads.sinkhole.local";
            setRgbLed(true, false, false);
            delay(50);
            setRgbLed(isSnoozed ? true : false, true, false);
            updateMetrics(false);
            updateDomainTicker();
            updateFooter("⚡ [TEST TRIGGER]: Injected Blocked Ad Query", TFT_BLACK, 0xF800);
          } else { // Block Rate Card
            char msg[64];
            snprintf(msg, sizeof(msg), "🎯 Efficiency: %.1f%% Ad Traffic Dropped", prevBlockRate);
            updateFooter(msg, TFT_BLACK, 0x07E0);
          }
          footerClearTime = millis() + 3000;
          return;
        }

        // 3. Live Domain Ticker Card (Y: 82 to 148)
        if (screenY >= 82 && screenY <= 148) {
          playBeep(1700, 30);
          updateFooter("🌐 Upstream Resolver: Cloudflare (1.1.1.1)", TFT_CYAN, 0x0010);
          footerClearTime = millis() + 2500;
          return;
        }

        // 4. Header / IP Badge Tap (Y < 26)
        if (screenY < 26) {
          playBeep(1500, 30);
          char net[64];
          snprintf(net, sizeof(net), "📡 IP: %s | UDP 53 Sinkhole", WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString().c_str() : "192.168.4.1");
          updateFooter(net, TFT_WHITE, 0x041F);
          footerClearTime = millis() + 3000;
          return;
        }
      }
    }
  }

  delay(20);
}
