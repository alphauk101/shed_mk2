#ifndef __RELAY_CONTROLLER__
#define __RELAY_CONTROLLER__

#include "IO_pins.h"

typedef bool RELAY_STATE_t;


class relay_manager {
public:
  void relay_init(void);
  void set_heater_state(bool state);
  void set_light_state(bool state);
  void set_fan_state(bool state);
  void set_misc_state(bool state);
  void set_state_all(bool heater, bool light, bool fan, bool misc);
private:
  void setall_state(RELAY_STATE_t);
};

#endif