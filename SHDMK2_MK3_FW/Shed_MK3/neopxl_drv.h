#ifndef _NEOPXL_H__
#define _NEOPXL_H__
#include <stdint.h>
#include <sys/types.h>

#define PXL_RED     1
#define PXL_BLUE    2
#define PXL_GREEN   3

class SHDPIXEL {
public:
  void init(void);
  void box_wipe(bool direction, uint16_t speed, uint32_t color);
  void task(bool);
  void show_temperature_as_color(float temperature);
  void show_action_swipe(int);
private:
  void set_box_topbottm(bool top, bool bottom, uint32_t color);
  void set_box_right(uint32_t color);
  void set_box_left(uint32_t color);
  void set_all(uint32_t color);
  uint32_t convert_color_to_32bit(int);
};

#endif