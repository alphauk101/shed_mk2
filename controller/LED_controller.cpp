#include "LED_controller.h"
#include <Adafruit_NeoPixel.h>

#define NUMBER_LEDS 12


#define LIGHT_LED 2
#define HEATER_LED 1
#define FAN_LED 11
#define MISC_LED 10

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

void LED_Controller::led_init() {
  led_ring.begin();  // Initialize NeoPixel strip object (REQUIRED)
  led_ring.show();   // Initialize all pixels to 'off'
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void LED_Controller::colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < led_ring.numPixels(); i++) {  // For each pixel in strip...
    led_ring.setPixelColor(i, color);               //  Set pixel's color (in RAM)
    led_ring.show();                                //  Update strip to match
    delay(wait);                                    //  Pause for a moment
  }
}


void LED_Controller::colourSwell(uint8_t r, uint8_t g, uint8_t b, int wait) {


  for (int a = 0; a < 10; a++) {
    led_ring.clear();
    for (int i = 0; i < led_ring.numPixels(); i++) {       // For each pixel in strip...
      led_ring.setPixelColor(i, led_ring.Color(r, g, b));  //  Set pixel's color (in RAM)                              //  Update strip to match                                 //  Pause for a moment
    }
    //   Set all pixels in RAM to 0 (off)
    led_ring.setBrightness(a * 25);
    led_ring.show();  // Update strip with new contents
    delay(wait);      // Pause for a moment
  }
  for (int b = 0; b < 10; b++) {
    led_ring.clear();
    for (int i = 0; i < led_ring.numPixels(); i++) {       // For each pixel in strip...
      led_ring.setPixelColor(i, led_ring.Color(r, g, b));  //  Set pixel's color (in RAM)                              //  Update strip to match                                 //  Pause for a moment
    }
    //   Set all pixels in RAM to 0 (off)
    led_ring.setBrightness(255 - (b * 25));
    led_ring.show();  // Update strip with new contents
    delay(wait);      // Pause for a moment
  }

  led_ring.clear();
  led_ring.show();
}



void LED_Controller::showSystemError(bool forever) {
  do {
    this->theaterChase(led_ring.Color(255, 0, 0), 30);
  } while (forever);
}

void LED_Controller::showSystemWorking() {
  this->rainbowFade2White(3, 5, 1);
}


/*Sets the given LEDs for the sockets animates LEDs
    light, heater, fan, misc
    */
void LED_Controller::setShowSocketStatus(bool light, bool heater, bool fan, bool misc) {

  for (int a = 0; a < 3; a++) {    // Repeat 10 times...
    for (int b = 0; b < 3; b++) {  //  'b' counts from 0 to 2...
      led_ring.clear();            //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...

      for (int i = 0; i < led_ring.numPixels(); i++) {
        led_ring.setPixelColor(i, led_ring.Color(0, 50, 0));  // Set pixel 'c' to value 'color'
      }

      for (int c = b; c < led_ring.numPixels(); c += 3) {
        led_ring.setPixelColor(c, led_ring.Color(0, 255, 0));  // Set pixel 'c' to value 'color'
      }

      if (light)
        led_ring.setPixelColor(LIGHT_LED, led_ring.Color(255, 0, 0));

      if (heater)
        led_ring.setPixelColor(HEATER_LED, led_ring.Color(255, 0, 0));

      if (fan)
        led_ring.setPixelColor(FAN_LED, led_ring.Color(255, 0, 0));

      if (misc)
        led_ring.setPixelColor(MISC_LED, led_ring.Color(255, 0, 0));


      led_ring.show();  // Update strip with new contents
      delay(50);        // Pause for a moment
    }
  }
}




// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void LED_Controller::theaterChase(uint32_t color, int wait) {
  for (int a = 0; a < 10; a++) {   // Repeat 10 times...
    for (int b = 0; b < 3; b++) {  //  'b' counts from 0 to 2...
      led_ring.clear();            //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for (int c = b; c < led_ring.numPixels(); c += 3) {
        led_ring.setPixelColor(c, color);  // Set pixel 'c' to value 'color'
      }
      led_ring.show();  // Update strip with new contents
      delay(wait);      // Pause for a moment
    }
  }
}



void LED_Controller::rainbowFade2White(int wait, int rainbowLoops, int whiteLoops) {
  int fadeVal = 0, fadeMax = 100;

  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  for (uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops * 65536;
       firstPixelHue += 256) {

    for (int i = 0; i < led_ring.numPixels(); i++) {  // For each pixel in strip...

      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      uint32_t pixelHue = firstPixelHue + (i * 65536L / led_ring.numPixels());

      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      led_ring.setPixelColor(i, led_ring.gamma32(led_ring.ColorHSV(pixelHue, 255,
                                                                   255 * fadeVal / fadeMax)));
    }

    led_ring.show();
    delay(wait);

    if (firstPixelHue < 65536) {                                 // First loop,
      if (fadeVal < fadeMax) fadeVal++;                          // fade in
    } else if (firstPixelHue >= ((rainbowLoops - 1) * 65536)) {  // Last loop,
      if (fadeVal > 0) fadeVal--;                                // fade out
    } else {
      fadeVal = fadeMax;  // Interim loop, make sure fade is at max
    }
  }

  for (int k = 0; k < whiteLoops; k++) {
    for (int j = 0; j < 256; j++) {  // Ramp up 0 to 255
      // Fill entire strip with white at gamma-corrected brightness level 'j':
      led_ring.fill(led_ring.Color(0, 0, 0, led_ring.gamma8(j)));
      led_ring.show();
    }
    delay(1000);                      // Pause 1 second
    for (int j = 255; j >= 0; j--) {  // Ramp down 255 to 0
      led_ring.fill(led_ring.Color(0, 0, 0, led_ring.gamma8(j)));
      led_ring.show();
    }
  }

  delay(500);  // Pause 1/2 second
}
