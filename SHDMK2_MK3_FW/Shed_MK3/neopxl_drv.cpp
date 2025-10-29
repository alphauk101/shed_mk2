#include <stdint.h>
#include <sys/types.h>
#include "neopxl_drv.h"
#include <Adafruit_NeoPixel.h>


#define LED_PIN                 0
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT               60

#define DEFAULT_BRIGHTNESS      200

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


void SHDPIXEL::init()
{
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(DEFAULT_BRIGHTNESS);
}


/*
This function should allow a wipe from top to bottom or, vice versa (depending on dir bool)
at given speed.
*/
void SHDPIXEL::box_wipe(bool direction, uint16_t speed, uint32_t color)
{

}