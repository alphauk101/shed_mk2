
#include "LED_controller.h"             //LEDs (Neo pixel)
#include "int_temphum_controller.h"     //Internal AHT25
#include "ext_temp_controller.h"        //External DS18b20 temperature probe
#include "relay_manager.h"
#include "disp_manager.h"

#define DEV_MODE    //If en, turns off pointless animas etc.

typedef struct{
  //Relay states
  bool    heater_relay;
  bool    light_relay;
  bool    misc_relay;
  bool    fan_relay;

  //Temperature data.
  float   int_temperature;
  float   int_humid;
  float   ext_temperature;

}APP_LOGIC_DATA;

APP_LOGIC_DATA gApp_data =
{
  false, //heater
  false, //light
  false, //misc
  false, //fan
};

//Pixel LED controller.
LED_Controller    gLEDcontroller;
internal_tmphum   gIntTempHum;
external_temp     gExtTemperature;
relay_manager     gRelayManager;
disp_manager      gDispManager;

void setup() {
  bool int_temp_ok = true, ext_temp_ok = true;

  Serial.begin(115200);
  Serial.println();

  gRelayManager.relay_init();


  //Initialise the LED controller.
  gLEDcontroller.led_init();
  //gLEDcontroller.theaterChase(0xFF00FFAA, 100);

  gDispManager.disp_init();

  //This could take a few seconds to initialise.
  int_temp_ok = gIntTempHum.init_temphum();

  ext_temp_ok = gExtTemperature.init_temp();


#ifndef DEV_MODE
  if ( (!int_temp_ok) || (!ext_temp_ok))
  {
    //Alert user, system has failed to init!
    Serial.println("--------- SETUP FAILED ---------");
    //Show setup bad on leds
    gLEDcontroller.showSystemError(true);

  }else{
    //Show setup ok on leds
    gLEDcontroller.showSystemWorking();
  }
#endif
      
}



void loop() {

  gApp_data.int_temperature = gIntTempHum.get_temp();
  gApp_data.int_humid = gIntTempHum.get_humd();

  gDispManager.disp_environments(gApp_data.int_temperature, gApp_data.int_humid, 15);




  //Serial.println("Internal temphum");
  //Serial.println(temp);
  //Serial.println(humd);


  float temp = gExtTemperature.get_temp();
  Serial.println("External temphum");
  Serial.println(temp);


  delay(2000);  //recomended polling frequency 8sec..30sec
  gLEDcontroller.setShowSocketStatus(gApp_data.light_relay, 
                                      gApp_data.heater_relay, 
                                      gApp_data.fan_relay, 
                                      gApp_data.misc_relay);
}
