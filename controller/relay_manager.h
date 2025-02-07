#ifndef __RELAY_CONTROLLER__
#define __RELAY_CONTROLLER__

#include "IO_pins.h"

typedef bool      RELAY_STATE_t;


class relay_manager
{
  public:
    void relay_init(void);
  private:
    void setall_state(RELAY_STATE_t);
};

#endif