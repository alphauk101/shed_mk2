#include <Wire.h>
#include "AHTxx.h"
#include "LED_controller.h"

AHTxx aht20(AHTXX_ADDRESS_X38, AHT2x_SENSOR); //sensor address, sensor type

//Pixel LED controller.
LED_Controller gLEDcontroller;


void setup() 
{
  Serial.begin(115200);
  Serial.println();

    while (aht20.begin() != true)
  {
    Serial.println(F("AHT2x not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free

    delay(5000);
  }

  Serial.println(F("AHT20 OK"));


  //Initialise the LED controller.
  gLEDcontroller.led_init();
  //gLEDcontroller.theaterChase(0xFF00FFAA, 100);


}

float ahtValue;                               //to store T/RH result

void loop() {
  /* DEMO - 1, every temperature or humidity call will read 6-bytes over I2C, total 12-bytes */
  Serial.println();
  Serial.println(F("DEMO 1: read 12-bytes"));

  ahtValue = aht20.readTemperature(); //read 6-bytes via I2C, takes 80 milliseconds

  Serial.print(F("Temperature...: "));
  
  if (ahtValue != AHTXX_ERROR) //AHTXX_ERROR = 255, library returns 255 if error occurs
  {
    Serial.print(ahtValue);
    Serial.println(F(" +-0.3C"));
  }
  else
  {
    //printStatus(); //print temperature command status

    if   (aht20.softReset() == true) Serial.println(F("reset success")); //as the last chance to make it alive
    else                             Serial.println(F("reset failed"));
  }

  delay(2000); //measurement with high frequency leads to heating of the sensor, see NOTE

  ahtValue = aht20.readHumidity(); //read another 6-bytes via I2C, takes 80 milliseconds

  Serial.print(F("Humidity......: "));
  
  if (ahtValue != AHTXX_ERROR) //AHTXX_ERROR = 255, library returns 255 if error occurs
  {
    Serial.print(ahtValue);
    Serial.println(F(" +-2%"));
  }
  else
  {
    //printStatus(); //print humidity command status
  }

  delay(2000); //measurement with high frequency leads to heating of the sensor, see NOTE

  /* DEMO - 2, temperature call will read 6-bytes via I2C, humidity will use same 6-bytes */
  Serial.println();
  Serial.println(F("DEMO 2: read 6-byte"));

  ahtValue = aht20.readTemperature(); //read 6-bytes via I2C, takes 80 milliseconds

  Serial.print(F("Temperature: "));
  
  if (ahtValue != AHTXX_ERROR) //AHTXX_ERROR = 255, library returns 255 if error occurs
  {
    Serial.print(ahtValue);
    Serial.println(F(" +-0.3C"));
  }
  else
  {
    //printStatus(); //print temperature command status
  }

  ahtValue = aht20.readHumidity(AHTXX_USE_READ_DATA); //use 6-bytes from temperature reading, takes zero milliseconds!!!

  Serial.print(F("Humidity...: "));
  
  if (ahtValue != AHTXX_ERROR) //AHTXX_ERROR = 255, library returns 255 if error occurs
  {
    Serial.print(ahtValue);
    Serial.println(F(" +-2%"));
  }
  else
  {
    //printStatus(); //print temperature command status not humidity!!! RH measurement use same 6-bytes from T measurement
  }

  delay(10000); //recomended polling frequency 8sec..30sec
}
