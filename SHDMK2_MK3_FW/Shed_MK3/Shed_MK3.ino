#include <Wire.h>
#include "IOEXP_drv.h"
#include "int_temphum.h"
#include "ext_temphum.h"
#include "neopxl_drv.h"
#include "screen_driver.h"
#include "network_manager.h"
#include "rtc_driver.h"

#include "shdmk3_config.h"

#define DEFAULT_RELAY_STATE  false

#define PRINTOUT(X) Serial.println(X)

#define MAIN_LOOP_DELAY 10

//Local drivers
IOEXP_DRV g_IOEXP_driver;
INTTMPHUM g_intTempHum;
EXTTEMP g_extTemp;
SHDPIXEL g_led_driver;
SCRNDRV g_screen_driver;
NETMANAGER g_network_manager;
RTCDRV g_rtc_driver;


//Single point of truth for app data/manager
static SHED_APP g_shed_data;
static int RTC_fail_count = 0;
#define MAX_RTC_ATTEMPTS 3  //If rtc fails to set, max amount before giving up so not to get IP blocked on the NTP

static void RTC_callback_event(int evt) {
  switch (evt) {
    case RTC_EVENT_RESET_NTP_ATTEMPTS:
      //Reset this as not to run out of attempts
      PRINTOUT("Reseting NTP attempts");
      RTC_fail_count = 0;
      break;
  }
}

#define NO_DELAY_STARTUP
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  int wait = 1000;
  while (!Serial) {
    wait--;
    delay(1);
    if (wait == 0) break;
  }

  //Set the default relay state 
  g_shed_data.power_states.blower = DEFAULT_RELAY_STATE;
  g_shed_data.power_states.lights = DEFAULT_RELAY_STATE;
  g_shed_data.power_states.fan = DEFAULT_RELAY_STATE;
  g_shed_data.power_states.misc = DEFAULT_RELAY_STATE;

  //MUST BE INIT'D BEFORE USING AN PERIPHERALS
  Wire.begin();

  PRINTOUT("Shed MK3 - V0.1 ... Starting");

  g_screen_driver.init();
  g_screen_driver.setStartUpMessage();
  g_screen_driver.updateStartUpMessage("IO Expander...", "Internal Temp...", "Internal Humidity...", "External Temp...", "LED Driver...", "Network...", "RTC...");


  //setup ios
  if (g_IOEXP_driver.poweron_setup()) {
    //set default states
    g_IOEXP_driver.set_relay_pins(RELAY_FAN_OFF, RELAY_DRYER_OFF, REALY_LIGHT_OFF, RELAY_MISC_OFF);
    g_screen_driver.updateStartUpMessage("IO Expander... OK", "", "", "", "", "", "");
  } else {
    g_screen_driver.updateStartUpMessage("IO Expander... ERROR!", "", "", "", "", "", "");
  }
  //Setup i2c temp hum sensor
  if (g_intTempHum.init()) {
    g_screen_driver.updateStartUpMessage("", "", "Internal Humidity... OK", "", "", "", "");
    g_screen_driver.updateStartUpMessage("", "Internal Temp... OK", "", "", "", "", "");
    g_shed_data.environmentals.internal_humidity_max = g_shed_data.environmentals.internal_humidity_min = g_shed_data.environmentals.internal_humidity = g_intTempHum.get_humidity();
    g_shed_data.environmentals.internal_temp_max = g_shed_data.environmentals.internal_temp_min = g_shed_data.environmentals.internal_temp = g_intTempHum.get_temperature();
  } else {
    g_screen_driver.updateStartUpMessage("", "", "Internal Humidity... ERROR", "", "", "", "");
    g_screen_driver.updateStartUpMessage("", "Internal Temp... ERROR", "", "", "", "", "");
  }

  if (g_extTemp.init()) {
    g_screen_driver.updateStartUpMessage("", "", "", "External Temp... OK", "", "", "");
    g_shed_data.environmentals.external_temp = g_shed_data.environmentals.external_temp_max = g_shed_data.environmentals.external_temp_min = g_extTemp.getTemp();
  } else {
    g_screen_driver.updateStartUpMessage("", "", "", "External Temp... ERROR", "", "", "");
  }

#ifndef NO_DELAY_STARTUP
  //Serial.println("Shed MK3 - V0.1 ... complete");
  g_led_driver.init();
  g_screen_driver.updateStartUpMessage("", "", "", "", "LED Driver... OK", "", "");
  //g_screen_driver.setStartUpMessage("Completed...");
#endif

  //Set the startup network state
  g_screen_driver.setNetworkState(not_connected);


  if (g_network_manager.init(&g_screen_driver)) {
    //wifi connected
    g_screen_driver.updateStartUpMessage("", "", "", "", "", "Network... OK", "");
  } else {
    g_screen_driver.updateStartUpMessage("", "", "", "", "", "Network... Failed", "");
  }

  if (g_rtc_driver.init(&RTC_callback_event)) {
    g_screen_driver.updateStartUpMessage("", "", "", "", "", "", "RTC...OK");
  } else {
    g_screen_driver.updateStartUpMessage("", "", "", "", "", "", "RTC...ERROR");
  }

#ifndef NO_DELAY_STARTUP
  //Small delay to allow visual confirmation
  delay(2000);
#endif
}


static void get_environment_sensors() {

  float t = g_intTempHum.get_temperature();
  float h = g_intTempHum.get_humidity();
  float et = g_extTemp.getTemp();


  if (!isnan(t)) {  // check if 'is not a number'
    g_shed_data.environmentals.internal_temp = t;
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
    g_shed_data.environmentals.internal_humidity = h;
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
    g_shed_data.environmentals.external_temp = et;
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
  UL_TIMER_t current_time = millis();


  //Temp and hum sensing
  if ((current_time - g_shed_data.app_timers.environment_timer) > ENVIRONMENT_SAMPLE_TIME) {
    g_shed_data.app_timers.environment_timer = current_time;
    get_environment_sensors();
  }

  //Check LED default state
  if ((current_time - g_shed_data.app_timers.led_timer) > LED_DEFAULT_TIME) {
    g_led_driver.show_temperature_as_color(g_shed_data.environmentals.internal_temp);
    //g_led_driver.show_temperature_as_color(count++);
    g_shed_data.app_timers.led_timer = current_time;
  }

  //Check screen
  if ((current_time - g_shed_data.app_timers.screen_timer) > SCREEN_UPDATE_TIME) {
    g_screen_driver.setPowerStates(g_shed_data.power_states.lights,
                                   g_shed_data.power_states.fan,
                                   g_shed_data.power_states.blower,
                                   g_shed_data.power_states.misc);


    networkState_icon net_icon = convertRSSIToIcon();
    g_screen_driver.setNetworkState(net_icon);
    g_screen_driver.task(g_shed_data.system_asleep, &g_shed_data);
    g_shed_data.app_timers.screen_timer = current_time;
  }

  if ((current_time - g_shed_data.app_timers.network_timer) > NETWORK_TASK_CHECK) {
    if (g_network_manager.isConnected()) {
      //g_network_manager.getTime();
    } else {
      //try to connect
      Serial.print("Wifi not connected");
    }
    g_shed_data.app_timers.network_timer = current_time;
  }

  //Check the RTC task
  if ((current_time - g_shed_data.app_timers.rtc_timer) > RTC_TIMER_TASK) {
    if (g_rtc_driver.task(&g_shed_data)) {
      //time is good do what we want here

    } else {
      /*The RTC has reported not running or not sest, this will try to 
      get the UTC time from an NTP however, to avoid getting IP blocked 
      we have amaximum of 2 tries and then it gives up. The attempts 
      are reset at midnight, this of course wont work if the RTC is not 
      functioning howver, at this point there is a bigger issue anyway.*/
      PRINTOUT("RTC not set requesting time");
      if (g_network_manager.isConnected()) {
        if (RTC_fail_count < MAX_RTC_ATTEMPTS) {
          unsigned long epoch = g_network_manager.getTime();
          RTC_fail_count++;
          if (epoch != 0) {
            PRINTOUT("Time recieved setting RTC from network");
            g_rtc_driver.setTIme(epoch);
          }
        } else {
          // no longer trying to get time from NTP
          PRINTOUT("NTP protection active!");
        }
      }
    }
    g_shed_data.app_timers.rtc_timer = current_time;
  }
}






networkState_icon convertRSSIToIcon() {
  long rssi = g_network_manager.getRSSI();
  // Serial.print("RSSI: ");
  //Serial.println(rssi);

  // A value of 0 often means "not available" or "no signal"
  if (rssi == 0) {
    return signal_none;
  }
  // Remember: -50 is GREATER than -70
  if (rssi >= -60) {
    return signal_good;  // -60 dBm or better
  } else if (rssi >= -70) {
    return signal_ok;  // -61 to -70 dBm
  } else if (rssi >= -80) {
    return signal_bad;  // -71 to -80 dBm
  } else {
    return signal_awful;  // -81 dBm or worse
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
