#include "ext_temp_controller.h"
#include <DS18B20.h>
#include "IO_pins.h"

/*Run ds18b20 multi example to get address*/
//40 57 159 92 6 0 0 166
DS18B20 ds(EXT_TEMPSENSOR_PIN);
uint8_t address[] = {40, 57, 159, 92, 6, 0, 0, 166};



bool external_temp::init_temp()
{
  bool selected = ds.select(address);

  if (selected) {
    return true;
  } else {
    return false;
  }
}


float external_temp::get_temp()
{
  //Very simple
  return ds.getTempC();  
}
