#ifndef _NEOPXL_H__
#define _NEOPXL_H__

class SHDPIXEL {
public:
  void init(void);
  void box_wipe(bool direction, uint16_t speed, uint32_t color);
};

#endif