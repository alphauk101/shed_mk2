#include "arduino.h"
#include "disp_manager.h"
#include "DigitLedDisplay.h"



/******************************************
*         Macros and helpers
******************************************/
#define CLEAR_DISP_1 display_1.clear()
#define CLEAR_DISP_2 display_2.clear()
#define CLEAR_DISP_3 display_3.clear()
#define CLEAR_DISP_4 display_4.clear()


DigitLedDisplay display_1 = DigitLedDisplay(13, 2, 11);
DigitLedDisplay display_2 = DigitLedDisplay(13, 3, 11);
DigitLedDisplay display_3 = DigitLedDisplay(13, 4, 11);
DigitLedDisplay display_4 = DigitLedDisplay(13, 5, 11);


void disp_manager::disp_init() {
  display_1.setDigitLimit(8);
  display_2.setDigitLimit(8);
  display_3.setDigitLimit(8);
  display_4.setDigitLimit(8);

  this->setBrightAllDisp(10);

  CLEAR_DISP_1;
  CLEAR_DISP_2;
  CLEAR_DISP_3;
  CLEAR_DISP_4;

/*
  double intp;
  double intf = modf(-15.6, &intp);
  Serial.println(intp);
  Serial.println(intf*10);
*/
}

void disp_manager::setBrightAllDisp(int bright) {
  display_1.setBright(bright);
  display_2.setBright(bright);
  display_3.setBright(bright);
  display_4.setBright(bright);
}

void disp_manager::disp_environments(float i_temp, float i_hum, float e_temp) {

  dispTempData temp_data;
  this->convertTemperatureData(i_temp, &temp_data);

  CLEAR_DISP_1;
  display_1.printDigit(temp_data.frac, 0, false);
  display_1.printDigit(temp_data.dec, 1, true);
}


void disp_manager::convertTemperatureData(float raw_temp, dispTempData *outdata_ptr)
{
    double intp, intf;

    intf = modf(raw_temp, &intp);
    outdata_ptr->positive = true;
    if(intp < 0)
    {
        intf *=-1;
        intp *=-1;
        outdata_ptr->positive = false;
    }
    intf*=10;
    outdata_ptr->dec = (int)intp;
    outdata_ptr->frac = (int)intf; 
}


