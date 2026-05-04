//=============================BUILDING MENUS/PAGES============================
enum Screen {
  SCREEN_INIT,
  SCREEN_STAT,
  SCREEN_INV,
  SCREEN_DATA,
  SCREEN_MAP,
  SCREEN_RADIO,
  SCREEN_COUNT
};

enum DataPage {
  DATA_PAGE_CURRENT,
  DATA_PAGE_WEATHER,
  DATA_PAGE_SYSTEM,
  DATA_PAGE_COUNT
};

struct TimeState {
  int hh = 0;
  int mm = 0;
  int ss = 0;
  bool valid = false;
};

struct WeatherState {
  float tempF = 0.0f;
  float hum = 0.0f;
  String cond = "UNKNOWN";
  bool online = false;
  unsigned long lastUpdateMs = 0;
};

struct MapState {
  bool available = false;
  bool loading = false;
  bool receiving = false;
  int width = 40;
  int height = 30;
};

struct UIState {
  Screen currentScreen = SCREEN_INIT;
  Screen previousScreen = SCREEN_INIT;

  DataPage currentDataPage = DATA_PAGE_CURRENT;
  DataPage previousDataPage = DATA_PAGE_CURRENT;

  bool screenDirty = true;
  bool layoutDirty = true;
  bool contentDirty = true;
  bool timeDirty = true;

  WeatherState weather;
  TimeState time;
};

struct EncoderState {
  int clkPin;
  int dtPin;
  int swPin;
  uint8_t prevState = 0;
  int8_t movement = 0;
};

UIState ui;
MapState mapState;

const unsigned long ENCODER_DEBOUNCE_MS = 5;

const int MAP_BUF_W = 40;
const int MAP_BUF_H = 30;
uint8_t mapBuffer[MAP_BUF_H][MAP_BUF_W];

//=========================================================

// Additional libraries rendered obsolete as the Pi will handle Wifi connection, time synchronization, 
// music control, and weather information
#include <TFT_eSPI.h>
#include "FS.h"
#include <SPI.h>

#define REPEAT_CAL false
#define Light_green 0x35C2
#define Dark_green 0x0261
#define Time_color 0x04C0
#define PI_RX 16
#define PI_TX 17

// Load GIF library
#include <AnimatedGIF.h>
AnimatedGIF gif;
void GIFDraw(GIFDRAW *pDraw);

#include "images/INIT.h"
#include "images/STAT.h"
#include "images/RADIO.h"
#include "images/DATA_1.h"
#include "images/TIME.h"
#include "images/Bottom_layer_2.h"
#include "images/Date.h"
#include "images/INV.h"
#include "images/temperatureTemp_hum.h"
#include "images/RADIATION.h"
#include "images/temperatureTemp_hum_F.h"
#include "images/vaultboy.h"

#define INIT INIT
#define STAT STAT
#define DATA_1 DATA_1
#define INV INV
#define IN_RADIO
#define IN_RADIO1 35
#define IN_RADIO2 36
//Controls main menu movement
#define MAIN_ENC_CLK 25 //25
#define MAIN_ENC_DT 26 //26
#define MAIN_ENC_SW 27 //27
//Controls submenu movement
#define SUB_ENC_CLK 21 //21
#define SUB_ENC_DT 22 //22
#define SUB_ENC_SW 32 //32

// Forward declarations
void displaySongInfo(uint8_t stationIndex);
void handleRadioStation(uint8_t stationIndex);
void drawMap(bool firstFrame);
void requestMapFromPi();

TFT_eSPI tft = TFT_eSPI();

char currentTitle[64] = "";
char currentArtist[64] = "";
// bool newSongInfo = false;

struct RadioStation {
  const char *name;
  const char *url;
};

EncoderState mainEncoder; 
EncoderState subEncoder;

void initEncoder(EncoderState &enc, int clkPin, int dtPin, int swPin) {
  enc.clkPin = clkPin;
  enc.dtPin = dtPin;
  enc.swPin = swPin;

  pinMode(enc.clkPin, INPUT_PULLUP);
  pinMode(enc.dtPin, INPUT_PULLUP);
  pinMode(enc.swPin, INPUT_PULLUP);

  enc.prevState = (digitalRead(enc.clkPin) << 1) | digitalRead(enc.dtPin);
  enc.movement = 0;
}

int readEncoderStep(EncoderState &enc) {
  static const int8_t transitionTable[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
  };

  uint8_t currState = (digitalRead(enc.clkPin) << 1) | digitalRead(enc.dtPin);
  uint8_t index = (enc.prevState << 2) | currState;

  enc.movement += transitionTable[index];
  enc.prevState = currState;

  if (enc.movement >= 4) {
    enc.movement = 0;
    return 1;
  }
  if (enc.movement <= -4) {
    enc.movement = 0;
    return -1;
  }

  return 0;
}

// Radio stations
const RadioStation stations[] = {
  { "60s and 70s Hits", "http://strm112.1.fm/60s_70s_mobile_mp3" },
  { "70s Greatest Hits", "http://hydra.cdnstream.com/1823_128" },
  { "Q102 80s", "http://wg.cdn.tibus.net/Q10280s" }
};

//=============================SETUP FUNCTION============================

void setup() {
Serial2.begin(115200, SERIAL_8N1, 16, 17);

  //pinMode(IN_RADIO, INPUT_PULLUP);
  pinMode(IN_RADIO1, INPUT_PULLUP);
  pinMode(IN_RADIO2, INPUT_PULLUP);

  pinMode(MAIN_ENC_CLK, INPUT_PULLUP);
  pinMode(MAIN_ENC_DT, INPUT_PULLUP);
  pinMode(MAIN_ENC_SW, INPUT_PULLUP);

  pinMode(SUB_ENC_CLK, INPUT_PULLUP);
  pinMode(SUB_ENC_DT, INPUT_PULLUP);
  pinMode(SUB_ENC_SW, INPUT_PULLUP);

  initEncoder(mainEncoder, MAIN_ENC_CLK, MAIN_ENC_DT, MAIN_ENC_SW);
  initEncoder(subEncoder, SUB_ENC_CLK, SUB_ENC_DT, SUB_ENC_SW);

  Serial.begin(115200);
  delay(1000);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(Light_green, TFT_BLACK);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(Light_green, TFT_BLACK);

  delay(1000);

  gif.begin(BIG_ENDIAN_PIXELS);

  delay(1000);
  Serial2.println("BOOT_SOUND");  //Prompt Pi 0 to play sound

  if (gif.open((uint8_t *)INIT, sizeof(INIT), GIFDraw)) {
    tft.startWrite();  // The TFT chip select is locked low
    while (gif.playFrame(true, NULL)) {
      yield();
    }

    gif.close();
    tft.endWrite();  // Release TFT chip select for other SPI devices
  }
}

//Printing Pi Data
void readPiData() {
  while (Serial2.available()) {
    String msg = Serial2.readStringUntil('\n');
    msg.trim();

    if (msg.length() == 0) {
      continue;
    }

    Serial.println("FROM PI: " + msg);

    if (msg.startsWith("TIME|")) {
      int h, m, s;
      sscanf(msg.c_str(), "TIME|%d|%d|%d", &h, &m, &s);
      ui.time.hh = h;
      ui.time.mm = m;
      ui.time.ss = s;
      ui.time.valid = true;
      ui.timeDirty = true;

      ui.weather.lastUpdateMs = millis();
      ui.weather.online = true;
    }
    else if (msg.startsWith("TEMP|")) {
      ui.weather.tempF = msg.substring(5).toFloat();
      ui.weather.lastUpdateMs = millis();
      ui.weather.online = true;
      ui.contentDirty = true;
    }
    else if (msg.startsWith("HUM|")) {
      ui.weather.hum = msg.substring(4).toFloat();
      ui.weather.lastUpdateMs = millis();
      ui.weather.online = true;
      ui.contentDirty = true;
    }
    else if (msg.startsWith("COND|")) {
      ui.weather.cond = msg.substring(5);
      ui.weather.lastUpdateMs = millis();
      ui.weather.online = true;
      ui.contentDirty = true;
    }
    else if (msg.startsWith("MAP_READY")) {
      mapState.available = true;
      mapState.loading = false;
      ui.layoutDirty = true;
      ui.contentDirty = true;
    }
    else if (msg.startsWith("MAP_UNAVAILABLE")) {
      mapState.available = false;
      mapState.loading = false;
      ui.layoutDirty = true;
      ui.contentDirty = true;
    }
    else if (msg.startsWith("MAP_BEGIN")) {
      int w, h;
      sscanf(msg.c_str(), "MAP_BEGIN|%d|%d", &w, &h);
      mapState.width = w;
      mapState.height = h;
      mapState.receiving = true;
      mapState.available = false;
    }

    else if (msg.startsWith("MAP_ROW|")) {
      int firstSep = msg.indexOf('|');
      int secondSep = msg.indexOf('|', firstSep + 1);
      int thirdSep = msg.indexOf('|', secondSep + 1);

    if (secondSep > 0 && thirdSep > 0) {
      int row = msg.substring(secondSep + 1, thirdSep).toInt();
      String rowData = msg.substring(thirdSep + 1);

    if (row >= 0 && row < MAP_BUF_H) {
      for (int i = 0; i < rowData.length() && i < MAP_BUF_W; i++) {
        mapBuffer[row][i] = (rowData[i] == '1') ? 1 : 0;
      }
    }
  }
}

    else if (msg.startsWith("MAP_END")) {
      mapState.receiving = false;
      mapState.available = true;
      mapState.loading = false;
      ui.layoutDirty = true;
      ui.contentDirty = true;
}
    
  }

  // mark offline if Pi stops talking
  if (millis() - ui.weather.lastUpdateMs > 5000) {
    ui.weather.online = false;
  }
}

void requestMapFromPi() {
  Serial2.println("MAP_REQ");
  Serial.println("TO PI: MAP_REQ");
}

//=============================DRAWING FUNCTION============================

void drawMapBuffer() {
  int originX = 80;
  int originY = 90;
  int pixelSize = 4;

  for (int y = 0; y < mapState.height; y++) {
    for (int x = 0; x < mapState.width; x++) {
      uint16_t color = mapBuffer[y][x] ? Light_green : TFT_BLACK;
      tft.fillRect(originX + x * pixelSize, originY + y * pixelSize, pixelSize, pixelSize, color);
    }
  }
}

void drawGlobalTime() {
  char timeBuffer[6];
  snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", ui.time.hh, ui.time.mm);

  // Clear clock region
  // tft.fillRect(320, 300, 120, 25, Light_green);

  tft.setTextColor(Time_color, TFT_BLACK);
  tft.drawRightString(timeBuffer, 400, 300, 2);
}

void clearDataContentArea(){
  tft.fillRect(35, 80, 410, 115, TFT_BLACK);
}

void drawDataPage(bool firstFrame){
  bool pageChanged = (ui.currentDataPage != ui.previousDataPage);

  if (firstFrame || pageChanged) {
    clearDataContentArea();
    ui.previousDataPage = ui.currentDataPage;
    firstFrame = true;
  }

  switch (ui.currentDataPage) {
    case DATA_PAGE_CURRENT:
      drawDataCurrent(firstFrame);
      break;
    case DATA_PAGE_WEATHER:
      drawDataWeather(firstFrame);
      break;
    case DATA_PAGE_SYSTEM:
      drawDataSystem(firstFrame);
      break;
  }
}

void drawDataCurrent(bool firstFrame) {
  tft.drawBitmap(35, 80, temperatureTemp_hum_F, 408, 29, Light_green);
  char tempBuf[24];
  char humBuf[24];

  snprintf(tempBuf, sizeof(tempBuf), "%.1f F", ui.weather.tempF);
  snprintf(humBuf, sizeof(humBuf), "%.1f %%", ui.weather.hum);

  tft.setTextColor(Time_color, TFT_BLACK);
  tft.drawString(tempBuf, 120, 140, 2);
  tft.drawString(humBuf, 318, 140, 2);
}

void drawDataWeather(bool firstFrame){
  tft.setTextColor(Time_color, TFT_BLACK);
  tft.drawString("WEATHER", 60, 120, 2);
  tft.drawString("COND: ", 60, 160, 2);
  tft.drawString(ui.weather.cond, 110, 160, 2);
}

void drawDataSystem(bool firstFrame){
  tft.setTextColor(Time_color, TFT_BLACK);
  tft.drawString("SYSTEM", 60, 90, 2);
  tft.drawString("STATUS: ", 180, 80, 2);
  tft.drawString(ui.weather.online ? "LIVE" : "OFFLINE", 320, 120, 2);
  if (ui.weather.online) {
    tft.drawString("PI LINK: ON", 60, 160, 2);
  } else {
    tft.drawString("PI LINK: OFF", 60, 160, 2);
  }
}

void drawData(bool firstFrame) {
    //Draws UI elements/headers
    if (firstFrame) {
      tft.fillScreen(TFT_BLACK);
      tft.drawBitmap(35, 300, Bottom_layer_2Bottom_layer_2, 380, 22, Dark_green);
      tft.drawBitmap(35, 300, myBitmapDate, 380, 22, Light_green);
      tft.drawBitmap(200, 200, RadiationRadiation, 62, 61, Light_green);

    if (gif.open((uint8_t *)DATA_1, sizeof(DATA_1), GIFDraw)) {
      tft.startWrite();  // The TFT chip select is locked low
      gif.playFrame(true, NULL);
      gif.close();
      tft.endWrite();  // Release TFT chip select for other SPI devices
    }
      tft.setTextColor(Time_color, TFT_BLACK);
    }
    drawDataPage(firstFrame);
}

void drawStat(bool firstFrame) {
  if (!firstFrame) return;

  tft.fillScreen(TFT_BLACK);
  
  if (gif.open((uint8_t *)STAT, sizeof(STAT), GIFDraw)) {
        tft.startWrite();  // The TFT chip select is locked low
        while(gif.playFrame(true, NULL)){
          yield();
        }
        gif.close();
        tft.endWrite();  // Release TFT chip select for other SPI devices
  }
}

void drawInv(bool firstFrame) {
  if (!firstFrame) return;
    tft.fillScreen(TFT_BLACK);
  

    if (gif.open((uint8_t *)INV, sizeof(INV), GIFDraw)) {
        tft.startWrite();  // The TFT chip select is locked low
        gif.playFrame(true, NULL);
        gif.close();
        tft.endWrite();  // Release TFT chip select for other SPI devices
  }
}

void drawMap(bool firstFrame) {
  if (firstFrame) {
    tft.fillScreen(TFT_BLACK);

    tft.drawRect(25, 55, 430, 230, Light_green);
    tft.drawRect(27, 57, 426, 226, Light_green);

    tft.setTextColor(Time_color, TFT_BLACK);
    tft.drawString("MAP", 35, 40, 2);

    tft.drawBitmap(35, 300, Bottom_layer_2Bottom_layer_2, 380, 22, Dark_green);
    tft.drawBitmap(35, 300, myBitmapDate, 380, 22, Light_green);

    tft.fillRect(40, 70, 400, 200, TFT_BLACK);

    if (!mapState.available && !mapState.loading) {
      tft.drawString("REQUESTING MAP...", 120, 150, 2);
      tft.drawString("WAITING FOR PI...", 120, 170, 2);
      requestMapFromPi();
      mapState.loading = true;
    }
  }

  if (mapState.available) {
    tft.fillRect(110, 145, 220, 40, TFT_BLACK);
    drawMapBuffer();

    int cx = 240;
    int cy = 170;
    tft.drawLine(cx - 8, cy, cx + 8, cy, Light_green);
    tft.drawLine(cx, cy - 8, cx, cy + 8, Light_green);

    tft.setTextColor(Time_color, TFT_BLACK);
    tft.drawString("MAP LOADED", 170, 80, 2);
  }
}

//=============================ENCODER CONTROLS============================

void handleMainEncoder() {
  int step = readEncoderStep(mainEncoder);
  if (step == 0) return;

  Screen oldScreen = ui.currentScreen;

  if (ui.currentScreen == SCREEN_INIT) {
    ui.currentScreen = SCREEN_STAT;
  } else {
    const int firstMenu = SCREEN_STAT;
    const int lastMenu = SCREEN_RADIO;

    int nextScreen = (int)ui.currentScreen;

    if (step > 0) {
      nextScreen++;
      if (nextScreen > lastMenu) nextScreen = firstMenu;
    } else {
      nextScreen--;
      if (nextScreen < firstMenu) nextScreen = lastMenu;
    }

    ui.currentScreen = (Screen)nextScreen;
  }

  if (ui.currentScreen != oldScreen) {
    ui.layoutDirty = true;
    ui.contentDirty = true;
    Serial.print("SCREEN = ");
    Serial.println((int)ui.currentScreen);
  }
}

void handleSubEncoder() {
  int step = readEncoderStep(subEncoder);
  if (step == 0) return;

  DataPage oldPage = ui.currentDataPage;

  if (step > 0) {
    ui.currentDataPage = (DataPage)((ui.currentDataPage + 1) % DATA_PAGE_COUNT);
  } else {
    ui.currentDataPage = (DataPage)((ui.currentDataPage - 1 + DATA_PAGE_COUNT) % DATA_PAGE_COUNT);
  }

  if (ui.currentDataPage != oldPage) {
    ui.layoutDirty = true;
    ui.contentDirty = true;
    Serial.print("DATA PAGE = ");
    Serial.println((int)ui.currentDataPage);
  }
}

void loop() {
  readPiData();
  handleMainEncoder();

  if (ui.currentScreen == SCREEN_DATA) {
    handleSubEncoder();
  }

  bool screenChanged = (ui.currentScreen != ui.previousScreen);
  if (screenChanged) {
    ui.layoutDirty = true;
    ui.contentDirty = true;
    ui.previousScreen = ui.currentScreen;
  }

  if (!ui.layoutDirty && !ui.contentDirty && !ui.timeDirty) {
    return;
  }

  switch (ui.currentScreen) {
    case SCREEN_STAT:
      drawStat(screenChanged || ui.layoutDirty);
      break;

    case SCREEN_INV:
      drawInv(screenChanged || ui.layoutDirty);
      break;

    case SCREEN_DATA:
      drawData(screenChanged || ui.layoutDirty);
      if (ui.timeDirty || screenChanged) {
        drawGlobalTime();
      }
      break;

    case SCREEN_MAP:
      drawMap(screenChanged || ui.layoutDirty);
      if (ui.timeDirty || screenChanged) {
        drawGlobalTime();
      }
      break;

    case SCREEN_RADIO:
      break;
  }

  ui.layoutDirty = false;
  ui.contentDirty = false;
  ui.timeDirty = false;
}

// Function to display the current song info
void displaySongInfo(uint8_t stationIndex) {

  tft.setTextColor(Light_green, TFT_BLACK);
  tft.setTextSize(2);

  // Clear
  tft.fillRect(10, 70, 270, 100, TFT_BLACK);

  // Display station
  tft.drawString("STATION:", 35, 70, 1.5);
  tft.drawString(stations[stationIndex].name, 35, 85, 2);

  // Clear the song info area
  tft.fillRect(35, 190, 480, 100, TFT_BLACK);

  // Display artist
  if (strlen(currentArtist) > 0) {
    tft.drawString("ARTIST:", 35, 190, 1.5);
    tft.drawString(currentArtist, 35, 205, 2);
  }

  // Display title
  if (strlen(currentTitle) > 0) {
    tft.drawString("SONG:", 35, 245, 1.5);
    tft.drawString(currentTitle, 35, 260, 2);
  }
}
