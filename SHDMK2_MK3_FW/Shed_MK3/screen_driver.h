#ifndef __SCRN_DRV_H_
#define __SCRN_DRV_H_
#include <string>
#include "shdmk3_config.h"

#define TFT_DC_PIN 2
#define TFT_CS_PIN 7
#define RESET_PIN 3
#define BACKLIGHT_PIN 1

#define DEFAULT_ORIENTATION   3


#define BACKLIGHT_ON LOW
#define BACKLIGHT_OFF HIGH

typedef enum{
  not_connected,
  signal_poor,
  sginal_ok,
  signal_good,
}networkState_icon;

class SCRNDRV {
public:
  void init(void);
  void task(bool , SHED_APP *);//Allows exposure to the environment data
  //Sets the default startup screen, waits for updates from below fxn.
  void setStartUpMessage(void);
  //Updates the screen with the relevant status from the main
  void updateStartUpMessage(const std::string& IOexp_sts,          //IO expander status
                                  const std::string& intTemperature_sts,  //Internal temperature
                                  const std::string& intHumidity_sts,     //Internal humidity
                                  const std::string& extTemperature_sts,  //External temperature
                                  const std::string& led_driver);       //LED driver
  void setNetworkState(networkState_icon);
private:
  void doReset(void);
  void setDefaultScreenLayout(void);
  void setNetworkIcon(void);

  //Sets the given screen content layout
  void SCRNDRV::SCREENLAYOUT_internalTemp(SHED_APP *);
  void SCRNDRV::SCREENLAYOUT_internalHumd(SHED_APP *);
  void SCRNDRV::SCREENLAYOUT_ExternalTemp(SHED_APP *);
  void SCRNDRV::SCREENLAYOUT_DoorStatus(SHED_APP *);
};

#endif

/* fonts
FreeMono12pt7b.h		FreeSansBoldOblique12pt7b.h
FreeMono18pt7b.h		FreeSansBoldOblique18pt7b.h
FreeMono24pt7b.h		FreeSansBoldOblique24pt7b.h
FreeMono9pt7b.h			FreeSansBoldOblique9pt7b.h
FreeMonoBold12pt7b.h		FreeSansOblique12pt7b.h
FreeMonoBold18pt7b.h		FreeSansOblique18pt7b.h
FreeMonoBold24pt7b.h		FreeSansOblique24pt7b.h
FreeMonoBold9pt7b.h		FreeSansOblique9pt7b.h
FreeMonoBoldOblique12pt7b.h	FreeSerif12pt7b.h
FreeMonoBoldOblique18pt7b.h	FreeSerif18pt7b.h
FreeMonoBoldOblique24pt7b.h	FreeSerif24pt7b.h
FreeMonoBoldOblique9pt7b.h	FreeSerif9pt7b.h
FreeMonoOblique12pt7b.h		FreeSerifBold12pt7b.h
FreeMonoOblique18pt7b.h		FreeSerifBold18pt7b.h
FreeMonoOblique24pt7b.h		FreeSerifBold24pt7b.h
FreeMonoOblique9pt7b.h		FreeSerifBold9pt7b.h
FreeSans12pt7b.h		FreeSerifBoldItalic12pt7b.h
FreeSans18pt7b.h		FreeSerifBoldItalic18pt7b.h
FreeSans24pt7b.h		FreeSerifBoldItalic24pt7b.h
FreeSans9pt7b.h			FreeSerifBoldItalic9pt7b.h
FreeSansBold12pt7b.h		FreeSerifItalic12pt7b.h
FreeSansBold18pt7b.h		FreeSerifItalic18pt7b.h
FreeSansBold24pt7b.h		FreeSerifItalic24pt7b.h
FreeSansBold9pt7b.h		FreeSerifItalic9pt7b.h
*/