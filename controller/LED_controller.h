#ifndef __LED_CONTROLLER__
#define __LED_CONTROLLER__
#include "Arduino.h" 
#include "IO_pins.h"

#define NUMBER_LEDS   12


class LED_Controller
{
  public:
    void led_init(void);
    void colorWipe(uint32_t color, int wait);
    void showSystemError(bool);
    void showSystemWorking(void);
  private:
    void theaterChase(uint32_t color, int wait);
    void rainbowFade2White(int wait, int rainbowLoops, int whiteLoops);
};

#endif