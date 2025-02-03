#include "LED_controller.h"
#include <Adafruit_NeoPixel.h>

// Declare our NeoPixel strip object:
Adafruit_NeoPixel led_ring(NUMBER_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

void LED_Controller::led_init()
{
  led_ring.begin(); // Initialize NeoPixel strip object (REQUIRED)
  led_ring.show();  // Initialize all pixels to 'off'
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void LED_Controller::colorWipe(uint32_t color, int wait) {
  for(int i=0; i<led_ring.numPixels(); i++) { // For each pixel in strip...
    led_ring.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    led_ring.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void LED_Controller::theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      led_ring.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<led_ring.numPixels(); c += 3) {
        led_ring.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      led_ring.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

