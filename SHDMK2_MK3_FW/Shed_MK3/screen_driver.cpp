#include "api/HardwareSPI.h"
#include "screen_driver.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "shdmk3_config.h"
#include "screen_grfx.h"
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

//#define DEBUG_SCREEN
#define MCR_MAND_TASK_FXN \
  this->showPowerStates(); \
  this->setNetworkIcon()

#define SPI_SPEED 8000000

#define TITLE_FONT &FreeSans12pt7b
#define DEFAULT_FONT &FreeSans9pt7b
#define HUGE_FONT &FreeSans24pt7b

//Position for the text in the power states boxes
#define LIGHT_TEXT_CURSOR_X 15
#define LIGHT_TEXT_CURSOR_Y 230
#define FAN_TEXT_CURSOR_X 105
#define FAN_TEXT_CURSOR_Y LIGHT_TEXT_CURSOR_Y
#define BLOWER_TEXT_CURSOR_X 180
#define BLOWER_TEXT_CURSOR_Y LIGHT_TEXT_CURSOR_Y
#define MISC_TEXT_CURSOR_X 260
#define MISC_TEXT_CURSOR_Y LIGHT_TEXT_CURSOR_Y



#define MCR_SET_DEF_TITLE_POSITION tft.setCursor(10, 20)

#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320

#define HORZ_NETWORK_DIVIDER 275  //Move the network section horizontal divider here
#define VERTICAL_TITLE_HEIGHT 30  //Title vertical height (everything should adjust auto)
#define VERITCAL_BASE_HEIGHT (SCREEN_HEIGHT - 30)
#define NETWORK_ICOM_HORZ 285

#define PIR_ICON_HORZ 250

#define HORZ_PWRSTS_SECTION_WIDTH_DIVIDER 80  //Power status section across the bottom of the screen, should be factor of 320

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS_PIN, TFT_DC_PIN);

#define MCR_SET_CURRENT_SCREEN(X) g_screen_data.current_screen = X

typedef enum {
  CS_none,
  CS_startup,  //Start up message screen
  CS_extTemperature,
  CS_intTemperature,
  CS_intHumidity,
  CS_countdown,
  CS_DoorState,
} currentScreen;


typedef struct {
  networkState_icon current_network_icon;
  networkState_icon update_network_icon;
  currentScreen current_screen;
  POWER_STATES power_states;
  bool update_power_states;
  bool PIR_current_State;
  bool screenBL_state;
  UL_TIMER_t screen_change_timer;
} SCREEN_DATA;
static SCREEN_DATA g_screen_data;

void SCRNDRV::init() {
  //setup reset and backlight pins.
  pinMode(BACKLIGHT_PIN, OUTPUT);

  pinMode(TOUCHSELECT_PIN, OUTPUT);
  digitalWrite(TOUCHSELECT_PIN, HIGH);  //deselect touchpin select, if connected.


  digitalWrite(BACKLIGHT_PIN, BACKLIGHT_ON);
  digitalWrite(RESET_PIN, HIGH);
  g_screen_data.screen_change_timer = 0;
  //Set default data.
  g_screen_data.current_network_icon = signal_none;  //ensure this is not valid so it gets overwritten
  //Set to un connected
  this->setNetworkState(not_connected);
  g_screen_data.current_screen = CS_none;


  g_screen_data.PIR_current_State = false;
  g_screen_data.update_power_states = true;


  //Reset the screen to ensure ready state
  this->doReset();
  tft.begin(SPI_SPEED);
  //set to false to force on
  g_screen_data.screenBL_state = false;
  this->fadeBackLight(true);
  //this->setStartUpMessage();

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
  this->setDefaultScreenLayout(false, STARTUP_MESSAGE);
  this->setNetworkIcon();  //Update this if necessary
}



#define FONT_HEIGHT 22
#define TEXT_START_X 30
/*
This creates the startup screen, this is only called on startup
*/
void SCRNDRV::updateStartUpMessage(const std::string& IOexp_sts,           //IO expander status
                                   const std::string& intTemperature_sts,  //Internal temperature
                                   const std::string& intHumidity_sts,     //Internal humidity
                                   const std::string& extTemperature_sts,  //External temperature
                                   const std::string& led_driver,
                                   const std::string& wifi_status,
                                   const std::string& rtc_status) {  //LED driver


  //Position for IO status
  int Y_cursor = 65;
  if (!IOexp_sts.empty()) {
    tft.setCursor(TEXT_START_X, Y_cursor);
    tft.println(IOexp_sts.c_str());
  }
  Y_cursor += FONT_HEIGHT;
  //Position for int temp status
  if (!intTemperature_sts.empty()) {
    tft.setCursor(TEXT_START_X, Y_cursor);
    tft.println(intTemperature_sts.c_str());
  }
  Y_cursor += FONT_HEIGHT;
  //Position for int hum status
  if (!intHumidity_sts.empty()) {
    tft.setCursor(TEXT_START_X, Y_cursor);
    tft.println(intHumidity_sts.c_str());
  }
  Y_cursor += FONT_HEIGHT;
  //Position for ext temp status
  if (!extTemperature_sts.empty()) {
    tft.setCursor(TEXT_START_X, Y_cursor);
    tft.println(extTemperature_sts.c_str());
  }
  Y_cursor += FONT_HEIGHT;
  //Position for LED
  if (!led_driver.empty()) {
    tft.setCursor(TEXT_START_X, Y_cursor);
    tft.println(led_driver.c_str());
  }

  Y_cursor += FONT_HEIGHT;
  //Position for LED
  if (!wifi_status.empty()) {
    tft.setCursor(TEXT_START_X, Y_cursor);
    tft.println(wifi_status.c_str());
  }
  Y_cursor += FONT_HEIGHT;
  //Position for LED
  if (!rtc_status.empty()) {
    tft.setCursor(TEXT_START_X, Y_cursor);
    tft.println(rtc_status.c_str());
  }
}

void SCRNDRV::fadeBackLight(bool dir) {

  // This 'for' loop starts with brightness at 0,
  // and keeps looping as long as brightness is less than or equal to 255.
  // In each loop, it increases brightness by 1.

  if (g_screen_data.screenBL_state != dir) {
    for (int brightness = 0; brightness <= 255; brightness++) {

      // Set the LED's brightness to the current value
      if (dir) {
        analogWrite(BACKLIGHT_PIN, (255 - brightness));
      } else {
        analogWrite(BACKLIGHT_PIN, brightness);
      }
      // This delay is crucial. It pauses the code for 10 milliseconds
      // so your eye can perceive the change in brightness.
      // Try changing this value to speed up or slow down the fade.
      delay(10);
    }
    g_screen_data.screenBL_state = dir;
  }
}

void SCRNDRV::setNetworkState(networkState_icon state) {
  g_screen_data.update_network_icon = state;
}



/*This sets the default screen layout but does not set the 
main content as it may change*/
static std::string last_message;
void SCRNDRV::setDefaultScreenLayout(bool update_if_changed = false, const std::string& title_str = "") {


  if (update_if_changed) {
    if (last_message == title_str) {
      return;  //dont update
    }
  }

  last_message = title_str;

  tft.fillScreen(ILI9341_WHITE);
  tft.setRotation(DEFAULT_ORIENTATION);
  //Draw the top line divider
  tft.drawFastHLine(0, VERTICAL_TITLE_HEIGHT, 320, ILI9341_BLACK);
  tft.drawFastHLine(0, VERTICAL_TITLE_HEIGHT + 1, 320, ILI9341_BLACK);
  //Draw the container for the network state

  tft.drawFastVLine(HORZ_NETWORK_DIVIDER, 0, VERTICAL_TITLE_HEIGHT, ILI9341_BLACK);
  tft.drawFastVLine(HORZ_NETWORK_DIVIDER + 1, 0, VERTICAL_TITLE_HEIGHT, ILI9341_BLACK);

  tft.drawFastHLine(0, VERITCAL_BASE_HEIGHT, 320, ILI9341_BLACK);
  tft.drawFastHLine(0, VERITCAL_BASE_HEIGHT + 1, 320, ILI9341_BLACK);

  tft.drawFastVLine((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER), (VERITCAL_BASE_HEIGHT), (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT), ILI9341_BLACK);
  tft.drawFastVLine((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER + 1), (VERITCAL_BASE_HEIGHT), (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT), ILI9341_BLACK);

  tft.drawFastVLine((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 2), (VERITCAL_BASE_HEIGHT), (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT), ILI9341_BLACK);
  tft.drawFastVLine(((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 2) + 1), (VERITCAL_BASE_HEIGHT), (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT), ILI9341_BLACK);

  tft.drawFastVLine((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 3), (VERITCAL_BASE_HEIGHT), (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT), ILI9341_BLACK);
  tft.drawFastVLine(((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 3) + 1), (VERITCAL_BASE_HEIGHT), (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT), ILI9341_BLACK);

  tft.drawFastVLine((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 4), (VERITCAL_BASE_HEIGHT), (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT), ILI9341_BLACK);
  tft.drawFastVLine(((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 4) + 1), (VERITCAL_BASE_HEIGHT), (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT), ILI9341_BLACK);



  // tft.drawFastVLine(41, 301, 20, ILI9341_BLACK);
  //This should leave a small area for the network symbol
  MCR_SET_DEF_TITLE_POSITION;

  tft.setTextColor(ILI9341_BLACK);
  tft.setFont(TITLE_FONT);
  tft.setTextSize(0);
  tft.print(title_str.c_str());
}

void SCRNDRV::clearDynamicSection(uint16_t BG_color) {
  tft.fillRect(0, VERTICAL_TITLE_HEIGHT + 2, SCREEN_WIDTH, SCREEN_HEIGHT - (((VERTICAL_TITLE_HEIGHT + 2) + (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT))), BG_color);

  //tft.fillRect(50, 50, 50, 50, ILI9341_BLACK);
}

void SCRNDRV::setShowNetConnect() {
  //toggle through the different icons, the speed is dictated by the calling fxn

  switch (g_screen_data.current_network_icon) {
    case signal_good:
      setNetworkState(signal_awful);
      //tft.drawRGBBitmap(NETWORK_ICOM_HORZ, 0, (uint16_t*)network_full_grfx, 40, 40);
      break;
    case signal_ok:
      setNetworkState(signal_good);
      // tft.drawRGBBitmap(NETWORK_ICOM_HORZ, 0, (uint16_t*)network_75_grfx, 40, 40);
      break;
    case signal_bad:
      setNetworkState(signal_ok);
      //tft.drawRGBBitmap(NETWORK_ICOM_HORZ, 0, (uint16_t*)network_50_grfx, 40, 40);
      break;
    case signal_awful:
      setNetworkState(signal_bad);
      break;
    default:
      setNetworkState(signal_awful);
      // tft.drawRGBBitmap(NETWORK_ICOM_HORZ, 0, (uint16_t*)network_off_grfx, 40, 40);
      //show not connected here
      break;
  }
  this->setNetworkIcon();
}


void SCRNDRV::setPowerStates(bool light, bool fan, bool blower, bool misc) {

  if ((g_screen_data.power_states.blower != blower) || (g_screen_data.power_states.fan != fan) || (g_screen_data.power_states.lights != light) || (g_screen_data.power_states.misc != misc)) {
    g_screen_data.update_power_states = true;
    g_screen_data.power_states.blower = blower;
    g_screen_data.power_states.fan = fan;
    g_screen_data.power_states.lights = light;
    g_screen_data.power_states.misc = misc;
  }
}

void SCRNDRV::changeViewingScreen() {
  this->changeSreen();
}


#define PS_BGCOLOUR_ON ILI9341_GREEN
#define PS_FONTCOL_ON ILI9341_BLACK
#define PS_BGCOLOUR_OFF ILI9341_RED
#define PS_FONTCOL_OFF ILI9341_WHITE
void SCRNDRV::showPowerStates() {

  if (g_screen_data.update_power_states) {

    tft.setFont(DEFAULT_FONT);
    tft.setTextSize(0);

    if (g_screen_data.power_states.lights == RELAY_LIGHT_ON) {
      //draw green
      tft.fillRect((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 0), VERITCAL_BASE_HEIGHT + 2, HORZ_PWRSTS_SECTION_WIDTH_DIVIDER, (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT) - 2, PS_BGCOLOUR_ON);
      tft.setTextColor(PS_FONTCOL_ON);

    } else {
      tft.fillRect((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 0), VERITCAL_BASE_HEIGHT + 2, HORZ_PWRSTS_SECTION_WIDTH_DIVIDER, (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT) - 2, PS_BGCOLOUR_OFF);
      tft.setTextColor(PS_FONTCOL_OFF);
    }
    tft.setCursor(LIGHT_TEXT_CURSOR_X, LIGHT_TEXT_CURSOR_Y);
    tft.print("Light");


    if (g_screen_data.power_states.fan == RELAY_FAN_ON) {
      //draw green
      tft.fillRect((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 1) + 2, VERITCAL_BASE_HEIGHT + 2, HORZ_PWRSTS_SECTION_WIDTH_DIVIDER - 2, (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT) - 2, PS_BGCOLOUR_ON);
      tft.setTextColor(PS_FONTCOL_ON);
    } else {
      tft.fillRect((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 1) + 2, VERITCAL_BASE_HEIGHT + 2, HORZ_PWRSTS_SECTION_WIDTH_DIVIDER - 2, (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT) - 2, PS_BGCOLOUR_OFF);
      tft.setTextColor(PS_FONTCOL_OFF);
    }
    tft.setCursor(FAN_TEXT_CURSOR_X, FAN_TEXT_CURSOR_Y);
    tft.print("Fan");


    if (g_screen_data.power_states.blower == RELAY_BLOWER_ON) {
      //draw green
      tft.fillRect((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 2) + 2, VERITCAL_BASE_HEIGHT + 2, HORZ_PWRSTS_SECTION_WIDTH_DIVIDER - 2, (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT) - 2, PS_BGCOLOUR_ON);
      tft.setTextColor(PS_FONTCOL_ON);
    } else {
      tft.fillRect((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 2) + 2, VERITCAL_BASE_HEIGHT + 2, HORZ_PWRSTS_SECTION_WIDTH_DIVIDER - 2, (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT) - 2, PS_BGCOLOUR_OFF);
      tft.setTextColor(PS_FONTCOL_OFF);
    }
    tft.setCursor(BLOWER_TEXT_CURSOR_X, BLOWER_TEXT_CURSOR_Y);
    tft.print("Dryer");


    if (g_screen_data.power_states.misc == RELAY_MISC_ON) {
      tft.setTextColor(PS_FONTCOL_ON);
      tft.fillRect((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 3) + 2, VERITCAL_BASE_HEIGHT + 2, HORZ_PWRSTS_SECTION_WIDTH_DIVIDER - 2, (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT) - 2, PS_BGCOLOUR_ON);
    } else {
      tft.setTextColor(PS_FONTCOL_OFF);
      tft.fillRect((HORZ_PWRSTS_SECTION_WIDTH_DIVIDER * 3) + 2, VERITCAL_BASE_HEIGHT + 2, HORZ_PWRSTS_SECTION_WIDTH_DIVIDER - 2, (SCREEN_HEIGHT - VERITCAL_BASE_HEIGHT) - 2, PS_BGCOLOUR_OFF);
    }
    tft.setCursor(MISC_TEXT_CURSOR_X, MISC_TEXT_CURSOR_Y);
    tft.print("Misc");

    g_screen_data.update_power_states = false;
  }
}


//Sets the appropriate network icon on the screen
#define NETWORK_ICON_PXL_SIZE 28
void SCRNDRV::setNetworkIcon() {

  if (g_screen_data.current_network_icon != g_screen_data.update_network_icon) {
    g_screen_data.current_network_icon = g_screen_data.update_network_icon;

    switch (g_screen_data.current_network_icon) {
      case signal_good:
        tft.drawRGBBitmap(NETWORK_ICOM_HORZ, 0, (uint16_t*)network_full_grfx, NETWORK_ICON_PXL_SIZE, NETWORK_ICON_PXL_SIZE);
        break;
      case signal_ok:
        tft.drawRGBBitmap(NETWORK_ICOM_HORZ, 0, (uint16_t*)network_75_grfx, NETWORK_ICON_PXL_SIZE, NETWORK_ICON_PXL_SIZE);
        break;
      case signal_bad:
        tft.drawRGBBitmap(NETWORK_ICOM_HORZ, 0, (uint16_t*)network_50_grfx, NETWORK_ICON_PXL_SIZE, NETWORK_ICON_PXL_SIZE);
        break;
      case signal_awful:
        tft.drawRGBBitmap(NETWORK_ICOM_HORZ, 0, (uint16_t*)network_25_grfx, NETWORK_ICON_PXL_SIZE, NETWORK_ICON_PXL_SIZE);
        break;
      default:
        tft.drawRGBBitmap(NETWORK_ICOM_HORZ, 0, (uint16_t*)network_off_grfx, NETWORK_ICON_PXL_SIZE, NETWORK_ICON_PXL_SIZE);
        //show not connected here
        break;
    }
  }
}

float tmp_inttemp = 0;
float tmp_exttemp = 0;
float tmp_humidity = 0;
UL_TIMER_t sleep_countdown = 0;
bool info_shown = false;

void SCRNDRV::resetScreenVars() {
  tmp_inttemp = 0;
  tmp_exttemp = 0;
  tmp_humidity = 0;
  sleep_countdown = 0;
  info_shown = false;
  g_screen_data.update_power_states = true;
  g_screen_data.current_network_icon = signal_none;
}

void SCRNDRV::task(SHED_APP* shd_data, bool sys_sleep, bool net_isConnected, bool PIR_state) {

  //the fxn will deal with the repeat calls.
  bool BLstate = (sys_sleep) ? false : true;
  this->fadeBackLight(BLstate);

  this->setPIRIcon(PIR_state);


  this->setPowerStates(shd_data->power_states.lights,
                       shd_data->power_states.fan,
                       shd_data->power_states.blower,
                       shd_data->power_states.misc);


  //check if the timer has elapsed and change the screen accordingly.
  unsigned long current_time = millis();
  if (!shd_data->sleep_countdown_act) {

    if ((current_time - g_screen_data.screen_change_timer) > SCREEN_CHANGE_TIMEOUT) {
      //change screen time
      this->changeSreen();
    }

    switch (g_screen_data.current_screen) {
      case CS_intTemperature:
        this->SCREENLAYOUT_internalTemp(shd_data);
        break;
      case CS_extTemperature:
        this->SCREENLAYOUT_ExternalTemp(shd_data);
        break;
      case CS_intHumidity:
        this->SCREENLAYOUT_internalHumd(shd_data);
        break;
      case CS_DoorState:
        this->SCREENLAYOUT_Information(shd_data, net_isConnected);
        break;
      default:
        break;
    }
  } else {

    if (g_screen_data.current_screen != CS_countdown) {
      this->setDefaultScreenLayout(false, "Sleep timer");
      //first time set initial properties
      g_screen_data.update_power_states = true;
    }

    //special case if system is in countdown mode
    this->SCREENLAYOUT_countdown(shd_data);
    MCR_SET_CURRENT_SCREEN(CS_countdown);
  }
}

#define PIR_GRFX_WIDTH 40
#define PIR_GRFX_HEIGHT 30
void SCRNDRV::setPIRIcon(bool state) {

  if (g_screen_data.PIR_current_State != state) {
    g_screen_data.PIR_current_State = state;

    if (g_screen_data.PIR_current_State) {
      tft.drawRGBBitmap(PIR_ICON_HORZ, 50, (uint16_t*)pir_detected_grfx, PIR_GRFX_WIDTH, PIR_GRFX_HEIGHT);
    }
  }
}

/*
Changes the screen type to next screen - sets up the staic compoents for the 
new screen layout*/
void SCRNDRV::changeSreen() {
  switch (g_screen_data.current_screen) {
    case CS_intTemperature:
      MCR_SET_CURRENT_SCREEN(CS_extTemperature);
      this->setDefaultScreenLayout(false, "Outdoor Temperature");
      break;
    case CS_extTemperature:
      MCR_SET_CURRENT_SCREEN(CS_intHumidity);
      this->setDefaultScreenLayout(false, "Indoor Humidity");
      break;
    case CS_intHumidity:
      MCR_SET_CURRENT_SCREEN(CS_DoorState);
      this->setDefaultScreenLayout(false, "Information");
      break;
    case CS_DoorState:
      MCR_SET_CURRENT_SCREEN(CS_intTemperature);
      this->setDefaultScreenLayout(false, "Indoor Temperature");
      break;
    default:
      //everything ..defaults to indoor temp
      MCR_SET_CURRENT_SCREEN(CS_intTemperature);
      this->setDefaultScreenLayout(false, "Indoor Temperature");
      break;
  }
  //reset the timer from here to allow this fxn from other places
  g_screen_data.screen_change_timer = millis();

  //reset screen dependant flags/vars here
  this->resetScreenVars();
}

/*********Below are the different screen layouts**************/



void SCRNDRV::SCREENLAYOUT_countdown(SHED_APP* shd_data) {

  if (shd_data->app_timers.sys_sleep_timer != sleep_countdown) {

    sleep_countdown = shd_data->app_timers.sys_sleep_timer;
    //Clears the dynamic part of the active screen
    this->clearDynamicSection(ILI9341_WHITE);

    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setFont(HUGE_FONT);
    tft.setCursor(100, 140);
    tft.print(sleep_countdown);
    //tft.print();
  }
  //do additional screen tasks here to avoid overwriting
  MCR_MAND_TASK_FXN;
}


void SCRNDRV::SCREENLAYOUT_internalTemp(SHED_APP* shd_data) {

  if (tmp_inttemp != shd_data->environmentals.internal_temp) {
    tmp_inttemp = shd_data->environmentals.internal_temp;
    //Clears the dynamic part of the active screen
    this->clearDynamicSection(ILI9341_WHITE);

    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setFont(HUGE_FONT);
    tft.setCursor(10, 140);
    tft.print(shd_data->environmentals.internal_temp);
    tft.print("c");

    tft.setTextSize(0);
    tft.setFont(DEFAULT_FONT);
    //Set the highest temperature
    tft.setCursor(10, 200);
    tft.print("Max: ");
    tft.print(shd_data->environmentals.internal_temp_max);
    tft.print("c");

    tft.setCursor(200, 200);
    tft.print("Min: ");
    tft.print(shd_data->environmentals.internal_temp_min);
    tft.print("c");
  }

  //do additional screen tasks here to avoid overwriting
  MCR_MAND_TASK_FXN;
}

void SCRNDRV::SCREENLAYOUT_internalHumd(SHED_APP* shd_data) {

  if (tmp_humidity != shd_data->environmentals.internal_humidity) {
    tmp_humidity = shd_data->environmentals.internal_humidity;

    //Clears the dynamic part of the active screen
    if (shd_data->environmentals.internal_temp < shd_data->environmentals.internal_dewpoint) {
      //if(true){
      this->clearDynamicSection(ILI9341_RED);
      tft.setTextColor(ILI9341_WHITE);
    } else {
      this->clearDynamicSection(ILI9341_WHITE);
      tft.setTextColor(ILI9341_BLACK);
    }

    //tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(1);
    tft.setFont(HUGE_FONT);
    tft.setCursor(90, 100);
    tft.print(shd_data->environmentals.internal_humidity);
    tft.print("%");
    /////////////////////////////////////////////////////////////////////////////////////////
    tft.setTextSize(1);
    tft.setFont(HUGE_FONT);
    tft.setCursor(20, 150);
    tft.print("Dew: ");
    tft.print(shd_data->environmentals.internal_dewpoint);
    tft.print("c");
    //////////////////////////////////////////////////////////////////////////////////////////
    tft.setTextSize(0);
    tft.setFont(DEFAULT_FONT);
    //Set the highest temperature
    tft.setCursor(10, 200);
    tft.print("Max: ");
    tft.print(shd_data->environmentals.internal_humidity_max);
    tft.print("%");

    tft.setCursor(200, 200);
    tft.print("Min: ");
    tft.print(shd_data->environmentals.internal_humidity_min);
    tft.print("%");
  }

  //do additional screen tasks here to avoid overwriting
  MCR_MAND_TASK_FXN;
}


void SCRNDRV::SCREENLAYOUT_ExternalTemp(SHED_APP* shd_data) {

  if (tmp_exttemp != shd_data->environmentals.external_temp) {
    tmp_exttemp = shd_data->environmentals.external_temp;
    //Clears the dynamic part of the active screen
    this->clearDynamicSection(ILI9341_WHITE);

    tft.setTextColor(ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setFont(HUGE_FONT);
    tft.setCursor(10, 140);
    tft.print(shd_data->environmentals.external_temp);
    tft.print("c");

    tft.setTextSize(0);
    tft.setFont(DEFAULT_FONT);
    //Set the highest temperature
    tft.setCursor(10, 200);
    tft.print("Max: ");
    tft.print(shd_data->environmentals.external_temp_max);
    tft.print("c");

    tft.setCursor(200, 200);
    tft.print("Min: ");
    tft.print(shd_data->environmentals.external_temp_min);
    tft.print("c");
  }
  //do additional screen tasks here to avoid overwriting
  MCR_MAND_TASK_FXN;
}


void SCRNDRV::SCREENLAYOUT_Information(SHED_APP* shd_data, bool net_isConnected) {
  //Door status
  //Last door open time
  //Door opened counter
  //Timestamp

  if (!info_shown) {
    info_shown = true;
    this->clearDynamicSection(ILI9341_WHITE);
    tft.setFont(DEFAULT_FONT);
    tft.setTextColor(ILI9341_BLACK);
    int Y_cursor = 46;
    //Reusing the start up screen message.

    tft.setCursor(TEXT_START_X, Y_cursor);
    tft.print("Door status: ");
    tft.print((shd_data->door_status.current_state) ? "OPEN" : "CLOSED");
    Y_cursor += FONT_HEIGHT;
    tft.setCursor(TEXT_START_X, Y_cursor);

    tft.print("Door count: ");
    tft.print(shd_data->door_status.open_counter);
    Y_cursor += FONT_HEIGHT;
    tft.setCursor(TEXT_START_X, Y_cursor);

    tft.print("Last open: ");
    tft.print(shd_data->door_status.last_opened.hour(), DEC);
    tft.print(':');
    tft.print(shd_data->door_status.last_opened.minute(), DEC);

    tft.print(' ');
    tft.print(shd_data->door_status.last_opened.day(), DEC);
    tft.print('/');
    tft.print(shd_data->door_status.last_opened.month(), DEC);

    Y_cursor += FONT_HEIGHT;
    tft.setCursor(TEXT_START_X, Y_cursor);

    /**********************************/

    if (net_isConnected) {
      tft.print("Network: connected ");
      Y_cursor += FONT_HEIGHT;
      tft.setCursor(TEXT_START_X, Y_cursor);
      tft.print("IP: ");
      tft.print(shd_data->network_info.ip);
      Y_cursor += FONT_HEIGHT;
      tft.setCursor(TEXT_START_X, Y_cursor);
      tft.print("RSSI: ");
      tft.print(shd_data->network_info.latest_RSSI);
      tft.print("dBm");
    } else {
      tft.print("Network: disconnected ");
    }
    Y_cursor += FONT_HEIGHT;
    tft.setCursor(TEXT_START_X, Y_cursor);
    /*Timestamp*/
    tft.print("Time: ");
    tft.print(shd_data->last_timestammp.hour(), DEC);
    tft.print(':');
    tft.print(shd_data->last_timestammp.minute(), DEC);
    tft.print(' ');
    tft.print(shd_data->last_timestammp.day(), DEC);
    tft.print('/');
    tft.print(shd_data->last_timestammp.month(), DEC);
    tft.print('/');
    tft.print(shd_data->last_timestammp.year(), DEC);
    Y_cursor += FONT_HEIGHT;
    tft.setCursor(TEXT_START_X, Y_cursor);

    String uptime = uptimeString(millis());
    tft.print("Uptime: ");
    tft.print(uptime);
  }
  //do additional screen tasks here to avoid overwriting
  MCR_MAND_TASK_FXN;
}

String SCRNDRV::uptimeString(unsigned long currentMillis) {
  unsigned long seconds = currentMillis / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;

  // Calculate the "remainders" for the display
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  // Create a formatted String (e.g., "1d 02:15:45")
  String timeString = String(days) + "d " + String(hours) + ":" + String(minutes) + ":" + String(seconds);

  return timeString;
}
