#ifndef __EXT_TEMPHUM__
#define __EXT_TEMPHUM__
#include "Arduino.h" 




class external_temp
{
  public:
    bool init_temp(void);
    float get_temp(void);
  private:


};

#endif