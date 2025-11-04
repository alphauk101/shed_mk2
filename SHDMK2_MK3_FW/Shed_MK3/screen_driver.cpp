#include "screen_driver.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "shdmk3_config.h"
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>

//#define DEBUG_SCREEN

#define TITLE_FONT &FreeSans12pt7b
#define DEFAULT_FONT &FreeSans9pt7b

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN);


void SCRNDRV::init() {
  //setup reset and backlight pins.
  pinMode(BACKLIGHT_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  digitalWrite(BACKLIGHT_PIN, BACKLIGHT_ON);
  digitalWrite(RESET_PIN, HIGH);

  //Reset the screen to ensure ready state
  this->doReset();

  tft.begin();

  this->setDefaultScreen();

#ifdef DEBUG_SCREEN
  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x");
  Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x");
  Serial.println(x, HEX);
#endif
}

void SCRNDRV::setDefaultScreen() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setRotation(DEFAULT_ORIENTATION);
}


void SCRNDRV::setStartUpMessage() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setCursor(15, 20);
  tft.setTextColor(ILI9341_BLACK);
  tft.setFont(TITLE_FONT);
  tft.println(STARTUP_MESSAGE);
  tft.setFont(DEFAULT_FONT);
  tft.drawFastHLine(0, 40, 320, ILI9341_BLACK);
  tft.drawFastHLine(0, 41, 320, ILI9341_BLACK);
}

#define FONT_HEIGHT 20
void SCRNDRV::updateStartUpMessage(const std::string& IOexp_sts,           //IO expander status
                                   const std::string& intTemperature_sts,  //Internal temperature
                                   const std::string& intHumidity_sts,     //Internal humidity
                                   const std::string& extTemperature_sts,  //External temperature
                                   const std::string& led_driver) {        //LED driver


  //Position for IO status

  int Y_cursor = 80;

  if (!IOexp_sts.empty()) {
    tft.setCursor(50, Y_cursor);
    tft.println(IOexp_sts.c_str());
  }
  Y_cursor += FONT_HEIGHT;
  //Position for int temp status
  if (!intTemperature_sts.empty()) {
    tft.setCursor(50, Y_cursor);
    tft.println(intTemperature_sts.c_str());
  }
  Y_cursor += FONT_HEIGHT;
  //Position for int hum status
  if (!intHumidity_sts.empty()) {
    tft.setCursor(50, Y_cursor);
    tft.println(intHumidity_sts.c_str());
  }
  Y_cursor += FONT_HEIGHT;
  //Position for ext temp status
  if (!extTemperature_sts.empty()) {
    tft.setCursor(50, Y_cursor);
    tft.println(extTemperature_sts.c_str());
  }
  Y_cursor += FONT_HEIGHT;
  //Position for LED
  if (!led_driver.empty()) {
    tft.setCursor(50, Y_cursor);
    tft.println(led_driver.c_str());
  }
}




void SCRNDRV::task(bool sys_asleep) {




  //If asleep make sure the screen B Light is off for highest power save.
}


void SCRNDRV::doReset() {
  digitalWrite(RESET_PIN, LOW);
  delay(100);
  digitalWrite(RESET_PIN, HIGH);
  ;
}