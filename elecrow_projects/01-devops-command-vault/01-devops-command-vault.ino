/**
 * @file 01-devops-command-vault.ino
 * @brief DevOps Command Vault (Docker, Git, Signals, DNS & HTTP Codes)
 * @author Tushar Aggarwal (@tushar2704)
 * @channel IamNotIntoTech (https://youtube.com/@IamNotIntoTech)
 * 
 * Hardware:
 * - Elecrow CrowPanel 2.8" / ESP32-2432S028R
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 Resistive Touch Controller (Dedicated VSPI)
 * - Audio: GPIO 26 Tone feedback
 * - LDR: GPIO 34 Ambient light sensing
 * - RGB LED: GPIO 4 (R), GPIO 16 (G), GPIO 17 (B) [Active LOW]
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
#define LDR_PIN       34
#define SPEAKER_PIN   26

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS);
TFT_eSPI tft = TFT_eSPI();

enum VaultCategory {
  CAT_DOCKER = 0,
  CAT_GIT,
  CAT_SIGNALS,
  CAT_DNS,
  CAT_HTTP,
  CAT_COUNT
};

VaultCategory currentTab = CAT_DOCKER;
int selectedItem = 0;
unsigned long lastTouchTime = 0;

struct VaultEntry {
  const char* title;
  const char* cmd;
  const char* desc;
};

// Docker commands
const VaultEntry dockerEntries[] = {
  {"Run Detached", "docker run -d --name app -p 8080:80 nginx", "Start container in background"},
  {"Exec Shell", "docker exec -it <id> /bin/sh", "Open interactive bash/sh inside container"},
  {"View Logs", "docker logs -f --tail 100 <id>", "Stream live logs with 100 line tail"},
  {"Prune All", "docker system prune -a --volumes -f", "Reclaim disk space: unused imgs & vols"},
  {"Compose Up", "docker compose up -d --build", "Build & start multi-container stack"}
};

// Git Rescue Cheat Sheet
const VaultEntry gitEntries[] = {
  {"Undo Commit", "git reset --soft HEAD~1", "Keep changes staged, rewind commit"},
  {"Rescue Reflog", "git reflog && git reset --hard HEAD@{1}", "Restore lost or deleted commits"},
  {"Detached HEAD", "git checkout -b rescue-branch", "Preserve work done in detached state"},
  {"Stash & Clean", "git stash save 'WIP' && git stash pop", "Temporarily shelter uncommitted files"},
  {"Force Safe", "git push --force-with-lease", "Safe force push checking remote status"}
};

// Linux Signals Reference
const VaultEntry signalEntries[] = {
  {"SIGTERM (15)", "kill -15 <pid>", "Graceful termination request to process"},
  {"SIGKILL (9)",  "kill -9 <pid>",  "Uncatchable immediate OS termination"},
  {"SIGHUP (1)",   "kill -1 <pid>",  "Hangup signal, reloads configs without exit"},
  {"SIGINT (2)",   "Ctrl + C",       "Terminal interrupt signal from keyboard"},
  {"SIGUSR1 (10)", "kill -10 <pid>", "User-defined custom application signal"}
};

// DNS Records Guide
const VaultEntry dnsEntries[] = {
  {"A Record",     "dig +short example.com A",      "IPv4 host address map"},
  {"AAAA Record",  "dig +short example.com AAAA",   "IPv6 128-bit host address"},
  {"CNAME Record", "dig example.com CNAME",         "Canonical alias domain pointer"},
  {"MX Record",    "dig example.com MX",            "Mail exchange server priority table"},
  {"TXT Record",   "dig example.com TXT",           "SPF, DKIM, DMARC security records"}
};

// HTTP Status Codes
const VaultEntry httpEntries[] = {
  {"401 Unauth",   "HTTP 401 Unauthorized",         "Missing or invalid auth credentials"},
  {"403 Forbidden", "HTTP 403 Forbidden",           "Server understood but refuses access"},
  {"404 Not Found", "HTTP 404 Not Found",           "Target resource cannot be found"},
  {"500 Server Err","HTTP 500 Internal Error",      "Unexpected backend server crash"},
  {"502 Bad Gateway","HTTP 502 Bad Gateway",        "Invalid response from upstream service"}
};

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int durationMs) {
  tone(SPEAKER_PIN, freq, durationMs);
  delay(durationMs);
  noTone(SPEAKER_PIN);
}

void drawHeader() {
  // Main title bar
  tft.fillRect(0, 0, 320, 24, 0x0010); // Deep Navy
  tft.setTextColor(0xFFE0, 0x0010);     // Bright Yellow
  tft.setTextSize(1);
  tft.drawString("DEVOPS COMMAND VAULT", 8, 8, 1);
  tft.setTextColor(0x07E0, 0x0010);
  tft.drawRightString("@IamNotIntoTech", 312, 8, 1);

  // Category Tabs (Y: 24..46)
  const char* tabs[CAT_COUNT] = {"DOCKER", "GIT", "SIGNALS", "DNS", "HTTP"};
  int tabWidth = 64;
  for (int i = 0; i < CAT_COUNT; i++) {
    uint16_t bg = (i == (int)currentTab) ? 0xFDC0 : 0x2104;
    uint16_t fg = (i == (int)currentTab) ? TFT_BLACK : TFT_LIGHTGREY;
    tft.fillRect(i * tabWidth, 24, tabWidth - 1, 22, bg);
    tft.drawRect(i * tabWidth, 24, tabWidth - 1, 22, TFT_BLACK);
    tft.setTextColor(fg, bg);
    tft.setTextSize(1);
    tft.drawCentreString(tabs[i], i * tabWidth + (tabWidth / 2), 31, 1);
  }
}

void drawFooter() {
  tft.fillRect(0, 224, 320, 16, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(1);
  tft.drawString("Tap to Inspect & Send to USB Serial | @IamNotIntoTech", 8, 228, 1);
}

void drawContent() {
  // Clear main workspace
  tft.fillRect(0, 48, 320, 175, 0x0821);

  const VaultEntry* currentList = nullptr;
  switch (currentTab) {
    case CAT_DOCKER:  currentList = dockerEntries;  break;
    case CAT_GIT:     currentList = gitEntries;     break;
    case CAT_SIGNALS: currentList = signalEntries;  break;
    case CAT_DNS:     currentList = dnsEntries;     break;
    case CAT_HTTP:    currentList = httpEntries;    break;
    default: break;
  }

  // Draw 5 selectable item buttons on left side
  int listWidth = 110;
  for (int i = 0; i < 5; i++) {
    int y = 50 + i * 26;
    bool isSel = (i == selectedItem);
    uint16_t btnBg = isSel ? 0x041F : 0x18E3;
    uint16_t border = isSel ? 0x07E0 : TFT_DARKGREY;
    tft.fillRoundRect(4, y, listWidth, 24, 3, btnBg);
    tft.drawRoundRect(4, y, listWidth, 24, 3, border);

    tft.setTextColor(isSel ? TFT_YELLOW : TFT_WHITE, btnBg);
    tft.setTextSize(1);
    tft.drawString(currentList[i].title, 8, y + 8, 1);
  }

  // Draw Command Details Card on the right side
  int cardX = 120;
  int cardY = 50;
  int cardW = 196;
  int cardH = 126;

  tft.fillRoundRect(cardX, cardY, cardW, cardH, 4, 0x0000);
  tft.drawRoundRect(cardX, cardY, cardW, cardH, 4, 0x07E0);

  // Card Header: Command Title
  tft.setTextColor(0x07FF, 0x0000);
  tft.setTextSize(1);
  tft.drawString("COMMAND: ", cardX + 8, cardY + 8, 1);
  tft.setTextColor(TFT_WHITE, 0x0000);
  tft.drawString(currentList[selectedItem].title, cardX + 68, cardY + 8, 1);

  // Command Syntax Box
  tft.drawFastHLine(cardX + 4, cardY + 22, cardW - 8, 0x2965);
  tft.fillRect(cardX + 6, cardY + 28, cardW - 12, 36, 0x10A2);
  tft.drawRect(cardX + 6, cardY + 28, cardW - 12, 36, 0x041F);

  tft.setTextColor(0x07E0, 0x10A2);
  tft.drawString(currentList[selectedItem].cmd, cardX + 10, cardY + 34, 1);

  // Description
  tft.setTextColor(0xFDC0, 0x0000);
  tft.drawString("INFO: ", cardX + 8, cardY + 70, 1);
  tft.setTextColor(0xDEFB, 0x0000);
  tft.drawString(currentList[selectedItem].desc, cardX + 44, cardY + 70, 1);

  // "SEND TO CLI" Button
  int actY = 180;
  tft.fillRoundRect(cardX, actY, cardW, 24, 4, 0xE280);
  tft.drawRoundRect(cardX, actY, cardW, 24, 4, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, 0xE280);
  tft.drawCentreString(">> COPY TO USB SERIAL <<", cardX + (cardW / 2), actY + 8, 1);
}

void sendCommandToSerial() {
  const VaultEntry* currentList = nullptr;
  switch (currentTab) {
    case CAT_DOCKER:  currentList = dockerEntries;  break;
    case CAT_GIT:     currentList = gitEntries;     break;
    case CAT_SIGNALS: currentList = signalEntries;  break;
    case CAT_DNS:     currentList = dnsEntries;     break;
    case CAT_HTTP:    currentList = httpEntries;    break;
    default: break;
  }

  Serial.println();
  Serial.println(F("=========================================="));
  Serial.print(F("⚡ [@IamNotIntoTech Vault] Executing: "));
  Serial.println(currentList[selectedItem].title);
  Serial.print(F("CLI Command: "));
  Serial.println(currentList[selectedItem].cmd);
  Serial.println(F("=========================================="));

  setRgbLed(false, false, true); // Blue flash on send
  playBeep(2400, 60);
  delay(100);
  setRgbLed(false, true, false);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(true, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(SPEAKER_PIN, LOW);
  pinMode(LDR_PIN, INPUT);

  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);

  // Initialize LCD
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Dedicated VSPI for Touch Controller (Pins 25, 39, 32, 33)
  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawHeader();
  drawFooter();
  drawContent();
  setRgbLed(false, true, false);
  playBeep(1800, 80);
  Serial.println(F("[SYSTEM] DevOps Command Vault Initialized."));
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (millis() - lastTouchTime > 200) {
        lastTouchTime = millis();

        // Non-inverted landscape coordinates
        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // Check category tab clicks (Y: 24 to 46)
        if (screenY >= 24 && screenY <= 46) {
          int clickedTab = screenX / 64;
          if (clickedTab >= 0 && clickedTab < CAT_COUNT) {
            if (clickedTab != (int)currentTab) {
              currentTab = (VaultCategory)clickedTab;
              selectedItem = 0;
              playBeep(1500, 40);
              drawHeader();
              drawContent();
            }
          }
        }
        // Check left item list clicks (X: 4 to 114, Y: 50 to 180)
        else if (screenX >= 4 && screenX <= 114 && screenY >= 50 && screenY <= 180) {
          int idx = (screenY - 50) / 26;
          if (idx >= 0 && idx < 5) {
            selectedItem = idx;
            playBeep(1200, 30);
            drawContent();
          }
        }
        // Check Action Button click (X: 120 to 316, Y: 180 to 204)
        else if (screenX >= 120 && screenX <= 316 && screenY >= 180 && screenY <= 204) {
          sendCommandToSerial();
        }
      }
    }
  }

  delay(20);
}

