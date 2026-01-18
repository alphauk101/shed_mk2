#include "ext_temphum.h"
#include <DS18B20.h>
#include "shdmk3_config.h"

#define LOW_ALARM 20
#define HIGH_ALARM 25

DS18B20 ds(0);
#ifdef PROTO_HARDWARE
uint8_t address[] = { 0x28, 0x65, 0xCA, 0x5C, 0x6, 0x0, 0x0, 0xB2 };
#else
uint8_t address[] = { 0x28, 0x3E, 0x99, 0x5C, 0x6, 0x0, 0x0, 0xBF };
#endif
uint8_t selected;


bool EXTTEMP::init() {
  bool sens_ok = ds.select(address);

  if (sens_ok)
  {
    ds.setResolution(RES_12_BIT);
  }
  return sens_ok;
 // ds.setAlarms(LOW_ALARM, HIGH_ALARM);
}

float EXTTEMP::getTemp() {
  //ds.sendCommand(MATCH_ROM, CONVERT_T); 
  //delay(800); // Manually force the wait to bypass the library's buggy polling
  return ds.getTempC();
}