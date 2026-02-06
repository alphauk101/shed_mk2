#include "api/Common.h"
#include "neopxl_drv.h"
#include <Adafruit_NeoPixel.h>


#ifdef LED_TIMER_FRAMING
#include <Adafruit_ZeroTimer.h>

//float freq = 1000.0;  // 1 KHz
float freq = 50;

// timer tester
Adafruit_ZeroTimer zerotimer = Adafruit_ZeroTimer(3);
#endif

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
          strip.setBrightness(DEFAULT_BRIGHTNESS); \
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
  //10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
};

constexpr uint8_t boxtop[]{
  31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41
};

constexpr uint8_t boxleft[]{
  21, 22, 23, 24, 25, 26, 27, 28, 29, 30
};

constexpr uint8_t boxright[]{
  //1, 2, 3, 4, 5, 6, 7, 8, 9
  0, 1, 2, 3, 4, 5, 6, 7, 8
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

#ifdef LED_TIMER_FRAMING
// the timer callback
volatile bool togglepin = false;

void TC3_Handler() {
  Adafruit_ZeroTimer::timerHandler(3);
}


void TimerCallback0(void) {
}

void setupHWTimer(void) {
  // Set up the flexible divider/compare
  uint16_t divider = 1;
  uint16_t compare = 0;
  tc_clock_prescaler prescaler = TC_CLOCK_PRESCALER_DIV1;

  if ((freq < 24000000) && (freq > 800)) {
    divider = 1;
    prescaler = TC_CLOCK_PRESCALER_DIV1;
    compare = 48000000 / freq;
  } else if (freq > 400) {
    divider = 2;
    prescaler = TC_CLOCK_PRESCALER_DIV2;
    compare = (48000000 / 2) / freq;
  } else if (freq > 200) {
    divider = 4;
    prescaler = TC_CLOCK_PRESCALER_DIV4;
    compare = (48000000 / 4) / freq;
  } else if (freq > 100) {
    divider = 8;
    prescaler = TC_CLOCK_PRESCALER_DIV8;
    compare = (48000000 / 8) / freq;
  } else if (freq > 50) {
    divider = 16;
    prescaler = TC_CLOCK_PRESCALER_DIV16;
    compare = (48000000 / 16) / freq;
  } else if (freq > 12) {
    divider = 64;
    prescaler = TC_CLOCK_PRESCALER_DIV64;
    compare = (48000000 / 64) / freq;
  } else if (freq > 3) {
    divider = 256;
    prescaler = TC_CLOCK_PRESCALER_DIV256;
    compare = (48000000 / 256) / freq;
  } else if (freq >= 0.75) {
    divider = 1024;
    prescaler = TC_CLOCK_PRESCALER_DIV1024;
    compare = (48000000 / 1024) / freq;
  } else {
    //Serial.println("Invalid frequency");
    //while (1) delay(10);
  }

  zerotimer.enable(false);
  zerotimer.configure(prescaler,                    // prescaler
                      TC_COUNTER_SIZE_16BIT,        // bit width of timer/counter
                      TC_WAVE_GENERATION_MATCH_PWM  // frequency or PWM mode
  );

  zerotimer.setCompare(0, compare);
  zerotimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, TimerCallback0);
  zerotimer.enable(true);
}

#endif


void SHDPIXEL::init() {
  //Used hardware timer to update LED patterns
#ifdef LED_TIMER_FRAMING
  setupHWTimer();
#endif

  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)  // Turn OFF all pixels ASAP
  g_led_data.g_sys_asleep = false;
  g_led_data.last_temperature = 0;
  strip.setBrightness(DEFAULT_BRIGHTNESS);
  MCR_CLEAR_STRIP;
  //this->set_box_right(strip.Color(255, 255, 255));
  //this->set_box_left(strip.Color(255, 255, 255));
  // this->set_box_topbottm(true, true, strip.Color(255, 255, 255));
  //this->set_all(strip.Color(255, 255, 255));


#define NEW_STARTUP
#ifdef NEW_STARTUP
  //this->side_wipe(5, strip.Color(255, 0, 0), true);
  this->rainbow(10); 
#else
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
#endif
}

uint32_t SHDPIXEL::convert_color_to_32bit(int col) {
  uint32_t out_colour = 0;
  switch (col) {
    case PXL_RED:
      out_colour = strip.Color(255, 0, 0);
      break;
    case PXL_GREEN:
      out_colour = strip.Color(0, 255, 0);
      break;
    case PXL_BLUE:
      out_colour = strip.Color(0, 0, 255);
      break;
  }
  return out_colour;
}

void SHDPIXEL::show_action_swipe(int color) {
  uint32_t colour = this->convert_color_to_32bit(color);

  this->box_wipe(true, 50, colour);
  this->box_wipe(false, 50, colour);
}

void SHDPIXEL::show_low_awake_colour()
{
  //This is a wake only task so check its not showing when asleep.
  MCR_SLEEP_GUARD();

  strip.fill(DEFAULT_WAKE_COLOUR, 0, LED_COUNT);
  strip.setBrightness(DEFAULT_BRIGHTNESS);
  strip.show();
}

uint32_t sleep_colours[5] = {
  strip.Color(0, 0, 255),
  strip.Color(0, 255, 0),
  strip.Color(255, 0, 0),
  strip.Color(255, 0, 255),
  strip.Color(255, 255, 255),
};
uint8_t s_counter = 0;
#define SLEEP_LIGHT_SPEED   5
void SHDPIXEL::show_lights_off_wake()
{
  MCR_SLEEP_GUARD();

  strip.fill(sleep_colours[s_counter], 0, LED_COUNT);
  this->do_brightness_swipe(true, SLEEP_LIGHT_SPEED);
  this->do_brightness_swipe(false, SLEEP_LIGHT_SPEED);

  if(s_counter == sizeof(sleep_colours)){
    s_counter = 0;
  }else{
    s_counter++;
  }
}


/*
Converts given temperature into a colour red hottest, blue coldest and shows
LED colours*/
#define FADE_SPEED 5
void SHDPIXEL::show_temperature_as_color(float temperature) {
  //if asleep then return here.
  //This is a wake only task so check its not showing when asleep.
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
  if (g_led_data.g_sys_asleep != asleep) {
    g_led_data.g_sys_asleep = asleep;
    if (g_led_data.g_sys_asleep) {
      //Turn everything off.
      MCR_CLEAR_STRIP;
      g_led_data.last_temperature = 0;
    }
  }
}


void SHDPIXEL::side_wipe(const uint16_t speed, const uint32_t color, const bool ignore_sleep) 
{
  MCR_CLEAR_STRIP;
  this->set_box_topbottm(false, true, color);
  strip.setBrightness(DEFAULT_BRIGHTNESS);
  strip.show();  // Ensure the hardware actually updates
  //this->do_brightness_swipe(true, speed);
  //this->do_brightness_swipe(false, speed);
  delay(200);


  MCR_CLEAR_STRIP;  // This clears pixel data
  this->set_box_topbottm(false, true, color);
  this->set_box_right(color);
  this->set_box_left(color);
  strip.setBrightness(DEFAULT_BRIGHTNESS);
  strip.show();  // Ensure the hardware actually updates
  // Brightness is already 0 from the end of previous do_brightness_swipe(false)
  //this->do_brightness_swipe(true, speed);
  //this->do_brightness_swipe(false, speed);
  delay(200);


  MCR_CLEAR_STRIP;
  this->set_box_topbottm(true, true, color);
  this->set_box_right(color);
  this->set_box_left(color);
  strip.setBrightness(DEFAULT_BRIGHTNESS);
  strip.show();  // Ensure the hardware actually updates

}


void SHDPIXEL::do_brightness_swipe(bool swipeUP, int speed) {
  // Determine direction: 1 for up, -1 for down
  int8_t step = swipeUP ? 1 : -1;
  int16_t target = swipeUP ? 255 : 0;
  int16_t brightness = swipeUP ? 0 : 255;

  // Continue until we reach or pass the target
  while (brightness != target) {
    brightness += step;
    strip.setBrightness(brightness);
    strip.show();  // Ensure the hardware actually updates
    delay(speed);
  }
}


/*
This function should allow a wipe from top to bottom or, vice versa (depending on dir bool)
at given speed.
*/
void SHDPIXEL::box_wipe(bool direction, uint16_t speed, uint32_t color) {

  //MCR_SLEEP_GUARD();

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

  if (direction) 
  {
    this->set_box_topbottm(true, false, color);
  } else {
    MCR_CLEAR_STRIP;
  }
}

void SHDPIXEL::set_all(uint32_t color) {
  //MCR_SLEEP_GUARD();
  MCR_CLEAR_STRIP;
  strip.fill(color, 0, LED_COUNT);
  strip.show();
}

void SHDPIXEL::set_box_right(uint32_t color) 
{
  for (int a = 0; a < sizeof(boxright); a++) 
  {
    strip.setPixelColor(boxright[a], color);
  }
}

void SHDPIXEL::set_box_left(uint32_t color) 
{
  for (int a = 0; a < sizeof(boxleft); a++) 
  {
    strip.setPixelColor(boxleft[a], color);
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void SHDPIXEL::rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}



/*
Sets the given colour to the top and or the bottom, if false is passed to 
either side then it is not changed
*/
void SHDPIXEL::set_box_topbottm(bool top, bool bottom, uint32_t color) {
  //MCR_SLEEP_GUARD();
  if (top) {
    for (int a = 0; a < sizeof(boxtop); a++) {
      strip.setPixelColor(boxtop[a], color);
    }
  }

  if (bottom) {
    for (int b = 0; b < sizeof(boxbottom); b++) {
      strip.setPixelColor(boxbottom[b], color);
    }
  }
  //strip.show();
}
