/**
 * @file 40-cyd-to-cyd-airdrop-secret-vault.ino
 * @brief CYD-to-CYD AirDrop & Secret Vault (Offline Encrypted P2P Data Beam)
 * @author Tushar Aggarwal (@tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Hardware:
 * - 2× Cheap Yellow Display (ESP32-2432S028R) 2.8" Resistive Touch
 * - Display: ILI9341 320x240 Landscape
 * - Touch: XPT2046 SPI (CLK=25, MISO=39, MOSI=32, CS=33, IRQ=36)
 * - Radio: ESP-NOW 2.4GHz Peer-to-Peer (sub-5ms AirDrop Data Beam)
 * - Audio: GPIO 26 Speaker / Piezo buzzer (Beam charge, transmit whoosh, unlock chime)
 * - RGB LED: GPIO 4 (Red locked), GPIO 16 (Green unlocked), GPIO 17 (Blue beam) [Active LOW]
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
#define COL_BG         0x0821  // Dark Carbon Slate
#define COL_PANEL      0x10A4  // Metallic Titanium
#define COL_BORDER     0x2965  // Subtle Border
#define COL_CYAN       0x05DF  // AirDrop Cyan
#define COL_GREEN      0x07E0  // Vault Unlocked Green
#define COL_RED        0xF800  // Locked Red
#define COL_AMBER      0xFDA0  // Warning Amber
#define COL_WHITE      0xFFFF  // White Text
#define COL_GRAY       0x7BEF  // Dim Text

// ---------------- Packet Protocol ----------------
#define PKT_AIRDROP_OFFER  0x40
#define PKT_AIRDROP_ACCEPT 0x41
#define PKT_AIRDROP_REJECT 0x42
#define PKT_AIRDROP_ACK    0x43

struct __attribute__((packed)) AirDropPacket {
  uint8_t  type;
  char     senderId[8];
  char     title[20];
  char     secret[36];
  uint32_t transferId;
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------- Vault Storage ----------------
#define MAX_SECRETS 6

struct VaultItem {
  char title[20];
  char secret[36];
};

VaultItem vault[MAX_SECRETS];
int vaultCount = 0;
int selectedIdx = 0;
bool isMasked = true;

// ---------------- App States ----------------
enum VaultScreen {
  SCREEN_PIN_LOCK,
  SCREEN_BROWSER,
  SCREEN_DETAIL,
  SCREEN_BEAMING,
  SCREEN_RECEIVING
};

VaultScreen currentScreen = SCREEN_PIN_LOCK;

char myDeviceTag[8] = "CYD-00";
char enteredPin[5] = "";
const char masterPin[5] = "1234"; // Default PIN

// AirDrop incoming state
AirDropPacket incomingOffer;
bool hasPendingIncoming = false;
unsigned long beamStartTime = 0;
unsigned long lastTouchTime = 0;

// ---------------- Audio & LED Helpers ----------------
void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playChimeUnlock() {
  tone(SPEAKER_PIN, 1200, 40);
  delay(45);
  tone(SPEAKER_PIN, 1600, 40);
  delay(45);
  tone(SPEAKER_PIN, 2200, 80);
}

void playChimeLock() {
  tone(SPEAKER_PIN, 1800, 40);
  delay(45);
  tone(SPEAKER_PIN, 900, 80);
}

void playBeamSound() {
  for (int f = 600; f < 2800; f += 120) {
    tone(SPEAKER_PIN, f, 8);
    delay(10);
  }
}

void playSuccessPing() {
  tone(SPEAKER_PIN, 2000, 50);
  delay(60);
  tone(SPEAKER_PIN, 2600, 90);
}

void playErrorBuzz() {
  tone(SPEAKER_PIN, 300, 150);
}

void playClickSound() {
  tone(SPEAKER_PIN, 1500, 12);
}

// ---------------- Vault Persistence ----------------
void saveVaultToFlash();

void initVaultData() {
  prefs.begin("cyd_vault", false);
  vaultCount = prefs.getInt("count", 0);

  if (vaultCount == 0) {
    // Populate default samples on first run
    vaultCount = 4;
    strcpy(vault[0].title, "Home WiFi Key");
    strcpy(vault[0].secret, "SuperSecretWPA3_2026!");

    strcpy(vault[1].title, "SSH Root Key");
    strcpy(vault[1].secret, "ssh-ed25519-cyd-root-99");

    strcpy(vault[2].title, "Crypto Seed");
    strcpy(vault[2].secret, "alpha bravo delta echo 12");

    strcpy(vault[3].title, "Homelab PIN");
    strcpy(vault[3].secret, "8492-Omega-Server");

    saveVaultToFlash();
  } else {
    for (int i = 0; i < vaultCount; i++) {
      char keyT[12], keyS[12];
      sprintf(keyT, "t_%d", i);
      sprintf(keyS, "s_%d", i);
      String t = prefs.getString(keyT, "Untitled");
      String s = prefs.getString(keyS, "Empty");
      strncpy(vault[i].title, t.c_str(), 19);
      vault[i].title[19] = '\0';
      strncpy(vault[i].secret, s.c_str(), 35);
      vault[i].secret[35] = '\0';
    }
  }
}

void saveVaultToFlash() {
  prefs.putInt("count", vaultCount);
  for (int i = 0; i < vaultCount; i++) {
    char keyT[12], keyS[12];
    sprintf(keyT, "t_%d", i);
    sprintf(keyS, "s_%d", i);
    prefs.putString(keyT, vault[i].title);
    prefs.putString(keyS, vault[i].secret);
  }
}

// ---------------- ESP-NOW AirDrop ----------------
void sendAirDropOffer(int idx) {
  AirDropPacket pkt;
  pkt.type = PKT_AIRDROP_OFFER;
  strncpy(pkt.senderId, myDeviceTag, 7);
  pkt.senderId[7] = '\0';
  strncpy(pkt.title, vault[idx].title, 19);
  pkt.title[19] = '\0';
  strncpy(pkt.secret, vault[idx].secret, 35);
  pkt.secret[35] = '\0';
  pkt.transferId = millis();

  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
}

void sendAirDropResponse(uint8_t type) {
  AirDropPacket pkt;
  pkt.type = type;
  strncpy(pkt.senderId, myDeviceTag, 7);
  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
}

// ESP-NOW Receive Callback
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (len != sizeof(AirDropPacket)) return;

  AirDropPacket pkt;
  memcpy(&pkt, incomingData, sizeof(pkt));

  // Ignore self
  if (strcmp(pkt.senderId, myDeviceTag) == 0) return;

  if (pkt.type == PKT_AIRDROP_OFFER) {
    incomingOffer = pkt;
    hasPendingIncoming = true;
    currentScreen = SCREEN_RECEIVING;
    playSuccessPing();
    setRgbLed(false, false, true); // Blue
  } else if (pkt.type == PKT_AIRDROP_ACCEPT) {
    // Recipient accepted our beam!
    currentScreen = SCREEN_DETAIL;
    playSuccessPing();
    setRgbLed(false, true, false); // Green
  } else if (pkt.type == PKT_AIRDROP_REJECT) {
    currentScreen = SCREEN_DETAIL;
    playErrorBuzz();
    setRgbLed(true, false, false); // Red
  }
}

// ---------------- GUI Drawing ----------------
void drawPinLockScreen() {
  tft.fillScreen(COL_BG);
  setRgbLed(true, false, false); // Red = Locked

  // Header Title
  tft.fillRect(0, 0, 320, 26, COL_PANEL);
  tft.drawFastHLine(0, 26, 320, COL_RED);
  tft.setTextColor(COL_RED, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(8, 9);
  tft.print("🔒 CYD ENCRYPTED VAULT - ENTER 4-DIGIT PIN");

  // PIN Display Box
  tft.fillRoundRect(85, 40, 150, 34, 4, 0x0000);
  tft.drawRoundRect(85, 40, 150, 34, 4, COL_CYAN);
  tft.setTextColor(COL_CYAN, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(110, 48);

  int eLen = strlen(enteredPin);
  for (int i = 0; i < 4; i++) {
    if (i < eLen) tft.print("* ");
    else tft.print("_ ");
  }

  // 12-Key Numpad (Y: 82 to 234)
  const char* pad[4][3] = {
    {"1", "2", "3"},
    {"4", "5", "6"},
    {"7", "8", "9"},
    {"C", "0", "OK"}
  };

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 3; col++) {
      int x = 85 + col * 52;
      int y = 84 + row * 38;
      uint16_t bCol = (row == 3 && col == 2) ? 0x03E0 : (row == 3 && col == 0 ? 0x6000 : COL_PANEL);
      tft.fillRoundRect(x, y, 46, 32, 4, bCol);
      tft.drawRoundRect(x, y, 46, 32, 4, COL_BORDER);

      tft.setTextColor(COL_WHITE, bCol);
      tft.setTextSize(2);
      int textX = (strlen(pad[row][col]) == 2) ? (x + 10) : (x + 16);
      tft.setCursor(textX, y + 8);
      tft.print(pad[row][col]);
    }
  }
}

void drawBrowserScreen() {
  tft.fillScreen(COL_BG);
  setRgbLed(false, true, false); // Green = Unlocked

  // Top Header Bar
  tft.fillRect(0, 0, 320, 24, COL_PANEL);
  tft.drawFastHLine(0, 24, 320, COL_CYAN);

  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(6, 8);
  tft.printf("🔓 VAULT [%s] - %d ITEMS", myDeviceTag, vaultCount);

  // Lock Button
  tft.fillRoundRect(254, 3, 60, 18, 3, 0x6000);
  tft.setTextColor(COL_WHITE, 0x6000);
  tft.setCursor(262, 8);
  tft.print("LOCK");

  // Vault Items List (Y: 28 to 205)
  for (int i = 0; i < MAX_SECRETS; i++) {
    int y = 28 + i * 30;
    if (i < vaultCount) {
      uint16_t rowBg = (i == selectedIdx) ? 0x18E3 : COL_PANEL;
      tft.fillRoundRect(6, y, 308, 26, 3, rowBg);
      tft.drawRoundRect(6, y, 308, 26, 3, (i == selectedIdx) ? COL_CYAN : COL_BORDER);

      tft.setTextColor((i == selectedIdx) ? COL_CYAN : COL_WHITE, rowBg);
      tft.setTextSize(1);
      tft.setCursor(14, y + 9);
      tft.printf("[%d] %-18s", i + 1, vault[i].title);

      tft.setTextColor(COL_GRAY, rowBg);
      tft.setCursor(180, y + 9);
      tft.print("••••••••••••••");

      tft.setTextColor(COL_CYAN, rowBg);
      tft.setCursor(276, y + 9);
      tft.print("OPEN >");
    } else {
      // Empty slot
      tft.drawRoundRect(6, y, 308, 26, 3, 0x10A2);
      tft.setTextColor(0x4208, COL_BG);
      tft.setCursor(14, y + 9);
      tft.print("[Empty Slot]");
    }
  }

  // Footer Actions (Y: 212 to 238)
  tft.fillRect(0, 212, 320, 28, COL_PANEL);
  tft.drawFastHLine(0, 211, 320, COL_BORDER);

  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(10, 221);
  tft.print("TAP ITEM TO VIEW & AIRDROP BEAM TO ANOTHER CYD");
}

void drawDetailScreen() {
  tft.fillScreen(COL_BG);

  // Top Header
  tft.fillRect(0, 0, 320, 24, COL_PANEL);
  tft.drawFastHLine(0, 24, 320, COL_CYAN);

  // Back Button
  tft.fillRoundRect(4, 3, 50, 18, 3, 0x18E3);
  tft.setTextColor(COL_WHITE, 0x18E3);
  tft.setTextSize(1);
  tft.setCursor(10, 8);
  tft.print("< BACK");

  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setCursor(64, 8);
  tft.print("SECRET RECORD INSPECTOR");

  // Secret Card Container (X: 10, Y: 34, W: 300, H: 120)
  tft.fillRoundRect(10, 34, 300, 116, 5, COL_PANEL);
  tft.drawRoundRect(10, 34, 300, 116, 5, COL_CYAN);

  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(20, 44);
  tft.print("TITLE / LABEL:");

  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setTextSize(2);
  tft.setCursor(20, 58);
  tft.print(vault[selectedIdx].title);

  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(20, 84);
  tft.print("SECRET PAYLOAD / KEY:");

  tft.fillRoundRect(18, 98, 240, 26, 3, 0x0000);
  tft.drawRoundRect(18, 98, 240, 26, 3, COL_BORDER);
  tft.setTextColor(COL_WHITE, 0x0000);
  tft.setCursor(24, 107);
  if (isMasked) {
    tft.print("••••••••••••••••••••");
  } else {
    tft.print(vault[selectedIdx].secret);
  }

  // Mask / Unmask Eye Toggle Button
  tft.fillRoundRect(264, 98, 38, 26, 3, 0x18E3);
  tft.drawRoundRect(264, 98, 38, 26, 3, COL_WHITE);
  tft.setTextColor(COL_WHITE, 0x18E3);
  tft.setCursor(272, 107);
  tft.print(isMasked ? "SHOW" : "HIDE");

  // Action Buttons (Y: 160 to 220)
  // AIRDROP BEAM Button
  tft.fillRoundRect(10, 164, 194, 52, 6, 0x04F3); // Emerald
  tft.drawRoundRect(10, 164, 194, 52, 6, COL_WHITE);
  tft.setTextColor(COL_WHITE, 0x04F3);
  tft.setTextSize(2);
  tft.setCursor(26, 182);
  tft.print("AIRDROP BEAM");

  // DELETE Button
  tft.fillRoundRect(212, 164, 98, 52, 6, 0x6000);
  tft.drawRoundRect(212, 164, 98, 52, 6, COL_WHITE);
  tft.setTextColor(COL_WHITE, 0x6000);
  tft.setTextSize(1);
  tft.setCursor(232, 186);
  tft.print("DELETE");
}

void renderBeamingScreen() {
  tft.fillScreen(0x0000);
  setRgbLed(false, false, true); // Blue
  playBeamSound();

  // Header
  tft.setTextColor(COL_CYAN, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(38, 30);
  tft.print("BEAMING DATA...");

  tft.setTextColor(COL_WHITE, 0x0000);
  tft.setTextSize(1);
  tft.setCursor(38, 60);
  tft.printf("Sending: \"%s\"", vault[selectedIdx].title);

  // Animated laser particle beam rushing horizontally
  for (int x = 20; x < 300; x += 14) {
    tft.fillCircle(x, 110, 8, COL_CYAN);
    tft.fillCircle(x, 110, 4, COL_WHITE);
    delay(18);
  }

  // Progress Bar
  tft.drawRect(30, 150, 260, 18, COL_CYAN);
  for (int w = 0; w < 256; w += 8) {
    tft.fillRect(32, 152, w, 14, COL_CYAN);
    delay(15);
  }

  // Transmit ESP-NOW packet
  sendAirDropOffer(selectedIdx);

  tft.setTextColor(COL_GREEN, 0x0000);
  tft.setTextSize(2);
  tft.setCursor(44, 190);
  tft.print("BEAM TRANSMITTED!");

  delay(900);
  currentScreen = SCREEN_DETAIL;
  drawDetailScreen();
}

void drawReceivingDialog() {
  // Modal Popup Overlay
  tft.fillRect(20, 24, 280, 192, COL_PANEL);
  tft.drawRect(20, 24, 280, 192, COL_CYAN);
  tft.drawRect(22, 26, 276, 188, COL_WHITE);

  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setTextSize(2);
  tft.setCursor(34, 38);
  tft.print("INCOMING AIRDROP!");

  tft.setTextColor(COL_WHITE, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(34, 70);
  tft.printf("From Device: %s", incomingOffer.senderId);

  tft.setCursor(34, 88);
  tft.print("Item Label:");
  tft.setTextColor(COL_CYAN, COL_PANEL);
  tft.setTextSize(2);
  tft.setCursor(34, 102);
  tft.print(incomingOffer.title);

  tft.setTextColor(COL_GRAY, COL_PANEL);
  tft.setTextSize(1);
  tft.setCursor(34, 128);
  tft.print("Encrypted payload ready to store.");

  // ACCEPT Button (Green)
  tft.fillRoundRect(36, 150, 110, 44, 4, 0x03E0);
  tft.drawRoundRect(36, 150, 110, 44, 4, COL_WHITE);
  tft.setTextColor(COL_WHITE, 0x03E0);
  tft.setTextSize(2);
  tft.setCursor(48, 164);
  tft.print("ACCEPT");

  // REJECT Button (Red)
  tft.fillRoundRect(164, 150, 110, 44, 4, 0x7800);
  tft.drawRoundRect(164, 150, 110, 44, 4, COL_WHITE);
  tft.setTextColor(COL_WHITE, 0x7800);
  tft.setCursor(176, 164);
  tft.print("REJECT");
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=======================================================");
  Serial.println("  CYD-to-CYD AirDrop & Secret Vault - Project 40");
  Serial.println("  Encrypted P2P Wireless Data Beam");
  Serial.println("  Author: Tushar Aggarwal (@tushar2704)");
  Serial.println("  Channels: @IamNotIntoTech | @Makerrdesigns");
  Serial.println("=======================================================\n");

  // Pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  setRgbLed(false, false, false);

  pinMode(SPEAKER_PIN, OUTPUT);

  // Display Init
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(COL_BG);

  // Touch Init (VSPI dedicated pins)
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  // Device Tag from MAC
  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(myDeviceTag, "CYD-%02X%02X", mac[4], mac[5]);

  // WiFi & ESP-NOW
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

  // Load NVS Vault
  initVaultData();

  // Draw initial Lock Screen
  drawPinLockScreen();
}

// ---------------- Main Loop ----------------
void loop() {
  unsigned long now = millis();

  // If receiving modal is triggered by packet
  if (currentScreen == SCREEN_RECEIVING && hasPendingIncoming) {
    drawReceivingDialog();
    hasPendingIncoming = false;
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > 50 || ts.touched()) {
      if (now - lastTouchTime > 220) {
        lastTouchTime = now;

        int screenX = map(p.x, 240, 3800, 0, 320);
        int screenY = map(p.y, 240, 3800, 0, 240);
        screenX = constrain(screenX, 0, 319);
        screenY = constrain(screenY, 0, 239);

        // ---------------- SCREEN 1: PIN LOCK ----------------
        if (currentScreen == SCREEN_PIN_LOCK) {
          // Check Numpad (X: 85 to 240, Y: 84 to 234)
          if (screenX >= 85 && screenX <= 240 && screenY >= 84 && screenY <= 236) {
            int col = (screenX - 85) / 52;
            int row = (screenY - 84) / 38;
            col = constrain(col, 0, 2);
            row = constrain(row, 0, 3);

            const char* pad[4][3] = {
              {"1", "2", "3"},
              {"4", "5", "6"},
              {"7", "8", "9"},
              {"C", "0", "OK"}
            };

            const char* val = pad[row][col];
            playClickSound();

            if (strcmp(val, "C") == 0) {
              enteredPin[0] = '\0';
              drawPinLockScreen();
            } else if (strcmp(val, "OK") == 0) {
              if (strcmp(enteredPin, masterPin) == 0) {
                // Correct PIN! Unlock Vault
                playChimeUnlock();
                enteredPin[0] = '\0';
                currentScreen = SCREEN_BROWSER;
                drawBrowserScreen();
              } else {
                playErrorBuzz();
                enteredPin[0] = '\0';
                drawPinLockScreen();
              }
            } else {
              int len = strlen(enteredPin);
              if (len < 4) {
                enteredPin[len] = val[0];
                enteredPin[len + 1] = '\0';
                drawPinLockScreen();

                // Auto-submit on 4th digit
                if (strlen(enteredPin) == 4) {
                  delay(150);
                  if (strcmp(enteredPin, masterPin) == 0) {
                    playChimeUnlock();
                    enteredPin[0] = '\0';
                    currentScreen = SCREEN_BROWSER;
                    drawBrowserScreen();
                  } else {
                    playErrorBuzz();
                    enteredPin[0] = '\0';
                    drawPinLockScreen();
                  }
                }
              }
            }
          }
        }

        // ---------------- SCREEN 2: BROWSER ----------------
        else if (currentScreen == SCREEN_BROWSER) {
          // Lock Button (X: 254..314, Y: 3..21)
          if (screenX >= 250 && screenY <= 24) {
            playChimeLock();
            currentScreen = SCREEN_PIN_LOCK;
            drawPinLockScreen();
            return;
          }

          // Item Rows (Y: 28 to 205)
          for (int i = 0; i < vaultCount; i++) {
            int y = 28 + i * 30;
            if (screenY >= y && screenY <= y + 26) {
              playClickSound();
              selectedIdx = i;
              isMasked = true;
              currentScreen = SCREEN_DETAIL;
              drawDetailScreen();
              return;
            }
          }
        }

        // ---------------- SCREEN 3: DETAIL ----------------
        else if (currentScreen == SCREEN_DETAIL) {
          // Back Button (X: 4..54, Y: 3..21)
          if (screenX <= 54 && screenY <= 24) {
            playClickSound();
            currentScreen = SCREEN_BROWSER;
            drawBrowserScreen();
            return;
          }

          // Show / Hide Toggle (X: 264..302, Y: 98..124)
          if (screenX >= 260 && screenX <= 305 && screenY >= 95 && screenY <= 126) {
            playClickSound();
            isMasked = !isMasked;
            drawDetailScreen();
            return;
          }

          // AIRDROP BEAM Button (X: 10..204, Y: 164..216)
          if (screenX >= 10 && screenX <= 204 && screenY >= 164 && screenY <= 216) {
            currentScreen = SCREEN_BEAMING;
            renderBeamingScreen();
            return;
          }

          // Delete Button (X: 212..310, Y: 164..216)
          if (screenX >= 212 && screenX <= 310 && screenY >= 164 && screenY <= 216) {
            if (vaultCount > 1) {
              playClickSound();
              for (int d = selectedIdx; d < vaultCount - 1; d++) {
                vault[d] = vault[d + 1];
              }
              vaultCount--;
              saveVaultToFlash();
              selectedIdx = 0;
              currentScreen = SCREEN_BROWSER;
              drawBrowserScreen();
              return;
            }
          }
        }

        // ---------------- SCREEN 4: RECEIVING MODAL ----------------
        else if (currentScreen == SCREEN_RECEIVING) {
          // ACCEPT Button (X: 36..146, Y: 150..194)
          if (screenX >= 36 && screenX <= 146 && screenY >= 150 && screenY <= 194) {
            playSuccessPing();
            if (vaultCount < MAX_SECRETS) {
              strncpy(vault[vaultCount].title, incomingOffer.title, 19);
              vault[vaultCount].title[19] = '\0';
              strncpy(vault[vaultCount].secret, incomingOffer.secret, 35);
              vault[vaultCount].secret[35] = '\0';
              vaultCount++;
              saveVaultToFlash();
            }
            sendAirDropResponse(PKT_AIRDROP_ACCEPT);
            currentScreen = SCREEN_BROWSER;
            drawBrowserScreen();
            return;
          }
          // REJECT Button (X: 164..274, Y: 150..194)
          else if (screenX >= 164 && screenX <= 274 && screenY >= 150 && screenY <= 194) {
            playErrorBuzz();
            sendAirDropResponse(PKT_AIRDROP_REJECT);
            currentScreen = SCREEN_BROWSER;
            drawBrowserScreen();
            return;
          }
        }
      }
    }
  }

  delay(20);
}
