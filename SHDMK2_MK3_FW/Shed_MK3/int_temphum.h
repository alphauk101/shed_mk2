#ifndef __EXTTMPHUM_H__
#define __EXTTMPHUM_H__

#define SHT31_ADDRESS 0x44

class INTTMPHUM {
public:
  bool init(void);
  float get_temperature(void);
  float get_humidity(void);
};


#endif