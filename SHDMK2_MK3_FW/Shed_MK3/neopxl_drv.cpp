#include "api/Common.h"

#include "neopxl_drv.h"
#include <Adafruit_NeoPixel.h>

#define MCR_SLEEP_GUARD(X) \
  if (g_led_data.g_sys_asleep) return X

#define TEMP_HIGHEST 40
#define TEMP_LOWEST 0

#define LED_PIN 6
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 42

#define DEFAULT_BRIGHTNESS 200

#define MCR_CLEAR_STRIP \
  strip.clear(); \
  strip.show()

typedef struct {
  bool g_sys_asleep;

  int last_temperature;
} LED_DATA;
static LED_DATA g_led_data;



constexpr int redToBlue[][3]{
  { 255, 0, 0 },
  { 248, 0, 7 },
  { 242, 0, 13 },
  { 235, 0, 20 },
  { 229, 0, 26 },
  { 222, 0, 33 },
  { 216, 0, 39 },
  { 209, 0, 46 },
  { 203, 0, 52 },
  { 196, 0, 59 },
  { 190, 0, 65 },
  { 183, 0, 72 },
  { 177, 0, 78 },
  { 170, 0, 85 },
  { 163, 0, 92 },
  { 157, 0, 98 },
  { 150, 0, 105 },
  { 144, 0, 111 },
  { 137, 0, 118 },
  { 131, 0, 124 },
  { 124, 0, 131 },
  { 118, 0, 137 },
  { 111, 0, 144 },
  { 105, 0, 150 },
  { 98, 0, 157 },
  { 92, 0, 163 },
  { 85, 0, 170 },
  { 78, 0, 177 },
  { 72, 0, 183 },
  { 65, 0, 190 },
  { 59, 0, 196 },
  { 52, 0, 203 },
  { 46, 0, 209 },
  { 39, 0, 216 },
  { 33, 0, 222 },
  { 26, 0, 229 },
  { 20, 0, 235 },
  { 13, 0, 242 },
  { 7, 0, 248 },
  { 0, 0, 255 }
};



/*These arrays represent the sides of the box as arrays*/
constexpr uint8_t boxbottom[]{
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
};

constexpr uint8_t boxtop[]{
  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42
};

constexpr uint8_t boxleft[]{
  22, 23, 24, 25, 26, 27, 28, 29, 30
};

constexpr uint8_t boxright[]{
  1, 2, 3, 4, 5, 6, 7, 8, 9
};

constexpr uint8_t boxwipe_side_index[][2]{

  { boxleft[8], boxright[0] },
  { boxleft[7], boxright[1] },
  { boxleft[6], boxright[2] },
  { boxleft[5], boxright[3] },
  { boxleft[4], boxright[4] },
  { boxleft[3], boxright[5] },
  { boxleft[2], boxright[6] },
  { boxleft[1], boxright[7] },
  { boxleft[0], boxright[8] },

};



Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


void SHDPIXEL::init() {
  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)  // Turn OFF all pixels ASAP
  strip.setBrightness(DEFAULT_BRIGHTNESS);
  MCR_CLEAR_STRIP;
  //this->set_box_right(strip.Color(255, 255, 255));
  //this->set_box_left(strip.Color(255, 255, 255));
  // this->set_box_topbottm(true, true, strip.Color(255, 255, 255));
  //this->set_all(strip.Color(255, 255, 255));
  int a = 1;
  while (a-- > 0) {
    this->box_wipe(true, 50, strip.Color(255, 0, 0));
    this->box_wipe(false, 50, strip.Color(255, 0, 0));
    delay(100);
    this->box_wipe(true, 50, strip.Color(0, 255, 0));
    this->box_wipe(false, 50, strip.Color(0, 255, 0));
    delay(100);
    this->box_wipe(true, 50, strip.Color(0, 0, 255));
    this->box_wipe(false, 50, strip.Color(0, 0, 255));
    delay(100);
  }

  g_led_data.g_sys_asleep = false;
  g_led_data.last_temperature = 0;
}

/*
Converts given temperature into a colour red hottest, blue coldest and shows
LED colours*/
#define FADE_SPEED      5
void SHDPIXEL::show_temperature_as_color(float temperature) {
  //if asleep then return here.
  MCR_SLEEP_GUARD();


  if (temperature > TEMP_HIGHEST) temperature = TEMP_HIGHEST;
  if (temperature < TEMP_LOWEST) temperature = TEMP_LOWEST;

  int t = ceil((int)temperature);  //round it
  int idx = TEMP_HIGHEST - t;

  if (g_led_data.last_temperature != idx) {
    g_led_data.last_temperature = idx;
    uint32_t colour = strip.Color(redToBlue[idx][0], redToBlue[idx][1], redToBlue[idx][2]);

    uint32_t tmp_col = strip.getPixelColor(0); 
    for (int a = DEFAULT_BRIGHTNESS; a > 0; a--) {
      strip.fill(tmp_col, 0, LED_COUNT);
      strip.setBrightness(a);
      strip.show();
      delay(FADE_SPEED);
    }
    for (int a = 0; a < DEFAULT_BRIGHTNESS; a++) {
      strip.fill(colour, 0, LED_COUNT);
      strip.setBrightness(a);
      strip.show();
      delay(FADE_SPEED);
    }
  }
}


void SHDPIXEL::task(bool asleep) {
  //Theres not much to do here however, if the system is asleep all leds should be off.
  g_led_data.g_sys_asleep = asleep;

  if (g_led_data.g_sys_asleep) {
    //Turn everything off.
    MCR_CLEAR_STRIP;
  }
}

/*
This function should allow a wipe from top to bottom or, vice versa (depending on dir bool)
at given speed.
*/
void SHDPIXEL::box_wipe(bool direction, uint16_t speed, uint32_t color) {

  MCR_SLEEP_GUARD();

  MCR_CLEAR_STRIP;
  //true = up, false = down;
  if (direction) {
    this->set_box_topbottm(false, true, color);
  } else {
    strip.fill(color, 0, LED_COUNT);
    strip.show();
    this->set_box_topbottm(true, false, 0);
    delay(speed);
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
    this->set_box_topbottm(true, false, color);
  } else {
    MCR_CLEAR_STRIP;
  }
}

void SHDPIXEL::set_all(uint32_t color) {
  MCR_SLEEP_GUARD();
  MCR_CLEAR_STRIP;
  strip.fill(color, 0, LED_COUNT);

  strip.show();
}

void SHDPIXEL::set_box_right(uint32_t color) {
  MCR_SLEEP_GUARD();
  for (int a = 0; a < sizeof(boxright); a++) {
    strip.setPixelColor(boxright[a], color);
  }

  strip.show();
}

void SHDPIXEL::set_box_left(uint32_t color) {

  for (int a = 0; a < sizeof(boxleft); a++) {
    strip.setPixelColor(boxleft[a], color);
  }

  strip.show();
}

/*
Sets the given colour to the top and or the bottom, if false is passed to 
either side then it is not changed
*/
void SHDPIXEL::set_box_topbottm(bool top, bool bottom, uint32_t color) {
  MCR_SLEEP_GUARD();
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
