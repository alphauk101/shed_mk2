#ifndef __FAN_CTRLR__
#define __FAN_CTRLR__
#include "shdmk3_config.h"

//#define _PWM_LOGLEVEL_       4


class fan_cntrllr {
public:
  enum FAN_SPEED{
    FAN_OFF,
    FAN_25,
    FAN_50,
    FAN_75,
    FAN_ON,
  };

  void setFanLevel(FAN_SPEED);
  void init(void);
  unsigned long getRPM(void);
private:
  void setup125kHz(int pin);
};




#endif
