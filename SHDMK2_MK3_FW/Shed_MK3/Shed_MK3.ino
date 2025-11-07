#include <Wire.h>
#include "IOEXP_drv.h"
#include "int_temphum.h"
#include "ext_temphum.h"
#include "neopxl_drv.h"
#include "screen_driver.h"

#include "shdmk3_config.h"


#define MAIN_LOOP_DELAY 10

//Local drivers
IOEXP_DRV g_IOEXP_driver;
INTTMPHUM g_intTempHum;
EXTTEMP g_extTemp;
SHDPIXEL g_led_driver;
SCRNDRV g_screen_driver;

//Single point of truth for app data/manager
static SHED_APP g_shed_data;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  int wait = 1000;
  while (!Serial) {
    wait--;
    delay(1);
    if (wait == 0) break;
  }

  //MUST BE INIT'D BEFORE USING AN PERIPHERALS
  Wire.begin();

  Serial.println("Shed MK3 - V0.1 ... Starting");

  g_screen_driver.init();
  g_screen_driver.setStartUpMessage();
  delay(250);
  //setup ios
  if (g_IOEXP_driver.poweron_setup()) {

    //set default states
    g_IOEXP_driver.set_relay_pins(RELAY_FAN_OFF, RELAY_DRYER_OFF, REALY_LIGHT_OFF, RELAY_MISC_OFF);
    g_screen_driver.updateStartUpMessage("IO Expander... OK", "", "", "", "");
  } else {
    g_screen_driver.updateStartUpMessage("IO Expander... ERROR!", "", "", "", "");
  }

  delay(500);

  //Setup i2c temp hum sensor
  if (g_intTempHum.init()) {
    g_screen_driver.updateStartUpMessage("", "Internal Temp... OK", "Internal Humidity... OK", "", "");
  } else {
    g_screen_driver.updateStartUpMessage("", "Internal Temp... ERROR", "Internal Humidity... ERROR", "", "");
  }

  delay(100);

  if (g_extTemp.init()) {
    g_screen_driver.updateStartUpMessage("", "", "", "External Temp... OK", "");
  } else {
    g_screen_driver.updateStartUpMessage("", "", "", "External Temp... ERROR", "");
  }
  delay(100);

  Serial.println("Shed MK3 - V0.1 ... complete");
  g_led_driver.init();
  g_screen_driver.updateStartUpMessage("", "", "", "", "LED Driver... OK");
  //g_screen_driver.setStartUpMessage("Completed...");

  //Set the startup network state
  g_screen_driver.setNetworkState(networkState_icon::not_connected);
  delay(500);
}

static void get_environment_sensors() {

  float t = g_intTempHum.get_temperature();
  float h = g_intTempHum.get_humidity();
  float et = g_extTemp.getTemp();


  if (!isnan(t)) {  // check if 'is not a number'
    g_shed_data.internal_temp = t;
#if DEBUG_ENVIRONMENTS
    Serial.print("Temp *C = ");
    Serial.print(t);
    Serial.println("");
#endif
  } else {
#if DEBUG_ENVIRONMENTS
    Serial.println("Failed to read temperature");
#endif
  }

  if (!isnan(h)) {  // check if 'is not a number'
    g_shed_data.internal_humidity = h;
#if DEBUG_ENVIRONMENTS
    Serial.print("Hum. % = ");
    Serial.println(h);
#endif
  } else {
#if DEBUG_ENVIRONMENTS
    Serial.println("Failed to read humidity");
#endif
  }

  if (!isnan(et)) {  // check if 'is not a number'
    g_shed_data.external_temp = et;
#if DEBUG_ENVIRONMENTS
    Serial.print("External temp *C = ");
    Serial.print(et);
    Serial.println("");
#endif
  } else {
#if DEBUG_ENVIRONMENTS
    Serial.println("Failed to read external temp sensor");
#endif
  }
}

/*
Checks all timers related to application tasks
*/
//int count = 0;
static void check_task_timers() {
  unsigned long current_time = millis();


  //Temp and hum sensing
  if ((current_time - g_shed_data.app_timers.environment_timer) > ENVIRONMENT_SAMPLE_TIME) {
    g_shed_data.app_timers.environment_timer = millis();
    get_environment_sensors();
  }

  //Check LED default state
  if ((current_time - g_shed_data.app_timers.led_timer) > LED_DEFAULT_TIME) {
    g_led_driver.show_temperature_as_color(g_shed_data.internal_temp);
    //g_led_driver.show_temperature_as_color(count++);
    g_shed_data.app_timers.led_timer = millis();
  }
}






void loop() {

  //g_IOEXP_driver.set_relay_pins(true, true, true, true);
  //delay(2000);
  //g_IOEXP_driver.set_relay_pins(false, false, false, false);
  //Serial.println("wjhopg");



  //Do all time related tasks... blocking.
  check_task_timers();


  //Any module tasks should be done here
  g_led_driver.task(g_shed_data.system_asleep);
  delay(MAIN_LOOP_DELAY);
}
