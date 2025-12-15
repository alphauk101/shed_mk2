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
  /*Triggers a post operation to the server*/
  void do_metrics_post(SHED_APP*);
private:
  unsigned long  parseTimeFromPacket(void);
  //void sendNTPpacket(const char *address);
  void sendNTPpacket(IPAddress&);
  bool start_client_connection(void);
  void get_server_response(void);
  void do_client_task(void);
  //anything with CT is a client task operation and shouldnt be called outside the SM
  void CT_start_request(void);
  void cancel_client_task(void);
};

#endif