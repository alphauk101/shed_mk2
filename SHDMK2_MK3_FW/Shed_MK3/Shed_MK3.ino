#include <Wire.h>
#include <math.h>
#include "IOEXP_drv.h"
#include "int_temphum.h"
#include "ext_temphum.h"
#include "neopxl_drv.h"
#include "screen_driver.h"
#include "network_manager.h"
#include "rtc_driver.h"
#include <arduino-timer.h>



#include "shdmk3_config.h"

#define DEFAULT_RELAY_STATE false

#define MCR_SET_RELAY_STATES g_IOEXP_driver.set_relay_pins(g_shed_data.power_states.blower, g_shed_data.power_states.lights, g_shed_data.power_states.misc, g_shed_data.power_states.fan);


#ifdef DEBUG_OUT
#define PRINTOUT(X) Serial.println(X)

//Use this to disable serial printout
//#define PRINTOUT(X) ;

#else
#define PRINTOUT(X) ;
#endif


uint8_t button_debounce = 0;
#define DOOR_STS_DEBOUNCE_COUNT 50

#define MAIN_LOOP_DELAY 10

auto timer = timer_create_default();  // create a timer with default settings

//Local drivers
static IOEXP_DRV g_IOEXP_driver;
static INTTMPHUM g_intTempHum;
static EXTTEMP g_extTemp;
static SHDPIXEL g_led_driver;
static SCRNDRV g_screen_driver;
static NETMANAGER g_network_manager;
static RTCDRV g_rtc_driver;
//Single point of truth for app data/manager
static SHED_APP g_shed_data;

UL_TIMER_t fw_led_timer = 0;
UL_TIMER_t WIFI_check_timer = 0;
static int RTC_fail_count = 0;


/***
  1hz timer callback useful for dec/inc app timers
  This is an SW interrupt caused by the timer module.
***/
bool onehz_callback(void *) {

  //Tick the timers if necessary
  if (g_shed_data.app_timers.sys_sleep_timer > 0)
    g_shed_data.app_timers.sys_sleep_timer--;

  if (g_shed_data.app_timers.dryer_timer > 0)
    g_shed_data.app_timers.dryer_timer--;

#if NETWORK_POST_METRICS_SECS > 0
  if (g_shed_data.app_timers.network_post_data > NETWORK_POST_METRICS_SECS) {
    PRINTOUT(">>>> Doing data post <<<<");
    g_shed_data.app_timers.network_post_data = 0;

    if (g_network_manager.isConnected())
      g_network_manager.do_metrics_post(&g_shed_data, TRIGGER_TYPE_TIMER);
  } else {
    g_shed_data.app_timers.network_post_data++;
  }
#endif
  if (WIFI_check_timer > WIFI_CONNECT_CHECK_SECS) {
    WIFI_check_timer = 0;
    if (!g_network_manager.isConnected()) {
      PRINTOUT("Network disconnected, retrying...");
      //This will block but this should be called when the network is not connected and at once a minute.
      g_network_manager.connect_to_WIFI_network(&g_screen_driver);
    }
  }else{
    WIFI_check_timer++;
  }
  return true;
}

static void RTC_callback_event(int evt) {
  switch (evt) {
    case RTC_EVENT_RESET_NTP_ATTEMPTS:
      //Reset this as not to run out of attempts
      PRINTOUT("Reseting NTP attempts");
      RTC_fail_count = 0;
      break;
  }
}

/*Callback from IOExp when button is pressed.*/
static void bttn_callback(int button, bool state) {
  switch (button) {
    case BUTTON_A:
      //Changes screen when pressed
      if (state) g_screen_driver.changeViewingScreen();
      PRINTOUT("BUTTON A");
      break;
    case BUTTON_B:
      PRINTOUT("BUTTON B");
      //Add time to fan timer
      if (state) g_shed_data.app_timers.dryer_timer += DRYER_TIME_MINUTES_SECS;
      Serial.println(g_shed_data.app_timers.dryer_timer, DEC);
      break;
    default:
      break;
  }
}

//#define NO_DELAY_STARTUP
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  int wait = 1000;
  while (!Serial) {
    wait--;
    delay(1);
    if (wait == 0) break;
  }



//#define GRAB_UUID
#ifdef GRAB_UUID
  // Read the 128-bit unique ID
  volatile uint32_t val[4];
  val[0] = *(volatile uint32_t *)0x008061FC;
  val[1] = *(volatile uint32_t *)0x00806010;
  val[2] = *(volatile uint32_t *)0x00806014;
  val[3] = *(volatile uint32_t *)0x00806018;

  // Print the ID
  Serial.print("MCU UUID: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(val[i], HEX);
  }
  Serial.println();
#endif

  g_shed_data.network_info.connected = false;

  //Set the default relay state
  g_shed_data.power_states.blower = DEFAULT_RELAY_STATE;
  g_shed_data.power_states.lights = DEFAULT_RELAY_STATE;
  g_shed_data.power_states.fan = DEFAULT_RELAY_STATE;
  g_shed_data.power_states.misc = DEFAULT_RELAY_STATE;
  g_shed_data.door_status.current_state = false;
  g_shed_data.door_status.open_counter = 0;

  pinMode(DOOR_STATUS_PIN, INPUT);

  //MUST BE INIT'D BEFORE USING AN PERIPHERALS
  Wire.begin();

  PRINTOUT("Shed MK3 - V0.2 ... Starting");

  g_screen_driver.init();
  g_screen_driver.setStartUpMessage();
  g_screen_driver.updateStartUpMessage("IO Expander...", "Internal Temp...", "Internal Humidity...", "External Temp...", "LED Driver...", "Network...", "RTC...");

  PRINTOUT("Screen ok...");

  //setup ios
  if (g_IOEXP_driver.poweron_setup(bttn_callback)) {
    g_screen_driver.updateStartUpMessage("IO Expander... OK", "", "", "", "", "", "");
    PRINTOUT("IO Exp, OK.");
  } else {
    g_screen_driver.updateStartUpMessage("IO Expander... ERROR!", "", "", "", "", "", "");
    PRINTOUT("IO Exp, ERROR.");
  }
  //Setup i2c temp hum sensor
  if (g_intTempHum.init()) {
    g_screen_driver.updateStartUpMessage("", "", "Internal Humidity... OK", "", "", "", "");
    g_screen_driver.updateStartUpMessage("", "Internal Temp... OK", "", "", "", "", "");
    g_shed_data.environmentals.internal_humidity_max = g_shed_data.environmentals.internal_humidity_min = g_shed_data.environmentals.internal_humidity = g_intTempHum.get_humidity();
    g_shed_data.environmentals.internal_temp_max = g_shed_data.environmentals.internal_temp_min = g_shed_data.environmentals.internal_temp = g_intTempHum.get_temperature();

    PRINTOUT("Internal Temp/Hum, OK.");
  } else {
    g_screen_driver.updateStartUpMessage("", "", "Internal Humidity... ERROR", "", "", "", "");
    g_screen_driver.updateStartUpMessage("", "Internal Temp... ERROR", "", "", "", "", "");
    PRINTOUT("Internal Temp/Hum, ERROR.");
  }

  if (g_extTemp.init()) {
    g_screen_driver.updateStartUpMessage("", "", "", "External Temp... OK", "", "", "");
    g_shed_data.environmentals.external_temp = g_shed_data.environmentals.external_temp_max = g_shed_data.environmentals.external_temp_min = g_extTemp.getTemp();
    PRINTOUT("External Temp, OK.");
  } else {
    g_screen_driver.updateStartUpMessage("", "", "", "External Temp... ERROR", "", "", "");
    PRINTOUT("External Temp, ERROR.");
  }

#ifndef NO_DELAY_STARTUP
  //Serial.println("Shed MK3 - V0.1 ... complete");
  g_led_driver.init();
  g_screen_driver.updateStartUpMessage("", "", "", "", "LED Driver... OK", "", "");
  //g_screen_driver.setStartUpMessage("Completed...");
#endif

  //Set the startup network state
  g_screen_driver.setNetworkState(not_connected);

  g_network_manager.init(&g_screen_driver);
  if (g_network_manager.isConnected()) {
    //wifi connected
    g_network_manager.getIP(g_shed_data.network_info.ip);
    g_screen_driver.updateStartUpMessage("", "", "", "", "", "Network... OK", "");
    //Only send power on message if connected
    g_network_manager.do_metrics_post(&g_shed_data, TRIGGER_TYPE_POWERON);
  } else {
    g_screen_driver.updateStartUpMessage("", "", "", "", "", "Network... Failed", "");
  }

  if (g_rtc_driver.init(&RTC_callback_event)) {
    g_screen_driver.updateStartUpMessage("", "", "", "", "", "", "RTC...OK");
  } else {
    g_screen_driver.updateStartUpMessage("", "", "", "", "", "", "RTC...ERROR");
  }

  timer.every(1000, onehz_callback);

  //Turn the misc relay on to workaround the cheap chinese relay issue.
  //g_shed_data.power_states.misc = RELAY_MISC_OFF;

  /*
  while (1) {
    g_shed_data.power_states.misc = RELAY_OFF;
    g_shed_data.power_states.blower = RELAY_OFF;
    g_shed_data.power_states.fan = RELAY_OFF;
    g_shed_data.power_states.lights = RELAY_OFF;
    MCR_SET_RELAY_STATES;

    delay(500);

    g_shed_data.power_states.misc = RELAY_ON;
    g_shed_data.power_states.blower = RELAY_OFF;
    g_shed_data.power_states.fan = RELAY_OFF;
    g_shed_data.power_states.lights = RELAY_OFF;
    MCR_SET_RELAY_STATES;

    delay(500);
  }
*/

#ifndef NO_DELAY_STARTUP
  //Small delay to allow visual confirmation
  delay(2000);
#endif
}


/**
 * @brief Calculates the dew point in Celsius using single-precision floats.
 *
 * This function uses the Magnus-Tetens formula (a common approximation)
 * to calculate the dew point from the ambient temperature and relative humidity.
 *
 * @param temperatureCelsius The current air temperature in degrees Celsius (float).
 * @param relativeHumidity The current relative humidity as a percentage (float).
 * @return The dew point temperature in degrees Celsius (float).
 */
float calculateDewPointF(float temperatureCelsius, float relativeHumidity) {
  // Check for edge cases
  if (relativeHumidity < 0.01f) {
    return -273.15f;  // Return a sentinel value
  }
  if (relativeHumidity > 100.0f) {
    relativeHumidity = 100.0f;  // Cap at 100%
  }

  // Constants for the Magnus-Tetens formula (single-precision)
  const float B = 17.67f;
  const float C = 243.5f;  // °C

  // Use std::logf() which is the C++ standard library function
  // for calculating the natural logarithm of a float.
  float gamma = logf(relativeHumidity / 100.0f) + (B * temperatureCelsius) / (C + temperatureCelsius);

  // This is the final step, inverting the formula to solve for T_d (dew point)
  float dewPoint = (C * gamma) / (B - gamma);

  return dewPoint;
}


static void get_environment_sensors() {

  float t = g_intTempHum.get_temperature();
  float h = g_intTempHum.get_humidity();
  float et = g_extTemp.getTemp();

  if ((!isnan(t)) && (!isnan(h))) {  // check if 'is not a number'
    g_shed_data.environmentals.internal_humidity = h;
    g_shed_data.environmentals.internal_temp = t;
    g_shed_data.environmentals.internal_dewpoint = calculateDewPointF(t, h);

    //Check max/min values if applicable
    if (g_shed_data.environmentals.internal_temp < g_shed_data.environmentals.internal_temp_min)
      g_shed_data.environmentals.internal_temp_min = g_shed_data.environmentals.internal_temp;

    if (g_shed_data.environmentals.internal_temp > g_shed_data.environmentals.internal_temp_max)
      g_shed_data.environmentals.internal_temp_max = g_shed_data.environmentals.internal_temp;

    //Check max/min values if applicable
    if (g_shed_data.environmentals.internal_humidity < g_shed_data.environmentals.internal_humidity_min)
      g_shed_data.environmentals.internal_humidity_min = g_shed_data.environmentals.internal_humidity;

    if (g_shed_data.environmentals.internal_humidity > g_shed_data.environmentals.internal_humidity_max)
      g_shed_data.environmentals.internal_humidity_max = g_shed_data.environmentals.internal_humidity;

#if DEBUG_ENVIRONMENTS
    Serial.print("Temp *C = ");
    Serial.print(t);
    Serial.println("");

    Serial.print("Calc dewpoint *C = ");
    Serial.print(g_shed_data.environmentals.internal_dewpoint);
    Serial.println("");

    Serial.print("Hum. % = ");
    Serial.println(h);
#endif

  } else {
#if DEBUG_ENVIRONMENTS
    Serial.println("Failed to internal temp and humd");
#endif
  }

  if (!isnan(et)) {  // check if 'is not a number'
    g_shed_data.environmentals.external_temp = et;
    check_fan_state();
#if DEBUG_ENVIRONMENTS
    Serial.print("External temp *C = ");
    Serial.print(et);
    Serial.println("");
#endif

    if (g_shed_data.environmentals.external_temp < g_shed_data.environmentals.external_temp_min)
      g_shed_data.environmentals.external_temp_min = g_shed_data.environmentals.external_temp;

    if (g_shed_data.environmentals.external_temp > g_shed_data.environmentals.external_temp_max)
      g_shed_data.environmentals.external_temp = g_shed_data.environmentals.external_temp;

  } else {
#if DEBUG_ENVIRONMENTS
    Serial.println("Failed to read external temp sensor");
#endif
  }
}


void check_fan_state() {
  if (g_shed_data.environmentals.external_temp < FAN_OFF_TEMPERATURE) {
    if (g_shed_data.power_states.fan == RELAY_FAN_ON) {
      g_shed_data.power_states.fan = RELAY_FAN_OFF;  //make sure it reflects the correct state when sent.
      //Create a interrupt type event if the fan has just turned off
      if (g_network_manager.isConnected())
          g_network_manager.do_metrics_post(&g_shed_data, TRIGGER_TYPE_INTERRUPT);
    }
  }

  if (g_shed_data.environmentals.external_temp > FAN_ON_TEMPERATURE) {
    g_shed_data.power_states.fan = RELAY_FAN_ON;
  }
}


static void check_task_timers() {
  UL_TIMER_t current_time = millis();

  //////////BELOW ARE NONE TIME RESTRICTED TASKS/////////////
  g_led_driver.task(g_shed_data.system_asleep);

  if ((current_time - fw_led_timer) > 500) {
    g_IOEXP_driver.toggle_firmwareLED_pin();
    fw_led_timer = current_time;
  }

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
    networkState_icon net_icon = convertRSSIToIcon();
    g_screen_driver.setNetworkState(net_icon);

    g_screen_driver.task( &g_shed_data, g_shed_data.system_asleep, g_network_manager.isConnected());
    g_shed_data.app_timers.screen_timer = current_time;
  }

  //Check the RTC task
  if ((current_time - g_shed_data.app_timers.rtc_timer) > RTC_TIMER_TASK) {
    if (g_rtc_driver.task(&g_shed_data)) {
      //Get a snapshot of the time
      g_rtc_driver.getLatestTime(&g_shed_data.last_timestammp);

    } else {
      /*The RTC has reported not running or not sest, this will try to 
      get the UTC time from an NTP however, to avoid getting IP blocked 
      we have amaximum of 2 tries and then it gives up. The attempts 
      are reset at midnight, this of course wont work if the RTC is not 
      functioning howver, at this point there is a bigger issue anyway.*/
      PRINTOUT("RTC not set requesting time");
      if (g_network_manager.isConnected()) {
        if (RTC_fail_count < MAX_RTC_ATTEMPTS) {
          //auto epoch = g_network_manager.getTime();
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

  //safe to call rapidly
  g_network_manager.task();
  g_IOEXP_driver.task();

  //checks whether the lights can be turned off to save power
  check_light_state();  
}




networkState_icon convertRSSIToIcon() {
  g_shed_data.network_info.latest_RSSI = g_network_manager.getRSSI();
  // Serial.print("RSSI: ");
  //Serial.println(rssi);

  // A value of 0 often means "not available" or "no signal"
  if (g_shed_data.network_info.latest_RSSI == 0) {
    return signal_none;
  }
  // Remember: -50 is GREATER than -70
  if (g_shed_data.network_info.latest_RSSI >= -60) {
    return signal_good;  // -60 dBm or better
  } else if (g_shed_data.network_info.latest_RSSI >= -70) {
    return signal_ok;  // -61 to -70 dBm
  } else if (g_shed_data.network_info.latest_RSSI >= -80) {
    return signal_bad;  // -71 to -80 dBm
  } else {
    return signal_awful;  // -81 dBm or worse
  }
}

#define DOOR_OPEN true
#define DOOR_CLOSED false
void doorStateChanged(bool DS) {

  g_shed_data.door_status.current_state = DS;

  if (DS == DOOR_OPEN) {
    PRINTOUT("DOOR STATE CHANGED - open");
    wake_up();
  } else {
    PRINTOUT("DOOR STATE CHANGED - closed");
    //The door state has changed update
    start_sleep_countdown();
  }
}


void start_sleep_countdown() {
  g_shed_data.app_timers.sys_sleep_timer = COUNTDOWN_TIME_SECONDS;
  g_shed_data.sleep_countdown_act = true;
}

void wake_up() {
  g_shed_data.app_timers.sys_sleep_timer = 0;
  g_shed_data.sleep_countdown_act = false;

  //wake the system
  g_shed_data.system_asleep = false;
  g_shed_data.app_timers.lightsaver_timer = 0;

  //g_led_driver.show_action_swipe(PXL_RED);
  //trigger an interrupt message to the server.
  if (g_network_manager.isConnected())
        g_network_manager.do_metrics_post(&g_shed_data, TRIGGER_TYPE_INTERRUPT);
}


void checkDoorState() {
  DateTime tmpDT;
  bool doorState = (digitalRead(DOOR_STATUS_PIN) == HIGH) ? DOOR_OPEN : DOOR_CLOSED;
  if (doorState != g_shed_data.door_status.current_state) {
    //debounce
    if (button_debounce < DOOR_STS_DEBOUNCE_COUNT) {
      button_debounce++;
    } else {
      if (g_shed_data.door_status.current_state)
        g_shed_data.door_status.open_counter++;

      //grab last door closed as last door open is not useful when viewed.... you had to open it to view it!
      if (doorState == DOOR_CLOSED) {
        if (g_rtc_driver.getLatestTime(&tmpDT)) {
          g_shed_data.door_status.last_opened = tmpDT;
        }
      }
      doorStateChanged(doorState);
    }
  } else {
    button_debounce = 0;
  }
}

viod check_light_state()
{

  if(! g_shed_data.system_asleep)
  {
    //This only applies when the system is awake.

    //1. check the PIR state
    if(g_IOEXP_driver.get_pir_state())
    {
      //The PIR has seen someone, make the sure the lights are on and the timer is started
      g_shed_data.power_states.lights = true;
      MCR_SET_RELAY_STATES;

      g_shed_data.app_timers.lightsaver_timer = 0;
    }

  }//system is asleep do nothing


}



void check_blower_relay_timer() {
  //If this is greater than zero then dryer is on!
  if (g_shed_data.app_timers.dryer_timer > 0) {
    g_shed_data.power_states.blower = RELAY_BLOWER_ON;
  } else {
    g_shed_data.power_states.blower = RELAY_BLOWER_OFF;
  }
}



void loop() {

  //Do all time related tasks... blocking.
  check_task_timers();
  checkDoorState();  //check the door state

  g_IOEXP_driver.set_statusLED_pin(g_shed_data.door_status.current_state);
  check_blower_relay_timer();

  if (g_shed_data.sleep_countdown_act) {
    if (g_shed_data.app_timers.sys_sleep_timer == 0) {
      //timer exp
      g_shed_data.system_asleep = true;
      g_shed_data.sleep_countdown_act = false;
      g_shed_data.show_asleep_LEDS = SHOW_SLEEP_BLINKS;

    }
  }
  //Set the lights, its ok to call the fxn repeatedly
  g_shed_data.power_states.lights = (g_shed_data.system_asleep) ? RELAY_LIGHT_OFF : RELAY_LIGHT_ON;

  //show LED swipe to indicate asleep.
  if (g_shed_data.show_asleep_LEDS > 0) {
    g_led_driver.box_wipe(true, 50, PXL_GREEN);
    g_shed_data.show_asleep_LEDS--;
  }


  timer.tick();  // tick the timer
  MCR_SET_RELAY_STATES;
  delay(MAIN_LOOP_DELAY);
}
