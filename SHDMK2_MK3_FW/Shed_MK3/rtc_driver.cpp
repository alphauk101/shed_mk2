#include "RTClib.h"
#include "rtc_driver.h"
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib


//What time in the day the app resets the NTP request counter, only reset once a day.
#define NTP_RESET_HOUR        23
#define NTP_RESET_MINUTE      59




RTC_DS1307 rtc;

typedef struct {
  bool isSet;

  void (*callback_RTC)(int);

  DateTime  last_timestamp;
} RTC_DATA;
static RTC_DATA g_rtc_data;


bool RTCDRV::init(void (*callback)(int)) {
  g_rtc_data.isSet = true;
  g_rtc_data.callback_RTC = callback;
  return rtc.begin();
}

bool RTCDRV::getLatestTime(DateTime *ts_ptr)
{
  if(g_rtc_data.isSet)
  {
    *ts_ptr = g_rtc_data.last_timestamp;
    return true;
  }else{
    return false;
  }
}


bool RTCDRV::task(SHED_APP *shd_ptr) {
  this->showtime();
  if (rtc.isrunning()) {
    DateTime now = rtc.now();
    g_rtc_data.last_timestamp = now;
    //check NTP reset time
    if ((now.hour() == NTP_RESET_HOUR) && (now.minute() == NTP_RESET_MINUTE)) {

      //Also reset the is RTC isset flag to trigger an update
      g_rtc_data.isSet = false;

      //Its ok to call this multiple times during the minute
      g_rtc_data.callback_RTC(RTC_EVENT_RESET_NTP_ATTEMPTS);
    }

    return g_rtc_data.isSet;
  } else {
    return false;
  }
}

void RTCDRV::setTIme(unsigned long epoch_secs) {
  DateTime dt = DateTime(epoch_secs);
  rtc.adjust(dt);
  g_rtc_data.isSet = true;
}

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
void RTCDRV::showtime() {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.print(" since midnight 1/1/1970 = ");
  Serial.print(now.unixtime());
  Serial.print("s = ");
  Serial.print(now.unixtime() / 86400L);
  Serial.println("d");

  // calculate a date which is 7 days, 12 hours, 30 minutes, and 6 seconds into the future
  DateTime future(now + TimeSpan(7, 12, 30, 6));

  Serial.print(" now + 7d + 12h + 30m + 6s: ");
  Serial.print(future.year(), DEC);
  Serial.print('/');
  Serial.print(future.month(), DEC);
  Serial.print('/');
  Serial.print(future.day(), DEC);
  Serial.print(' ');
  Serial.print(future.hour(), DEC);
  Serial.print(':');
  Serial.print(future.minute(), DEC);
  Serial.print(':');
  Serial.print(future.second(), DEC);
  Serial.println();

  Serial.println();
}


/*
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {
  Serial.begin(57600);

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
}

void loop () {
    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");

    // calculate a date which is 7 days, 12 hours, 30 minutes, and 6 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));

    Serial.print(" now + 7d + 12h + 30m + 6s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();

    Serial.println();
    delay(3000);
}
*/