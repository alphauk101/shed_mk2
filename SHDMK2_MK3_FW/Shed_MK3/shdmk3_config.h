#ifndef __SHDMKR_CFG__
#define __SHDMKR_CFG__

#define DEBUG_ENVIRONMENTS 0

/*************************NETWORK *********************/
#define SECRET_SSID "fudgemesh"
#define SECRET_PASS "ancienthill347"
/*********************************************************/


/**********DEFAULT SCREEN MESSAGES********************/
#define STARTUP_MESSAGE "Shed mk3.... V0.1"
#define SCREEN_CHANGE_TIMEOUT             10000 //change between screen
/*****************************************************/

/*
              Relays states
              =============
To optimise current, this version will set the relays to there most 
used state in the none-energised position ie.
Fan is on most of the time so, relay will be NC vs. NO for the fan this 
way the power will be less when the fan is on.

This will apply to the others ie. lights will be on NO ie. not on regularly 
use the below defines to set the states so it is synchronised with the hardware.
*/

#define RELAY_ON false
#define RELAY_OFF true


#define RELAY_FAN_OFF RELAY_ON  //Spends most time in ON
#define RELAY_FAN_ON RELAY_OFF

#define RELAY_DRYER_OFF RELAY_OFF  //Spends most time in OFF
#define RELAY_DRYER_ON RELAY_ON    //Spends most time in OFF

#define REALY_LIGHT_OFF RELAY_ON  //Spends most time in OFF
#define REALY_LIGHT_ON RELAY_OFF  //Spends most time in OFF

#define RELAY_MISC_OFF RELAY_ON  //Spends most time in OFF
#define RELAY_MISC_ON RELAY_OFF  //Spends most time in OFF



/*
      APPLICATION TIME VALUES
*/

#define ENVIRONMENT_SAMPLE_TIME 5000
#define LED_DEFAULT_TIME 5000
#define SCREEN_UPDATE_TIME 500
#define NETWORK_TASK_CHECK  10000
#define RTC_TIMER_TASK      30000

typedef unsigned long UL_TIMER_t;

typedef struct {
  UL_TIMER_t environment_timer = ENVIRONMENT_SAMPLE_TIME;
  UL_TIMER_t led_timer = LED_DEFAULT_TIME;
  UL_TIMER_t screen_timer = SCREEN_UPDATE_TIME;
  UL_TIMER_t network_timer = NETWORK_TASK_CHECK;
  unsigned long rtc_timer = RTC_TIMER_TASK;
} APP_TIMERS;


typedef struct{
  bool lights;
  bool fan;
  bool blower;
  bool misc;
}POWER_STATES;


typedef struct {
  float internal_temp = 0;
  float internal_humidity = 0;
  float external_temp = 0;

  float external_temp_min = 20;
  float external_temp_max = 0;

  float internal_temp_min = 20;
  float internal_temp_max = 0;

  float internal_humidity_min = 20;
  float internal_humidity_max = 0;

} ENVIRON_READS;


typedef struct {
  ENVIRON_READS environmentals;

  POWER_STATES    power_states;

  bool system_asleep = false;  //If sleeping, all modules should be in lowest power mode.

  APP_TIMERS app_timers;
} SHED_APP;

#endif