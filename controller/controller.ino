
#include "LED_controller.h"          //LEDs (Neo pixel)
#include "int_temphum_controller.h"  //Internal AHT25
#include "ext_temp_controller.h"     //External DS18b20 temperature probe
#include "relay_manager.h"
#include "disp_manager.h"
#include "arduino-timer.h"
#include "config.h"

#define DEV_MODE  //If en, turns off pointless animas etc.

#define SOCKET_CHECK_TIME_SEC 2

#define DOOR_CLOSED LOW
#define DOOR_OPEN HIGH

typedef struct {
  //Relay states
  bool heater_relay;
  bool light_relay;
  bool misc_relay;
  bool fan_relay;

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
} APP_LOGIC_DATA;

static APP_LOGIC_DATA gApp_data;

//Pixel LED controller.
LED_Controller gLEDcontroller;
internal_tmphum gIntTempHum;
external_temp gExtTemperature;
relay_manager gRelayManager;
disp_manager gDispManager;


auto timer_1hz = timer_create_default();  // create a timer with default settings


bool sys_tick_irq(void *) {
  gApp_data.sys_time++;
  return true;  // repeat? true
}

void door_sense_irq() {
  //Serial.println("PING!");
  //gApp_data.ds_irq_triggered = true;
}

void setup() {
  bool int_temp_ok = true, ext_temp_ok = true;

  //setup IRQ for door sense
  pinMode(DOOR_SENSE_PIN, INPUT);
  //attachInterrupt(digitalPinToInterrupt(DOOR_SENSE_PIN), door_sense_irq, CHANGE);

  gApp_data.heater_relay = false;
  gApp_data.light_relay = false;
  gApp_data.misc_relay = false;
  gApp_data.fan_relay = false;

  gApp_data.ds_irq_triggered = false;

  //Door sensor state
  gApp_data.system_sleeping = false;

  gApp_data.sys_time = 0;
  gApp_data.environ_timer = 0;
  gApp_data.socket_timer = 0;

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


  if ((!int_temp_ok) || (!ext_temp_ok)) {
#ifdef DEV_MODE
    Serial.println(int_temp_ok);
    Serial.println(ext_temp_ok);
    //Alert user, system has failed to init!
    Serial.println("--------- SETUP FAILED ---------");
    //Show setup bad on leds
    gLEDcontroller.showSystemError(true);
#endif
  } else {
    //Show setup ok on leds
    gLEDcontroller.showSystemWorking();
  }

  //Start the sys timer
  timer_1hz.every(1000, sys_tick_irq);
}



void loop() {

  //sample temps and hum
  do_environment_sampling();

  //Set sockets states
  do_check_sockets();

  //All modules should be informed of the sleep state
  gDispManager.system_sleeping(gApp_data.system_sleeping);
  timer_1hz.tick();  // tick the timer

  //Door sense logic
  check_door_state();


  
  static uint64_t dev_timer;
  if (gApp_data.sys_time != dev_timer) {
    Serial.println("=======SYS TIMER======");
    dev_timer = gApp_data.sys_time;
    Serial.print((int)gApp_data.sys_time);
    Serial.println(" Secs");
  }
}


/*Samples once a second*/
void do_environment_sampling() {
  if (gApp_data.environ_timer < gApp_data.sys_time) {
    //DO temp and hum readings
    gApp_data.int_temperature = gIntTempHum.get_temp();
    gApp_data.int_humid = gIntTempHum.get_humd();
    gApp_data.ext_temperature = gExtTemperature.get_temp();

    gDispManager.disp_environments(gApp_data.int_temperature,
                                   gApp_data.int_humid,
                                   gApp_data.ext_temperature);

    gApp_data.environ_timer = gApp_data.sys_time;

#ifdef DEV_MODE
    Serial.println("=======INTERNAL======");
    Serial.print(gApp_data.int_temperature);
    Serial.println(" C");
    Serial.print(gApp_data.int_humid);
    Serial.println(" %");

    Serial.println("=======EXTERNAL======");
    Serial.print(gApp_data.ext_temperature);
    Serial.println(" C");
#endif
  }
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
    gApp_data.light_relay = (gApp_data.system_sleeping)?false:true;

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
    //Make sure the light is turned on
    gApp_data.system_sleeping = false;
    gApp_data.doorclosed_timer = (DOOR_HOLD_OPEN_TIMER + gApp_data.sys_time);
  } else {
#ifdef DEV_MODE
    //Serial.println("Door closed");
#endif
    if (gApp_data.doorclosed_timer < gApp_data.sys_time) {
      //system asleep
      gApp_data.system_sleeping = true;
    }
  }
  //}
}
