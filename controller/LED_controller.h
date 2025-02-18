#ifndef __LED_CONTROLLER__
#define __LED_CONTROLLER__
#include "Arduino.h" 
#include "IO_pins.h"




class LED_Controller
{
  public:
    void led_init(void);
    void colorWipe(uint32_t color, int wait);
    void showSystemError(bool);
    void showSystemWorking(void);
    void colourSwell(uint8_t r,uint8_t g,uint8_t b, int wait);
    /*Sets the given LEDs for the sockets animates LEDs
    light, heater, fan, misc
    */
    void setShowSocketStatus(bool,bool,bool,bool);
  private:
    void theaterChase(uint32_t color, int wait);
    void rainbowFade2White(int wait, int rainbowLoops, int whiteLoops);
};

#endif