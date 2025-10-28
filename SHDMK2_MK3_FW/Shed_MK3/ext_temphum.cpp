#include "ext_temphum.h"
#include <DS18B20.h>

#define LOW_ALARM 20
#define HIGH_ALARM 25

DS18B20 ds(0);
uint8_t address[] = { 0x28, 0x3E, 0x99, 0x5C, 0x6, 0x0, 0x0, 0xBF };
uint8_t selected;


void EXTTEMP::init() {
  selected = ds.select(address);
 // ds.setAlarms(LOW_ALARM, HIGH_ALARM);
}

float  EXTTEMP::getTemp() {
  return ds.getTempC();
}