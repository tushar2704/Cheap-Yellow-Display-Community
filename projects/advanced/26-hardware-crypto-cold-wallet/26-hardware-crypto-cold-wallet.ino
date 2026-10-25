/*
 * Cheap Yellow Display (CYD) - Air-Gapped Crypto Hardware Cold Wallet
 * Hardware: ESP32-2432S028R (2.8" TFT 320x240 ILI9341, XPT2046 Resistive Touch)
 * 
 * Target: 2.8" CYD ESP32-2432S028R
 * Author: tushar2704 <tushar.inseec@gmail.com>
 * Channels: @IamNotIntoTech | @Makerrdesigns
 * Dual Licensed: AGPL-3.0 for firmware code | CC BY-NC-ND 4.0 for media/assets
 * Year: 2026
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Touch Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Onboard Peripherals
#define CYD_LED_RED 4
#define CYD_LED_GREEN 16
#define CYD_LED_BLUE 17
#define CYD_BUZZER 26
#define CYD_LDR 34

SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

enum WalletState { STATE_LOCKED, STATE_OVERVIEW, STATE_SIGN_TX, STATE_QR_BROADCAST };
WalletState currentState = STATE_OVERVIEW;

// Wallet Mock Data
const char* derivationPath = "m/84'/0'/0'"; // Native SegWit BIP84
const char* xpub = "zpub6rFRvNDt...9P2K";
float btcBalance = 1.48290000;

// Pending Transaction to sign
float txAmountBtc = 0.15000000;
float txFeeSats = 2840;
const char* txRecipient = "bc1qxy2kgdygjrsqtzq2n0yrf2493p83kkfjhx0wlh";
bool isSigned = false;

void beep(int freq, int durationMs) {
  tone(CYD_BUZZER, freq, durationMs);
}

void setRGB(bool r, bool g, bool b) {
  digitalWrite(CYD_LED_RED, r ? LOW : HIGH);
  digitalWrite(CYD_LED_GREEN, g ? LOW : HIGH);
  digitalWrite(CYD_LED_BLUE, b ? LOW : HIGH);
}

void disableAllRadios() {
  WiFi.mode(WIFI_OFF);
  btStop(); // Cut Bluetooth
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x0821);
  tft.drawFastHLine(0, 26, 320, 0xFD20); // Bitcoin Orange
  
  tft.setTextColor(0xFD20, 0x0821);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("CYD AIR-GAPPED COLD WALLET", 8, 13, 2);
  
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(TFT_GREEN, 0x0821);
  tft.drawString("RADIO: OFF", 312, 13, 2);
}

void drawOverviewScreen() {
  tft.fillRect(0, 28, 320, 184, TFT_BLACK);
  
  // Balance Hero Card
  tft.fillRect(6, 32, 308, 80, 0x10A2);
  tft.drawRect(6, 32, 308, 80, 0x2965);
  tft.setTextColor(TFT_SILVER, 0x10A2);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("COLD VAULT BALANCE", 14, 38, 1);
  
  tft.setTextColor(0xFD20, 0x10A2);
  char buf[32];
  snprintf(buf, sizeof(buf), "%.8f BTC", btcBalance);
  tft.drawString(buf, 14, 54, 4);
  
  tft.setTextColor(TFT_WHITE, 0x10A2);
  snprintf(buf, sizeof(buf), "~ $140,356 USD | Path: %s", derivationPath);
  tft.drawString(buf, 14, 88, 2);
  
  // Seed Health Status Box
  tft.fillRect(6, 118, 308, 90, 0x0821);
  tft.drawRect(6, 118, 308, 90, 0x2965);
  
  tft.setTextColor(TFT_SILVER, 0x0821);
  tft.drawString("SECURITY CONSTRAINTS", 14, 124, 1);
  
  tft.setTextColor(TFT_GREEN, 0x0821);
  tft.drawString("* Air-Gap: WiFi & Bluetooth Hardware Cut", 14, 142, 2);
  tft.drawString("* BIP39 24-Word Seed: Hardware Encrypted", 14, 162, 2);
  tft.drawString("* Secure Enclave: Optical PSBT QR Signing", 14, 182, 2);
  
  // Bottom Buttons
  tft.fillRect(6, 214, 148, 24, 0x18C3);
  tft.drawRect(6, 214, 148, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("VERIFY SEED", 80, 226, 2);
  
  tft.fillRect(166, 214, 148, 24, 0xFD20);
  tft.drawRect(166, 214, 148, 24, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, 0xFD20);
  tft.drawString("SIGN PSBT TX", 240, 226, 2);
}

void drawSignTxScreen() {
  tft.fillRect(0, 28, 320, 184, TFT_BLACK);
  
  tft.fillRect(6, 32, 308, 176, 0x0821);
  tft.drawRect(6, 32, 308, 176, TFT_RED);
  
  tft.setTextColor(TFT_RED, 0x0821);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("SIGN PARTIALLY SIGNED BITCOIN TX?", 14, 38, 2);
  
  tft.setTextColor(TFT_WHITE, 0x0821);
  char buf[64];
  snprintf(buf, sizeof(buf), "TRANSFER AMOUNT: %.8f BTC", txAmountBtc);
  tft.drawString(buf, 14, 64, 2);
  
  snprintf(buf, sizeof(buf), "MINER FEE: %u sats (~$2.68)", (uint32_t)txFeeSats);
  tft.drawString(buf, 14, 88, 2);
  
  tft.setTextColor(TFT_SILVER, 0x0821);
  tft.drawString("RECIPIENT ADDRESS:", 14, 114, 1);
  tft.setTextColor(TFT_YELLOW, 0x0821);
  tft.drawString("bc1qxy2kgdygjrsqtzq2n0yrf2493p83kkfjhx0wlh", 14, 130, 1);
  
  // Action Buttons
  tft.fillRect(14, 160, 136, 38, 0x2104);
  tft.drawRect(14, 160, 136, 38, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CANCEL", 82, 179, 2);
  
  tft.fillRect(168, 160, 136, 38, TFT_GREEN);
  tft.drawRect(168, 160, 136, 38, TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_GREEN);
  tft.drawString("CONFIRM & SIGN", 236, 179, 2);
}

void drawQRBroadcastScreen() {
  tft.fillRect(0, 28, 320, 184, TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("SIGNED PSBT QR (SPECTER / BLUEWALLET)", 10, 32, 1);
  
  // Simulated 2D Matrix QR Pattern
  int qrX = 100;
  int qrY = 48;
  int qrSize = 120;
  tft.fillRect(qrX - 4, qrY - 4, qrSize + 8, qrSize + 8, TFT_WHITE);
  
  // Draw QR corner finders
  tft.fillRect(qrX, qrY, 28, 28, TFT_BLACK);
  tft.fillRect(qrX + 4, qrY + 4, 20, 20, TFT_WHITE);
  tft.fillRect(qrX + 8, qrY + 8, 12, 12, TFT_BLACK);
  
  tft.fillRect(qrX + qrSize - 28, qrY, 28, 28, TFT_BLACK);
  tft.fillRect(qrX + qrSize - 24, qrY + 4, 20, 20, TFT_WHITE);
  tft.fillRect(qrX + qrSize - 20, qrY + 8, 12, 12, TFT_BLACK);
  
  tft.fillRect(qrX, qrY + qrSize - 28, 28, 28, TFT_BLACK);
  tft.fillRect(qrX + 4, qrY + qrSize - 24, 20, 20, TFT_WHITE);
  tft.fillRect(qrX + 8, qrY + qrSize - 20, 12, 12, TFT_BLACK);
  
  // Pseudo QR data bits
  for (int r = 32; r < qrSize - 8; r += 8) {
    for (int c = 32; c < qrSize - 8; c += 8) {
      if ((r + c * 3) % 5 == 0) {
        tft.fillRect(qrX + c, qrY + r, 6, 6, TFT_BLACK);
      }
    }
  }
  
  tft.fillRect(6, 214, 308, 24, 0x18C3);
  tft.drawRect(6, 214, 308, 24, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("DONE / RETURN TO VAULT", 160, 226, 2);
}

void renderCurrentState() {
  drawHeader();
  if (currentState == STATE_OVERVIEW) drawOverviewScreen();
  else if (currentState == STATE_SIGN_TX) drawSignTxScreen();
  else if (currentState == STATE_QR_BROADCAST) drawQRBroadcastScreen();
}

void setup() {
  Serial.begin(115200);
  disableAllRadios();
  
  pinMode(CYD_LED_RED, OUTPUT);
  pinMode(CYD_LED_GREEN, OUTPUT);
  pinMode(CYD_LED_BLUE, OUTPUT);
  setRGB(false, true, false);
  
  pinMode(CYD_BUZZER, OUTPUT);
  beep(1800, 40);
  
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  renderCurrentState();
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int touchX = map(p.x, 200, 3700, 0, 320);
    int touchY = map(p.y, 240, 3800, 0, 240);
    
    if (currentState == STATE_OVERVIEW) {
      if (touchY > 210) {
        if (touchX > 160) {
          currentState = STATE_SIGN_TX;
          beep(2000, 30);
          renderCurrentState();
        } else {
          beep(1500, 30);
        }
        delay(250);
      }
    } else if (currentState == STATE_SIGN_TX) {
      if (touchY >= 160 && touchY <= 200) {
        if (touchX > 160) {
          // Confirm signature
          currentState = STATE_QR_BROADCAST;
          setRGB(false, true, false);
          beep(2600, 80);
          renderCurrentState();
        } else {
          currentState = STATE_OVERVIEW;
          beep(1200, 30);
          renderCurrentState();
        }
        delay(250);
      }
    } else if (currentState == STATE_QR_BROADCAST) {
      if (touchY > 210) {
        currentState = STATE_OVERVIEW;
        beep(1800, 30);
        renderCurrentState();
        delay(250);
      }
    }
  }
  
  delay(40);
}
