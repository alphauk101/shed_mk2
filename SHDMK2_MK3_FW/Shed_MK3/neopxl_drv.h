#ifndef _NEOPXL_H__
#define _NEOPXL_H__
#include <stdint.h>
#include <sys/types.h>


class SHDPIXEL {
public:
  void init(void);
  void box_wipe(bool direction, uint16_t speed, uint32_t color);

private:
  void set_box_topbottm(bool top, bool bottom, uint32_t color);
};

#endif