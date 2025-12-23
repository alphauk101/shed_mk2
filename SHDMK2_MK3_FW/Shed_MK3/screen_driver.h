#ifndef __SCRN_DRV_H_
#define __SCRN_DRV_H_
#include <string>
#include "shdmk3_config.h"

#define TFT_DC_PIN 2
#define TFT_CS_PIN 7
#define RESET_PIN 3
#define BACKLIGHT_PIN 1

#define DEFAULT_ORIENTATION 3


#define BACKLIGHT_ON LOW
#define BACKLIGHT_OFF HIGH


typedef enum {
  not_connected,
  signal_bad,
  signal_awful,
  signal_ok,
  signal_good,
  signal_none,
} networkState_icon;


class SCRNDRV {
public:
  void init(void);
  void task(bool, SHED_APP*);  //Allows exposure to the environment data
  //Sets the default startup screen, waits for updates from below fxn.
  void setStartUpMessage(void);
  //Updates the screen with the relevant status from the main
  void updateStartUpMessage(const std::string& IOexp_sts,           //IO expander status
                            const std::string& intTemperature_sts,  //Internal temperature
                            const std::string& intHumidity_sts,     //Internal humidity
                            const std::string& extTemperature_sts,  //External temperature
                            const std::string& led_driver,
                            const std::string& wifi_status,
                            const std::string& rtc_Status);
  void setNetworkState(networkState_icon);
  void setShowNetConnect(void);
  void changeViewingScreen(void);
private:
  void doReset(void);
  //void setDefaultScreenLayout(const std::string&);
  void setDefaultScreenLayout(bool change_on_update, const std::string&);
  void setNetworkIcon(void);
  void fadeBackLight(bool);
  void changeSreen(void);
  //Sets the given screen content layout
  void SCREENLAYOUT_internalTemp(SHED_APP*);
  void SCREENLAYOUT_internalHumd(SHED_APP*);
  void SCREENLAYOUT_ExternalTemp(SHED_APP*);
  void SCREENLAYOUT_Information(SHED_APP*);
  void SCREENLAYOUT_countdown(SHED_APP*);
  void clearDynamicSection(uint16_t);
  void showPowerStates(void);
  String uptimeString(unsigned long);
  void resetScreenVars(void);
  void setPowerStates(bool light, bool fan, bool blower, bool misc);
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
