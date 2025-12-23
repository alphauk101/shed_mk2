#ifndef __RTC_DRV_H_
#define __RTC_DRV_H_
#include <string.h>
#include "shdmk3_config.h"


#define RTC_EVENT_RESET_NTP_ATTEMPTS    100


class RTCDRV {
public:
  bool init(void (*callback)(int));
  bool task(SHED_APP*);  //returns true or false dependant on if the closk is set ie. known to be accurate.
  void setTIme(unsigned long);
  bool getLatestTime(DateTime *); //if true time ok, if false then time is not valid.
private:
  void showtime(void);
};


#endif
