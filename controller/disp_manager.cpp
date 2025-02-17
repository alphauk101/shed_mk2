#include "arduino.h"
#include "disp_manager.h"
#include "DigitLedDisplay.h"
#include "IO_pins.h"

typedef struct {
  float ext_hi, ext_lo;
  float int_hi, int_lo;
  bool sys_sleep;
  bool scrns_initd;
} DISPLAY_DATA;
DISPLAY_DATA g_display_data;

/******************************************
*         Macros and helpers
******************************************/
#define INTTEMPHUM_DISPLAY display_1
#define EXTTEMP_DISPLAY display_2
#define HILO_INT_DISPLAY display_3
#define HILO_EXT_DISPLAY display_4

#define CLEAR_INTTEMPHUM_DISP INTTEMPHUM_DISPLAY.clear()
#define CLEAR_EXTTEMP_DISP EXTTEMP_DISPLAY.clear()
#define CLEAR_DISP_HILO_INT HILO_INT_DISPLAY.clear()
#define CLEAR_DISP_HILO_EXT HILO_EXT_DISPLAY.clear()

#define INIT_ALL_DISPLAYS \
  INTTEMPHUM_DISPLAY.setDigitLimit(8); \
  HILO_EXT_DISPLAY.setDigitLimit(8); \
  HILO_INT_DISPLAY.setDigitLimit(8); \
  EXTTEMP_DISPLAY.setDigitLimit(8); \
  g_display_data.scrns_initd = true;

#define CLEAR_ALL_DISPLAYS \
  CLEAR_INTTEMPHUM_DISP; \
  CLEAR_EXTTEMP_DISP; \
  CLEAR_DISP_HILO_INT; \
  CLEAR_DISP_HILO_EXT



DigitLedDisplay display_1 = DigitLedDisplay(13, DISP_1_PIN, 11);
DigitLedDisplay display_2 = DigitLedDisplay(13, DISP_2_PIN, 11);
DigitLedDisplay display_3 = DigitLedDisplay(13, DISP_3_PIN, 11);
DigitLedDisplay display_4 = DigitLedDisplay(13, DISP_4_PIN, 11);



void disp_manager::disp_init() {

  INIT_ALL_DISPLAYS;

  this->setBrightAllDisp(10);

  CLEAR_ALL_DISPLAYS;

  //Set all display data to the extremes
  g_display_data.ext_lo = __FLT_MAX__;
  g_display_data.int_lo = __FLT_MAX__;
  g_display_data.ext_hi = __FLT_MIN__;
  g_display_data.int_hi = __FLT_MIN__;

  g_display_data.sys_sleep = false;
}

void disp_manager::setBrightAllDisp(int bright) {
  INTTEMPHUM_DISPLAY.setBright(bright);
  EXTTEMP_DISPLAY.setBright(bright);
  HILO_INT_DISPLAY.setBright(bright);
  HILO_EXT_DISPLAY.setBright(bright);
}

void disp_manager::system_sleeping(bool sleep) {
  //Informs this class if system is sleeping or not.
  g_display_data.sys_sleep = sleep;
}

void disp_manager::disp_environments(float i_temp, float i_hum, float e_temp) {

  dispTempData temp_data;

  //Clear the displays first

  if (!g_display_data.sys_sleep) {

    //Reinit any displays if they have lost power
    if(g_display_data.scrns_initd == false)
    {
      INIT_ALL_DISPLAYS;
    }

    //If system not sleeping then display content
    this->convertTemperatureData(i_temp, &temp_data);
    //Internal temp and hum
    CLEAR_INTTEMPHUM_DISP;
    INTTEMPHUM_DISPLAY.printDigit(temp_data.frac, 0, false);
    INTTEMPHUM_DISPLAY.printDigit(temp_data.dec, 1, true);
    if (!temp_data.positive) {
      INTTEMPHUM_DISPLAY.printMinus((temp_data.dec < 10) ? 3 : 4);
    }
    //Make sure the hum is as far left as possible.
    if (i_hum < 10) {
      INTTEMPHUM_DISPLAY.printDigit(i_hum, 7, false);
    } else {
      INTTEMPHUM_DISPLAY.printDigit(i_hum, 6, false);
    }
    this->convertTemperatureData(e_temp, &temp_data);
    //External temp
    CLEAR_EXTTEMP_DISP;
    EXTTEMP_DISPLAY.printDigit(temp_data.frac, 0, false);
    EXTTEMP_DISPLAY.printDigit(temp_data.dec, 1, true);
    if (!temp_data.positive) {
      EXTTEMP_DISPLAY.printMinus((temp_data.dec < 10) ? 3 : 4);
    }
  }
  //Do historic
  this->disp_historic_environments(i_temp, i_hum, e_temp);

  //If system is a sleep make sure the displays are cleared
  if (g_display_data.sys_sleep) {
    CLEAR_ALL_DISPLAYS;
    g_display_data.scrns_initd = false;
  }
}

void disp_manager::disp_historic_environments(float i_temp, float i_hum, float e_temp) {
  dispTempData temp_data;

  /*If the system is a sleep we still want to store the historic temp data 
  but dont display it*/

  if (i_temp > g_display_data.int_hi) g_display_data.int_hi = i_temp;
  if (i_temp < g_display_data.int_lo) g_display_data.int_lo = i_temp;

  if (e_temp > g_display_data.ext_hi) g_display_data.ext_hi = e_temp;
  if (e_temp < g_display_data.ext_lo) g_display_data.ext_lo = e_temp;


  //Internal temp and hum
  if (!g_display_data.sys_sleep) {
    this->convertTemperatureData(g_display_data.int_hi, &temp_data);
    CLEAR_DISP_HILO_INT;
    HILO_INT_DISPLAY.printDigit(temp_data.frac, 5, false);
    HILO_INT_DISPLAY.printDigit(temp_data.dec, 6, true);

    this->convertTemperatureData(g_display_data.int_lo, &temp_data);
    HILO_INT_DISPLAY.printDigit(temp_data.frac, 0, false);
    HILO_INT_DISPLAY.printDigit(temp_data.dec, 1, true);
    if (!temp_data.positive) {
      HILO_INT_DISPLAY.printMinus((temp_data.dec < 10) ? 3 : 4);
    }

    //External temp and hum
    CLEAR_DISP_HILO_EXT;
    this->convertTemperatureData(g_display_data.ext_hi, &temp_data);
    HILO_EXT_DISPLAY.printDigit(temp_data.frac, 5, false);
    HILO_EXT_DISPLAY.printDigit(temp_data.dec, 6, true);

    this->convertTemperatureData(g_display_data.ext_lo, &temp_data);
    HILO_EXT_DISPLAY.printDigit(temp_data.frac, 0, false);
    HILO_EXT_DISPLAY.printDigit(temp_data.dec, 1, true);
    if (!temp_data.positive) {
      HILO_EXT_DISPLAY.printMinus((temp_data.dec < 10) ? 3 : 4);
    }
  }
}

void disp_manager::convertTemperatureData(float raw_temp, dispTempData *outdata_ptr) {
  double intp, intf;

  intf = modf(raw_temp, &intp);
  outdata_ptr->positive = true;
  if (intp < 0) {
    intf *= -1;
    intp *= -1;
    outdata_ptr->positive = false;
  }
  intf *= 10;
  outdata_ptr->dec = (int)intp;
  outdata_ptr->frac = (int)intf;
}
