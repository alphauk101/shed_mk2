#ifndef _NETTMANAGERL_H__
#define _NETTMANAGERL_H__
#include <stdint.h>
#include <sys/types.h>
#include <WiFiNINA.h>
#include "screen_driver.h"

class NETMANAGER {
public:
  bool init(SCRNDRV *);
  bool task(void);
  long getRSSI(void);
  unsigned long  getTime(void); //return secs from unix epoch
  bool isConnected(void);
  void getIP(String &ip_ptr);
private:
  unsigned long  parseTimeFromPacket(void);
  //void sendNTPpacket(const char *address);
  void sendNTPpacket(IPAddress&);
};

#endif