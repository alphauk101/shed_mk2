#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "LED_controller.h"          //LEDs (Neo pixel)
#include "int_temphum_controller.h"  //Internal AHT25
#include "ext_temp_controller.h"     //External DS18b20 temperature probe
#include "relay_manager.h"
#include "disp_manager.h"
#include "arduino-timer.h"
#include "ATWIFI_driver.h"
#include "config.h"

//if defd sends data to server
#define HTTP_SEND_METRICS
//#define DEV_MODE  //If en, turns off pointless animas etc.

#define SOCKET_CHECK_TIME_SEC 2

#define SLEEP_BLINK_TIME 30

#define DOOR_CLOSED LOW
#define DOOR_OPEN HIGH

//Enables the software serial for outputting messages
#define DEBUG_SERIAL

typedef struct {
  String JSON_data;

  bool data_waiting;
} http_metrics_data;

typedef struct {
  //Relay states
  bool heater_relay;
  bool light_relay;
  bool misc_relay;
  bool fan_relay;

  bool door_state;
  uint8_t led_countdown;

  //Temperature data.
  float int_temperature;
  float int_humid;
  float ext_temperature;

  //FLag to indicate if the system is sleeping or not.
  bool system_sleeping;

  uint64_t sys_time;

  uint64_t environ_timer;

  uint64_t socket_timer;

  uint64_t doorclosed_timer;

  bool ds_irq_triggered;

  http_metrics_data http_data;
} APP_LOGIC_DATA;
static APP_LOGIC_DATA gApp_data;
static atwifi_driver g_network_module;

uint8_t leds_lit = 0;

//Pixel LED controller.
LED_Controller gLEDcontroller;
internal_tmphum gIntTempHum;
external_temp gExtTemperature;
relay_manager gRelayManager;
disp_manager gDispManager;

auto timer_1hz = timer_create_default();  // create a timer with default settings

#ifdef DEBUG_SERIAL
SoftwareSerial mySerial(7, 11);  // RX, TX
#endif

static unsigned long ms_timer = 0;
bool sys_tick_irq(void *) {
  gApp_data.sys_time++;
  if ((gApp_data.sys_time % 10) == 0) {
    if (gApp_data.led_countdown > 0) gApp_data.led_countdown--;
  }
  return true;  // repeat? true
}


//Event handler for wifi
static void wifi_eventhandler(int evt) {

  switch (evt) {
    case EVNT_SETUP_OK:
#ifdef DEBUG_SERIAL
    mySerial.println("Network setup - OK");
#endif
      break;
    case EVNT_SETUP_FAIL:
#ifdef DEBUG_SERIAL
    mySerial.println("Network setup - FAILED");
#endif
      break;
    case EVNT_CONNECTION_OK:
#ifdef DEBUG_SERIAL
    mySerial.println("Connection - OK - starting POST");
#endif
      //Connection is active, if we have some data -> SEND IT!!!!!
      if (gApp_data.http_data.data_waiting) {
         g_network_module.startHTTPPost(&gApp_data.http_data.JSON_data,
                                       gApp_data.http_data.JSON_data.length());
         gApp_data.http_data.data_waiting = false;
      }
      break;
    case EVNT_CONNECTION_FAILED:
#ifdef DEBUG_SERIAL
    mySerial.println("Connection - FAILED");
#endif
      break;
    case EVNT_OP_ABORTED_ERROR:
#ifdef DEBUG_SERIAL
    mySerial.println("Network operation - FAILED");
#endif
      break;
    default:
      break;
  }
}


void door_sense_irq() {
  //check_door_state();
}

void setup() {
  bool int_temp_ok = true, ext_temp_ok = true;

#ifdef DEBUG_SERIAL
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  mySerial.println("Debug on SW serial started.");
#endif

  //setup IRQ for door sense
  pinMode(DOOR_SENSE_PIN, INPUT);
  //attachInterrupt(digitalPinToInterrupt(DOOR_SENSE_PIN), door_sense_irq, CHANGE);

  gApp_data.heater_relay = false;
  gApp_data.light_relay = false;
  gApp_data.misc_relay = false;
  gApp_data.fan_relay = false;

  gApp_data.door_state = false;

  gApp_data.ds_irq_triggered = false;

  //Door sensor state
  gApp_data.system_sleeping = false;

  gApp_data.sys_time = 0;
  gApp_data.environ_timer = 0;
  gApp_data.socket_timer = 0;

  Serial.begin(115200);
  //Serial.println();

  gRelayManager.relay_init();

  //Initialise the LED controller.
  gLEDcontroller.led_init();
  //gLEDcontroller.theaterChase(0xFF00FFAA, 100);

  gDispManager.disp_init();

  //This could take a few seconds to initialise.
  int_temp_ok = gIntTempHum.init_temphum();
  ext_temp_ok = gExtTemperature.init_temp();


  if ((!int_temp_ok) || (!ext_temp_ok)) {
#ifdef DEV_MODE
    //Show setup bad on leds
    gLEDcontroller.showSystemError(true);
#endif

  } else {
#ifndef DEV_MODE
    //Show setup ok on leds
    gLEDcontroller.showSystemWorking();
#endif
  }

#ifdef HTTP_SEND_METRICS
  g_network_module.init(wifi_eventhandler);
  gApp_data.http_data.data_waiting = false;
  gApp_data.http_data.JSON_data.reserve(200);
#endif

  //Start the sys timer
  timer_1hz.every(1000, sys_tick_irq);
}

/*If the system has woken up then this is called use this to reset timer values etc.*/
static void system_woken_irq() {
  gApp_data.environ_timer = 0;  //Force a environ resample
  gDispManager.system_sleeping(false);
  gApp_data.socket_timer = 0;
}

void loop() {

  //sample temps and hum
  do_environment_sampling();

  //Set sockets states
  do_check_sockets();

  //All modules should be informed of the sleep state
  gDispManager.system_sleeping(gApp_data.system_sleeping);

  //Door sense logic
  check_door_state();

  if (gApp_data.system_sleeping == false) {
    if (gApp_data.door_state == false) {
      gLEDcontroller.setShowCountdown(gApp_data.led_countdown);
    } else {
      //bool light, bool heater, bool fan, bool misc
      gLEDcontroller.setShowSocketStatus(gApp_data.light_relay,
                                         gApp_data.heater_relay,
                                         gApp_data.fan_relay,
                                         gApp_data.misc_relay);
    }
  } else {
    if ((gApp_data.sys_time % SLEEP_BLINK_TIME) == 0) {
      gLEDcontroller.colourSwell(255, 0, 0, 20);
    }
  }


#ifdef DEV_MODE_SYSTIME
  static uint64_t dev_timer;
  if (gApp_data.sys_time != dev_timer) {
    // Serial.println("=======SYS TIMER======");
    dev_timer = gApp_data.sys_time;
    //  Serial.print((int)gApp_data.sys_time);
    //  Serial.println(" Secs");
  }
#endif


#ifdef HTTP_SEND_METRICS
  g_network_module.task();
#endif


  timer_1hz.tick();  // tick the timer
}

void serialEvent() {
  //lock_in_error();
  //g_network_module.serialEventHandler();
}


/*Samples once a second*/
void do_environment_sampling() {
  if (gApp_data.sys_time > gApp_data.environ_timer) {

    gApp_data.environ_timer = (TEMPHUM_SAMPLE_TIME + gApp_data.sys_time);

    //DO temp and hum readings
    gApp_data.int_temperature = gIntTempHum.get_temp();
    gApp_data.int_humid = gIntTempHum.get_humd();
    gApp_data.ext_temperature = gExtTemperature.get_temp();

    gDispManager.disp_environments(gApp_data.int_temperature,
                                   gApp_data.int_humid,
                                   gApp_data.ext_temperature);
#ifdef HTTP_SEND_METRICS
    do_http_metrics_operation();
#endif

#ifdef DEV_MODE
    //Serial.println("=======INTERNAL======");
    //Serial.print(gApp_data.int_temperature);
    // Serial.println(" C");
    // Serial.print(gApp_data.int_humid);
    // Serial.println(" %");

    //  Serial.println("=======EXTERNAL======");
    //  Serial.print(gApp_data.ext_temperature);
    // Serial.println(" C");
#endif
  }
}

void do_http_metrics_operation() {
  //gApp_data.http_data.JSON_data = get_json_packet();
  gApp_data.http_data.data_waiting = true;

  //This will result in an event
  g_network_module.startGetConnectionStatus();
  //Trigger a cnnection check, this in turn will trigger a data send if data available.
}


String get_json_packet() {
  JsonDocument doc;

  doc["intT"] = gIntTempHum.get_temp();
  doc["intH"] = gIntTempHum.get_humd();
  doc["extT"] = gExtTemperature.get_temp();
  doc["door"] = gApp_data.door_state;
  doc["fan"] = gApp_data.fan_relay;
  doc["heater"] = gApp_data.fan_relay;

  serializeJson(doc, gApp_data.http_data.JSON_data);

#ifdef DEBUG_SERIAL
  mySerial.print("JSON Data: ");
  mySerial.println(gApp_data.http_data.JSON_data);
#endif

  return gApp_data.http_data.JSON_data;
}


void do_check_sockets() {
  if (gApp_data.sys_time > gApp_data.socket_timer) {
    gApp_data.socket_timer = (gApp_data.sys_time + SOCKET_CHECK_TIME_SEC);
    /*
        The heater and fan have inverse logic, the fan is on when hot and off when cold
        the heater is opposite
      */
    if (gApp_data.ext_temperature >= FAN_ON_TEMPERATURE) {
      gApp_data.fan_relay = true;
    } else if (gApp_data.ext_temperature < FAN_OFF_TEMPERATURE) {
      gApp_data.fan_relay = false;
    }

    if (gApp_data.int_temperature >= HEATER_OFF_TEMPERATURE) {
      gApp_data.heater_relay = false;
    } else if (gApp_data.int_temperature < HEATER_ON_TEMPERATURE) {
      gApp_data.heater_relay = true;
    }

    //door state = light
    gApp_data.light_relay = (gApp_data.system_sleeping) ? false : true;

    // set the states

    gRelayManager.set_state_all(gApp_data.heater_relay,
                                gApp_data.light_relay,
                                gApp_data.fan_relay,
                                gApp_data.misc_relay);
  }
}



void check_door_state() {

  //if (gApp_data.ds_irq_triggered == true)
  //{
  //door sense triggered

  if (digitalRead(DOOR_SENSE_PIN) == DOOR_OPEN) {
#ifdef DEV_MODE
    //Serial.println("Door open");
#endif
    //detect whether the system has transisitioned
    if (gApp_data.system_sleeping == true) {
      system_woken_irq();
    }

    leds_lit = 0;
    //Make sure the light is turned on
    gApp_data.system_sleeping = false;
    gApp_data.door_state = true;
    gApp_data.doorclosed_timer = (DOOR_HOLD_OPEN_TIMER + gApp_data.sys_time);
    gApp_data.led_countdown = 11;
  } else {
#ifdef DEV_MODE
    //Serial.println("Door closed");
#endif

    gApp_data.door_state = false;
    if (gApp_data.doorclosed_timer < gApp_data.sys_time) {
      //system asleep
      gApp_data.doorclosed_timer = 0;
      gApp_data.system_sleeping = true;
    }
  }
#ifdef OVERRIDE_DOORSENSOR
  gApp_data.system_sleeping = false;
#endif
}
