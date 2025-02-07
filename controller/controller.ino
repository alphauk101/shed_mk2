
#include "LED_controller.h"             //LEDs (Neo pixel)
#include "int_temphum_controller.h"     //Internal AHT25
#include "ext_temp_controller.h"        //External DS18b20 temperature probe
#include "relay_manager.h"




typedef struct{
  bool    heater_relay;
  bool    light_relay;
  bool    misc_relay;
  bool    fan_relay;


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

void setup() {
  bool int_temp_ok = true, ext_temp_ok = true;

  Serial.begin(115200);
  Serial.println();


  gRelayManager.relay_init();


  //Initialise the LED controller.
  gLEDcontroller.led_init();
  //gLEDcontroller.theaterChase(0xFF00FFAA, 100);


  //This could take a few seconds to initialise.
  int_temp_ok = gIntTempHum.init_temphum();

  ext_temp_ok = gExtTemperature.init_temp();


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

      
}


float temp, humd;
void loop() {

  temp = gIntTempHum.get_temp();
  humd = gIntTempHum.get_humd();

  Serial.println("Internal temphum");
  Serial.println(temp);
  Serial.println(humd);


  temp = gExtTemperature.get_temp();
  Serial.println("External temphum");
  Serial.println(temp);


  delay(5000);  //recomended polling frequency 8sec..30sec
  gLEDcontroller.setShowSocketStatus(gApp_data.light_relay, 
                                      gApp_data.heater_relay, 
                                      gApp_data.fan_relay, 
                                      gApp_data.misc_relay);
}
