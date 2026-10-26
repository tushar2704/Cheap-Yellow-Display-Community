/**
 * @file main.cpp
 * @brief 34 - CYD-to-CYD Walkie-Talkie & Acoustic Voice Intercom
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * Community Channels:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Hardware:
 * - Two or more ESP32-2432S028R Cheap Yellow Displays
 * - Protocol: Raw ESP-NOW 2.4GHz Peer-to-Peer
 * - Audio Output: FM8002A Amplifier on GPIO 26
 * - Visual Alerts: RGB LED (GPIO 4, 16, 17)
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

// Packet Definitions
#define AUDIO_PKT_TONE   0x01
#define AUDIO_PKT_CHIME  0x02
#define AUDIO_PKT_PING   0x03

struct __attribute__((packed)) WalkiePacket {
  uint8_t msgType;
  uint16_t freq;
  uint16_t durationMs;
  uint8_t volume;
  char caller[10];
};

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

char myCallsign[10] = "CYD-UNIT1";
char peerCallsign[10] = "LISTENING";
unsigned long lastPeerSeen = 0;

// Intercom State
bool isTransmitting = false;
bool isReceiving = false;
unsigned long rxAudioUntil = 0;
uint16_t selectedToneFreq = 1000;
uint8_t volumeLevel = 80;

// Spectrum Bar Heights
int barHeights[16] = {0};

void setRgb(bool r, bool g, bool b) {
  digitalWrite(LED_RED, r ? LOW : HIGH);
  digitalWrite(LED_GREEN, g ? LOW : HIGH);
  digitalWrite(LED_BLUE, b ? LOW : HIGH);
}

void playBeep(int freq, int dur) {
  tone(SPEAKER_PIN, freq, dur);
}

void sendWalkiePacket(uint8_t type, uint16_t freq, uint16_t dur) {
  WalkiePacket pkt;
  pkt.msgType = type;
  pkt.freq = freq;
  pkt.durationMs = dur;
  pkt.volume = volumeLevel;
  strncpy(pkt.caller, myCallsign, 9);
  pkt.caller[9] = '\0';
  esp_now_send(broadcastMac, (uint8_t*)&pkt, sizeof(pkt));
}

// Receive Callback
void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len == sizeof(WalkiePacket)) {
    WalkiePacket pkt;
    memcpy(&pkt, data, sizeof(pkt));
    lastPeerSeen = millis();
    strncpy(peerCallsign, pkt.caller, 9);
    peerCallsign[9] = '\0';

    if (pkt.msgType == AUDIO_PKT_TONE) {
      isReceiving = true;
      rxAudioUntil = millis() + pkt.durationMs;
      tone(SPEAKER_PIN, pkt.freq, pkt.durationMs);
      setRgb(false, false, true); // Blue on RX
    } else if (pkt.msgType == AUDIO_PKT_CHIME) {
      isReceiving = true;
      rxAudioUntil = millis() + 400;
      setRgb(false, true, true);
      playBeep(1200, 100);
      delay(110);
      playBeep(1800, 150);
      setRgb(false, false, false);
    }
  }
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 26, 0x1082); // Dark Navy Bar

  tft.setTextColor(0xFDC0, 0x1082);
  tft.setTextSize(1);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(String("INTERCOM: ") + myCallsign, 8, 13);

  bool linked = (millis() - lastPeerSeen < 4000);
  tft.setTextDatum(MR_DATUM);
  if (linked) {
    tft.setTextColor(0x07E0, 0x1082);
    tft.drawString(String("LINKED: ") + peerCallsign, 312, 13);
  } else {
    tft.setTextColor(TFT_LIGHTGREY, 0x1082);
    tft.drawString("PEER: SCANNING...", 312, 13);
  }
}

void drawPttStatus() {
  int pttX = 16, pttY = 32, pttW = 288, pttH = 64;

  if (isTransmitting) {
    tft.fillRoundRect(pttX, pttY, pttW, pttH, 8, 0xF800); // Red TX
    tft.setTextColor(TFT_WHITE, 0xF800);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("TRANSMITTING (PTT ACTIVE)", 160, 56);
    tft.setTextSize(1);
    tft.drawString(String("Frequency: ") + selectedToneFreq + " Hz", 160, 80);
  } else if (isReceiving) {
    tft.fillRoundRect(pttX, pttY, pttW, pttH, 8, 0x001F); // Blue RX
    tft.setTextColor(TFT_WHITE, 0x001F);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("RECEIVING AUDIO...", 160, 56);
    tft.setTextSize(1);
    tft.drawString(String("From: ") + peerCallsign, 160, 80);
  } else {
    tft.fillRoundRect(pttX, pttY, pttW, pttH, 8, 0x07E0); // Green Standby
    tft.setTextColor(TFT_BLACK, 0x07E0);
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("HOLD TO TALK (PTT)", 160, 56);
    tft.setTextSize(1);
    tft.drawString("Press anywhere inside this box to transmit", 160, 80);
  }
}

void drawSpectrumBars() {
  int startX = 16, startY = 104, totalW = 288, totalH = 40;
  tft.fillRect(startX, startY, totalW, totalH, 0x0821);

  int barW = 14;
  for (int i = 0; i < 16; i++) {
    int x = startX + i * 18;
    int h = (isTransmitting || isReceiving) ? random(6, totalH) : 2;
    uint16_t col = (i < 10) ? 0x07E0 : (i < 13 ? 0xFDC0 : 0xF800);
    tft.fillRect(x, startY + totalH - h, barW, h, col);
  }
}

void drawControls() {
  // Quick Call Buttons
  tft.fillRoundRect(16, 150, 92, 36, 4, 0x18C3);
  tft.drawRoundRect(16, 150, 92, 36, 4, 0x39C7);
  tft.setTextColor(0xFDC0, 0x18C3);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CALL CHIME", 62, 168);

  tft.fillRoundRect(114, 150, 92, 36, 4, 0x18C3);
  tft.drawRoundRect(114, 150, 92, 36, 4, 0x39C7);
  tft.setTextColor(0x07FF, 0x18C3);
  tft.drawString("ROGER BEEP", 160, 168);

  tft.fillRoundRect(212, 150, 92, 36, 4, 0x2104);
  tft.drawRoundRect(212, 150, 92, 36, 4, 0xF800);
  tft.setTextColor(0xF800, 0x2104);
  tft.drawString("SOS SIREN", 258, 168);

  // Tone Frequency Selector (400, 800, 1200, 2000 Hz)
  const uint16_t freqs[] = {400, 800, 1200, 2000};
  for (int i = 0; i < 4; i++) {
    int x = 16 + i * 72;
    int y = 194;
    bool isSel = (selectedToneFreq == freqs[i]);
    uint16_t bg = isSel ? 0xFDC0 : 0x18C3;
    uint16_t fg = isSel ? TFT_BLACK : TFT_WHITE;

    tft.fillRoundRect(x, y, 68, 38, 4, bg);
    tft.drawRoundRect(x, y, 68, 38, 4, 0x39C7);
    tft.setTextColor(fg, bg);
    tft.drawString(String(freqs[i]) + "Hz", x + 34, y + 19);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[CYD-34] CYD Walkie-Talkie & Intercom Initializing...");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  setRgb(false, false, false);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(myCallsign, sizeof(myCallsign), "CYD-%02X%02X", mac[4], mac[5]);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_now_init();
  esp_now_register_recv_cb(onDataRecv);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  drawHeader();
  drawPttStatus();
  drawSpectrumBars();
  drawControls();

  playBeep(1200, 60);
  delay(80);
  playBeep(1800, 80);
}

unsigned long lastPing = 0;
unsigned long lastSpecUpdate = 0;

void loop() {
  // Clear RX state after playback duration
  if (isReceiving && millis() > rxAudioUntil) {
    isReceiving = false;
    setRgb(false, false, false);
    drawPttStatus();
  }

  // Periodic Heartbeat
  if (millis() - lastPing > 2500) {
    lastPing = millis();
    sendWalkiePacket(AUDIO_PKT_PING, 0, 0);
    drawHeader();
  }

  // Animated Spectrum Bar Refresh
  if (millis() - lastSpecUpdate > 60) {
    lastSpecUpdate = millis();
    drawSpectrumBars();
  }

  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // PTT Box (16, 32, 288, 64)
    if (x >= 16 && x <= 304 && y >= 32 && y <= 96) {
      if (!isTransmitting) {
        isTransmitting = true;
        setRgb(true, false, false); // Red on TX
        drawPttStatus();
      }
      tone(SPEAKER_PIN, selectedToneFreq);
      sendWalkiePacket(AUDIO_PKT_TONE, selectedToneFreq, 80);
      delay(40);
      return;
    }

    // Call Chime (16, 150, 92, 36)
    if (x >= 16 && x <= 108 && y >= 150 && y <= 186) {
      sendWalkiePacket(AUDIO_PKT_CHIME, 0, 0);
      playBeep(1200, 80);
      delay(90);
      playBeep(1800, 100);
      delay(200);
      return;
    }

    // Roger Beep (114, 150, 92, 36)
    if (x >= 114 && x <= 206 && y >= 150 && y <= 186) {
      sendWalkiePacket(AUDIO_PKT_TONE, 2200, 120);
      playBeep(2200, 120);
      delay(200);
      return;
    }

    // SOS Siren (212, 150, 92, 36)
    if (x >= 212 && x <= 304 && y >= 150 && y <= 186) {
      for (int f = 800; f <= 2400; f += 200) {
        sendWalkiePacket(AUDIO_PKT_TONE, f, 30);
        tone(SPEAKER_PIN, f, 30);
        delay(35);
      }
      delay(200);
      return;
    }

    // Tone Frequencies (y >= 194)
    if (y >= 194 && y <= 232) {
      const uint16_t freqs[] = {400, 800, 1200, 2000};
      for (int i = 0; i < 4; i++) {
        int fx = 16 + i * 72;
        if (x >= fx && x <= fx + 68) {
          selectedToneFreq = freqs[i];
          drawControls();
          playBeep(selectedToneFreq, 40);
          delay(180);
          break;
        }
      }
      return;
    }
  } else {
    // Finger released PTT
    if (isTransmitting) {
      noTone(SPEAKER_PIN);
      isTransmitting = false;
      setRgb(false, false, false);
      drawPttStatus();
    }
  }

  delay(15);
}
