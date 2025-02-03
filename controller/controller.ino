
#include "LED_controller.h"
#include "int_temphum_controller.h"


//Pixel LED controller.
LED_Controller gLEDcontroller;
internal_tmphum gIntTempHum;

void setup() {
  bool setup_ok = true;

  Serial.begin(115200);
  Serial.println();


  //Initialise the LED controller.
  gLEDcontroller.led_init();
  //gLEDcontroller.theaterChase(0xFF00FFAA, 100);


  //This could take a few seconds to initialise.
  setup_ok = gIntTempHum.init_temphum();




  if (!setup_ok) {
    //Alert user, system has failed to init!
    Serial.println("--------- SETUP FAILED ---------");
  }
}


float temp, humd;
void loop() {

  temp = gIntTempHum.get_temp();
  humd = gIntTempHum.get_humd();

  Serial.println(temp);
  Serial.println(humd);

  delay(5000);  //recomended polling frequency 8sec..30sec
}
