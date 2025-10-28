#include <Wire.h>
#include "IOEXP_drv.h"
#include "int_temphum.h"
#include "ext_temphum.h"
#include "shdmk3_config.h"


#define MAIN_LOOP_DELAY 250

IOEXP_DRV g_IOEXP_driver;
INTTMPHUM g_intTempHum;
EXTTEMP g_extTemp;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  while (!Serial) {}

  //MUST BE INIT'D BEFORE USING AN PERIPHERALS
  Wire.begin();

  Serial.println("Shed MK3 - V0.1 ... Starting");

  //setup ios
  g_IOEXP_driver.poweron_setup();
  //set default states
  g_IOEXP_driver.set_relay_pins(RELAY_FAN_OFF, RELAY_DRYER_OFF, REALY_LIGHT_OFF, RELAY_MISC_OFF);


  //Setup i2c temp hum sensor
  g_intTempHum.init();
  g_extTemp.init();

  Serial.println("Shed MK3 - V0.1 ... complete");
}


void loop() {

  //g_IOEXP_driver.set_relay_pins(true, true, true, true);
  //delay(2000);
  //g_IOEXP_driver.set_relay_pins(false, false, false, false);
  //Serial.println("wjhopg");

  float t = g_intTempHum.get_temperature();
  float h = g_intTempHum.get_humidity();
  if (!isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = ");
    Serial.print(t);
    Serial.print("\t\t");
  } else {
    Serial.println("Failed to read temperature");
  }

  if (!isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = ");
    Serial.println(h);
  } else {
    Serial.println("Failed to read humidity");
  }


  float et = g_extTemp.getTemp();
  if (!isnan(et)) {  // check if 'is not a number'
    Serial.print("External temp *C = ");
    Serial.print(et);
    Serial.print("\t\t");
  } else {
    Serial.println("Failed to read external temp sensor");
  }




  delay(MAIN_LOOP_DELAY);
}
