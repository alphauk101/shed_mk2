#include <Wire.h>
#include "IOEXP_drv.h"
#include "int_temphum.h"
#include "ext_temphum.h"
#include "shdmk3_config.h"


#define MAIN_LOOP_DELAY 10

//Local drivers
IOEXP_DRV g_IOEXP_driver;
INTTMPHUM g_intTempHum;
EXTTEMP g_extTemp;

typedef struct {
  unsigned long environment_timer = ENVIRONMENT_SAMPLE_TIME;
} APP_TIMERS;

typedef struct {
  float internal_temp = 0;
  float internal_humidity = 0;
  float external_temp = 0;

  APP_TIMERS app_timers;
} SHED_APP;

//Single point of truth for app data/manager
static SHED_APP g_shed_data;


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
static void check_task_timers() {
  unsigned long current_time = millis();


  //Temp and hum sensing
  if ((current_time - g_shed_data.app_timers.environment_timer) > ENVIRONMENT_SAMPLE_TIME) {
    g_shed_data.app_timers.environment_timer = millis();
    get_environment_sensors();
  }
}



void loop() {

  //g_IOEXP_driver.set_relay_pins(true, true, true, true);
  //delay(2000);
  //g_IOEXP_driver.set_relay_pins(false, false, false, false);
  //Serial.println("wjhopg");



  //Do all time related tasks... blocking.
  check_task_timers();

  delay(MAIN_LOOP_DELAY);
}
