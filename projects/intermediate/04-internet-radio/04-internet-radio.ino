/**
 * @file 04-internet-radio.ino
 * @brief Live WiFi Internet Radio Player for CYD 2.8" (ESP32-2432S028R)
 * @author Tushar Aggarwal (tushar2704)
 * @repository https://github.com/tushar2704/Cheap-Yellow-Display-Community
 * 
 * 📺 Follow on YouTube:
 * - Tech & Coding: https://youtube.com/@IamNotIntoTech
 * - 3D Printing & Enclosure: https://youtube.com/@Makerrdesigns
 * 
 * Streams live audio over WiFi to the CYD onboard speaker via internal DAC (GPIO 26).
 */

#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "Audio.h" // ESP32-audioI2S library

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// CYD 2.8" Touchscreen Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 13
#define XPT2046_MISO 12
#define XPT2046_CLK 14
#define XPT2046_CS 33

// Onboard Audio DAC Pin (Speaker is wired to GPIO 26)
#define CYD_SPEAKER_DAC 26

#define TS_MINX 300
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3700

SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
Audio audio(true, CYD_SPEAKER_DAC); // Use internal DAC mode on GPIO 26

// Curated Stream List
struct RadioStation {
  const char* name;
  const char* genre;
  const char* url;
};

RadioStation stations[4] = {
  {"Lofi Girl Radio", "Beats / Chill", "http://stream.zeno.fm/f3wvbbqmdg8uv"},
  {"BBC World Service", "News / Talk", "http://stream.live.vc.bbcmedia.co.uk/bbc_world_service"},
  {"Classical Chill", "Instrumental", "http://stream.srg-ssr.ch/m/rsc_de/mp3_128"},
  {"Retro 80s Synth", "Electronic", "http://stream.zeno.fm/0r0xa792kwzuv"}
};

int currentStationIdx = 0;
int currentVolume = 12; // 0 to 21
bool isPlaying = true;
String currentTrackTitle = "Streaming live...";

void drawRadioUI() {
  tft.fillScreen(0x0821);

  // Header Bar
  tft.fillRect(0, 0, 320, 34, 0x18C3);
  tft.setTextColor(TFT_WHITE, 0x18C3);
  tft.setTextSize(2);
  tft.setCursor(8, 9);
  tft.print("CYD 2.8\" Web Radio");

  // Station Info Card
  tft.fillRoundRect(12, 44, 296, 76, 8, 0x10A2);
  tft.drawRoundRect(12, 44, 296, 76, 8, 0xFDC0); // CYD Yellow

  tft.setTextColor(0xFDC0, 0x10A2);
  tft.setTextSize(2);
  tft.setCursor(24, 54);
  tft.print(stations[currentStationIdx].name);

  tft.setTextColor(TFT_CYAN, 0x10A2);
  tft.setTextSize(1);
  tft.setCursor(24, 78);
  tft.printf("Genre: %s", stations[currentStationIdx].genre);

  tft.setTextColor(TFT_LIGHTGREY, 0x10A2);
  tft.setCursor(24, 94);
  tft.print(currentTrackTitle.substring(0, 36));

  // Station Switcher Buttons (PREV / NEXT)
  tft.fillRoundRect(12, 130, 94, 38, 6, 0x2104);
  tft.drawRoundRect(12, 130, 94, 38, 6, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setTextSize(2);
  tft.setCursor(24, 140);
  tft.print("< PREV");

  // Play / Pause Button
  tft.fillRoundRect(113, 130, 94, 38, 6, isPlaying ? 0x03E0 : 0x8000);
  tft.setTextColor(TFT_WHITE, isPlaying ? 0x03E0 : 0x8000);
  tft.setCursor(125, 140);
  tft.print(isPlaying ? "PAUSE" : "PLAY");

  // Next Button
  tft.fillRoundRect(214, 130, 94, 38, 6, 0x2104);
  tft.drawRoundRect(214, 130, 94, 38, 6, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, 0x2104);
  tft.setCursor(226, 140);
  tft.print("NEXT >");

  // Volume Bar
  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setTextSize(1);
  tft.setCursor(14, 180);
  tft.print("VOLUME:");

  int volW = map(currentVolume, 0, 21, 0, 210);
  tft.fillRect(70, 178, volW, 14, 0xFDC0);
  tft.fillRect(70 + volW, 178, 210 - volW, 14, 0x2104);
  tft.drawRect(70, 178, 210, 14, TFT_DARKGREY);

  tft.setTextColor(TFT_WHITE, 0x0821);
  tft.setCursor(288, 180);
  tft.printf("%d", currentVolume);

  // Footer
  tft.fillRect(0, 222, 320, 18, TFT_BLACK);
  tft.setTextColor(0x07E0, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(6, 226);
  tft.print("CYD Community | @IamNotIntoTech | @Makerrdesigns");
}

void switchStation(int delta) {
  currentStationIdx = (currentStationIdx + delta + 4) % 4;
  audio.connecttohost(stations[currentStationIdx].url);
  currentTrackTitle = "Connecting...";
  drawRadioUI();
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x0821);

  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  tft.setTextColor(0xFDC0, 0x0821);
  tft.setTextSize(2);
  tft.setCursor(30, 100);
  tft.print("Connecting WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 25) {
    delay(400);
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    audio.setVolume(currentVolume);
    audio.connecttohost(stations[currentStationIdx].url);
    drawRadioUI();
  } else {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(20, 100);
    tft.print("WiFi Failed! Check credentials.");
  }
}

void loop() {
  audio.loop();

  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    int y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

    // Prev Button (12, 130, 94, 38)
    if (x >= 12 && x <= 106 && y >= 130 && y <= 168) {
      switchStation(-1);
      delay(250);
    }
    // Play/Pause Button (113, 130, 94, 38)
    else if (x >= 113 && x <= 207 && y >= 130 && y <= 168) {
      isPlaying = !isPlaying;
      audio.pauseResume();
      drawRadioUI();
      delay(250);
    }
    // Next Button (214, 130, 94, 38)
    else if (x >= 214 && x <= 308 && y >= 130 && y <= 168) {
      switchStation(1);
      delay(250);
    }
    // Volume Bar (70, 178, 210, 14)
    else if (x >= 70 && x <= 280 && y >= 170 && y <= 200) {
      currentVolume = constrain(map(x, 70, 280, 0, 21), 0, 21);
      audio.setVolume(currentVolume);
      drawRadioUI();
      delay(150);
    }
  }
}

// Audio callbacks
void audio_showstation(const char *info){
  currentTrackTitle = String(info);
}

void audio_showstreamtitle(const char *info){
  currentTrackTitle = String(info);
  drawRadioUI();
}
