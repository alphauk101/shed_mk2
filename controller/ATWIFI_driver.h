#ifndef __ATWIFI__
#define __ATWIFI__
#include <Arduino.h>

#define EVNT_SETUP_OK 1
#define EVNT_SETUP_FAIL 2
#define EVNT_CONNECTION_OK 3
#define EVNT_CONNECTION_FAILED 4
#define EVNT_OP_ABORTED_ERROR 5

struct WifiStatus {
  bool isConnected;
  String ssid;
  String bssid;
  int channel;
  int rssi;
};


class atwifi_driver {
public:
  void init(void (*)(int));
  void task(void);
  void serialEventHandler(void);
  bool startGetConnectionStatus(void);
  bool startConnectToWifi(void);
  bool startHTTPPost(String* json_ptr, unsigned int len);
private:
  void init_modem(void);
  void getATOK(void);
  void send_AT_command(const char*);
  void send_AT_command(const String&);
  void flush_rx_buffer(void);
  void printWifiStatus(const WifiStatus&);
  WifiStatus parseWifiStatus(const String&);
  void kill_operation(void);
  void createHTTPPostCommand(unsigned int);
  void sendPostBody(void);
  bool blockWaitForCommand(const char* cmd_ptr);
protected:
};

#endif