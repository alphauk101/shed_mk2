#include "int_temphum.h"
#include "Adafruit_SHT31.h"


Adafruit_SHT31 sht31 = Adafruit_SHT31();


bool INTTMPHUM::init() {
  return sht31.begin(SHT31_ADDRESS);
}

float INTTMPHUM::get_temperature() {
  return sht31.readTemperature();
}

float INTTMPHUM::get_humidity() {
  return sht31.readHumidity();
}