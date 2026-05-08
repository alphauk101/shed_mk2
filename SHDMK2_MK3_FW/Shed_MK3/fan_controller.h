#ifndef __FAN_CTRLR__
#define __FAN_CTRLR__
#include "shdmk3_config.h"

//#define _PWM_LOGLEVEL_       4
#define FAN_HUMIDITY_MAX 85  //IF humidity if higher than this then the fan will be on full
#define FAN_HUMIDITY_MID 65
#define FAN_HUMIDITY_LOW 50
#define FAN_IN_OUT_TEMP_DIFF 15  //If the inside temperature difference ve. out door exceeds this.

#define MAX_DEW_THRESHOLD     3 //If the int dew gets within 3 degrees of threshold then blast it!

//Night mode as the fan can be very loud!
#define FAN_LOW_NIGHT_MODE_STARTHOUR      21
#define FAN_LOW_NIGHT_MODE_ENDHOUR        8


class fan_cntrllr {
public:
  enum FAN_SPEED{
    FAN_INIT, //used to show state not set (start up)
    FAN_OFF,
    FAN_25,
    FAN_50,
    FAN_75,
    FAN_ON,
  };

  void setFanLevel(FAN_SPEED);
  void init(void);
  unsigned long getRPM(void);
  /*
  Must be called regularly, will use the shed data to 
  calculate the required fan speed based on the current
  environmental parameters.
  */
  void task(SHED_APP*);
private:
  void setup125kHz(int pin);
};




#endif
