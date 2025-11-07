#include "screen_driver.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "shdmk3_config.h"
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

//#define DEBUG_SCREEN

#define TITLE_FONT &FreeSans12pt7b
#define DEFAULT_FONT &FreeSans9pt7b

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN);

#define MCR_SET_CURRENT_SCREEN(X)     g_screen_data.current_screen=X

typedef enum{
  CS_none,
  CS_startup,  //Start up message screen
  CS_extTemperature,
  CS_intTemperature,
  CS_intHumidity,
  CS_DoorState,
}currentScreen;


typedef struct {
  networkState_icon   network_icon;
  currentScreen       current_screen;

} SCREEN_DATA;
static SCREEN_DATA g_screen_data;

void SCRNDRV::init() {
  //setup reset and backlight pins.
  pinMode(BACKLIGHT_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  digitalWrite(BACKLIGHT_PIN, BACKLIGHT_ON);
  digitalWrite(RESET_PIN, HIGH);

  //Set default data.
  g_screen_data.network_icon = not_connected;
  g_screen_data.current_screen = CS_none;

  //Reset the screen to ensure ready state
  this->doReset();

  tft.begin();

  this->setStartUpMessage();

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

void SCRNDRV::doReset() {
  digitalWrite(RESET_PIN, LOW);
  delay(100);
  digitalWrite(RESET_PIN, HIGH);
}


void SCRNDRV::setStartUpMessage() {

  //No need to check previous screen, just populate.
  g_screen_data.current_screen = CS_startup;

  tft.setRotation(DEFAULT_ORIENTATION);
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
/*
This creates the startup screen, this is only called on startup
*/
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

void SCRNDRV::setNetworkState(networkState_icon state) {
  g_screen_data.network_icon = state;
}

/*This sets the default screen layout but does not set the 
main content as it may change*/
void SCRNDRV::setDefaultScreenLayout() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setRotation(DEFAULT_ORIENTATION);
  //Draw the top line divider
  tft.drawFastHLine(0, 40, 320, ILI9341_BLACK);
  tft.drawFastHLine(0, 41, 320, ILI9341_BLACK);
  //Draw the container for the network state
  tft.drawFastHLine(41, 300, 20, ILI9341_BLACK);
  tft.drawFastHLine(41, 301, 20, ILI9341_BLACK);
  //This should leave a small area for the network symbol
}

//Sets the appropriate network icon on the screen
void SCRNDRV::setNetworkIcon() {
  switch (g_screen_data.network_icon) {
    case signal_good:

      break;
    case sginal_ok:

      break;
    case signal_poor:

      break;
    default:
      //show not connected here
      break;
  }
}


void SCRNDRV::task(bool sys_aleep, SHED_APP* shd_data) {




  //If asleep make sure the screen B Light is off for highest power save.
}


/*********Below are the different screen layouts**************/
void SCRNDRV::SCREENLAYOUT_internalTemp(SHED_APP* shd_data) {

  //Check whether this is the first time this screen is being drawn, if so,
  //then only update the values.    FreeSans24pt7b
  if (g_screen_data.current_screen != CS_intTemperature) {
    //set the screen
    this->setDefaultScreenLayout();

    tft.setFont(DEFAULT_FONT);

    //Set the highest temperature
    tft.setCursor(10, 180);
    tft.print("Max: ");
    



    MCR_SET_CURRENT_SCREEN(CS_intTemperature);
  } else {
    //This is an update only screen.
  }

  this->setNetworkIcon();  //Update this if necessary
}

void SCRNDRV::SCREENLAYOUT_internalHumd(SHED_APP* shd_data) {



  this->setNetworkIcon();  //Update this if necessary
}


void SCRNDRV::SCREENLAYOUT_ExternalTemp(SHED_APP* shd_data) {



  this->setNetworkIcon();  //Update this if necessary
}

void SCRNDRV::SCREENLAYOUT_DoorStatus(SHED_APP* shd_data) {



  this->setNetworkIcon();  //Update this if necessary
}
