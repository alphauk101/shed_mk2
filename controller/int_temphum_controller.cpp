#include "int_temphum_controller.h"
#include <Wire.h>
#include "AHTxx.h"

AHTxx aht20(AHTXX_ADDRESS_X38, AHT2x_SENSOR);  //sensor address, sensor type

bool internal_tmphum::init_temphum()
{

  bool return_ok = false;
  int retries = 5;


  while ((retries != true) && (retries > 0)) {
    retries = aht20.begin();
    //Serial.println(F("AHT2x not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free

    if (retries)
    {
      return_ok = true;
      break;
    }
      

    delay(5000);
    retries--;
  }


  //Serial.println(F("AHT20 OK"));


  return return_ok;
}

float internal_tmphum::get_temp() {
  float raw_temp;

  raw_temp = aht20.readTemperature();  //read 6-bytes via I2C, takes 80 milliseconds

  if (raw_temp == AHTXX_ERROR)  //AHTXX_ERROR = 255, library returns 255 if error occurs
  {
    aht20.softReset();
    delay(2000);
    raw_temp = aht20.readTemperature();
    //read again, if error occurs then assume a fault
  }

  return raw_temp;
}

float internal_tmphum::get_humd() {
  float raw_temp;
  raw_temp = aht20.readHumidity();  //read another 6-bytes via I2C, takes 80 milliseconds
  if (raw_temp == AHTXX_ERROR)      //AHTXX_ERROR = 255, library returns 255 if error occurs
  {
    aht20.softReset();
    delay(2000);
    raw_temp = aht20.readHumidity();
    //read again, if error occurs then assume a fault
  }

  return raw_temp;
}


