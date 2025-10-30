
#include "neopxl_drv.h"
#include <Adafruit_NeoPixel.h>


#define MCR_CLEAR_STRIP \
  strip.clear(); \
  strip.show()

/*These arrays represent the sides of the box as arrays*/
constexpr uint8_t boxbottom[]{
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

constexpr uint8_t boxtop[]{
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40
};

constexpr uint8_t boxleft[]{
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39
};

constexpr uint8_t boxright[]{
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39
};

constexpr uint8_t boxwipe_side_index[][2]{

  { boxleft[9], boxright[1] },
  { boxleft[8], boxright[2] },
  { boxleft[7], boxright[3] },
  { boxleft[6], boxright[4] },
  { boxleft[5], boxright[5] },
  { boxleft[4], boxright[6] },
  { boxleft[3], boxright[7] },
  { boxleft[2], boxright[8] },
  { boxleft[1], boxright[9] },

};


#define LED_PIN 0
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 60

#define DEFAULT_BRIGHTNESS 200

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


void SHDPIXEL::init() {
  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();   // Turn OFF all pixels ASAP
  strip.setBrightness(DEFAULT_BRIGHTNESS);
}


/*
This function should allow a wipe from top to bottom or, vice versa (depending on dir bool)
at given speed.
*/
void SHDPIXEL::box_wipe(bool direction, uint16_t speed, uint32_t color) {

  MCR_CLEAR_STRIP;

  //true = up, false = down;
  if (direction) {
    set_box_topbottm(false, true, color);
  } else {
    strip.fill(color, 0, LED_COUNT);
    strip.show();
    delay(speed);
    set_box_topbottm(false, true, color);
  }

  delay(speed);

  //Do the side climb
  size_t side_count = sizeof(boxleft);
  for (int a = 0; a < side_count; a++) {
    if (direction)  //up
    {
      strip.setPixelColor(boxleft[a], color);
      strip.setPixelColor(boxright[side_count - a], color);
    } else {  //down

      strip.setPixelColor(boxright[a], 0);
      strip.setPixelColor(boxleft[side_count - a], 0);
    }
    strip.show();
    delay(speed);
  }

  if (direction) {
    set_box_topbottm(false, true, color);
  } else {
    set_box_topbottm(true, false, color);
  }
}


/*
Sets the given colour to the top and or the bottom, if false is passed to 
either side then it is not changed
*/
void SHDPIXEL::set_box_topbottm(bool top, bool bottom, uint32_t color) {

  if (top) {
    for (int a = 0; a < sizeof(boxtop); a++) {
      strip.setPixelColor(boxtop[a], color);
    }
  }

  if (bottom) {
    for (int a = 0; a < sizeof(boxtop); a++) {
      strip.setPixelColor(boxbottom[a], color);
    }
  }

  strip.show();
}
