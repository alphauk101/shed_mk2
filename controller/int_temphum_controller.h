#ifndef __INT_TEMPHUM__
#define __INT_TEMPHUM__
#include "Arduino.h" 
#include "IO_pins.h"



class internal_tmphum
{
  public:
    bool init_temphum(void);

    float get_temp();
    float get_humd();
  private:


};

#endif