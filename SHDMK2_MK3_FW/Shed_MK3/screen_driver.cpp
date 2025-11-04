#include "screen_driver.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "shdmk3_config.h"
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

//#define DEBUG_SCREEN

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
  tft.setCursor(50, 30);
  tft.setTextColor(ILI9341_BLACK);
  tft.setFont(&FreeSans9pt7b);
  //tft.setTextSize(3);
  tft.println(STARTUP_MESSAGE);
  tft.drawFastHLine(0, 320, 3, ILI9341_BLACK);
}


void SCRNDRV::updateStartUpMessage(const std::string& IOexp_sts,           //IO expander status
                                   const std::string& intTemperature_sts,  //Internal temperature
                                   const std::string& intHumidity_sts,     //Internal humidity
                                   const std::string& extTemperature_sts,  //External temperature
                                   const std::string& led_driver) {        //LED driver

  //Position for IO status
  if (!IOexp_sts.empty()) {
    tft.setCursor(50, 70);
    tft.println(IOexp_sts.c_str());
  }

  //Position for int temp status
  if (!intTemperature_sts.empty()) {
    tft.setCursor(50, 100);
    tft.println(intTemperature_sts.c_str());
  }
  //Position for int hum status
  if (!intHumidity_sts.empty()) {
    tft.setCursor(50, 120);
    tft.println(intHumidity_sts.c_str());
  }

  //Position for ext temp status
  if (!extTemperature_sts.empty()) {
    tft.setCursor(50, 170);
    tft.println(extTemperature_sts.c_str());
  }

  //Position for LED
  if (!led_driver.empty()) {
    tft.setCursor(50, 200);
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