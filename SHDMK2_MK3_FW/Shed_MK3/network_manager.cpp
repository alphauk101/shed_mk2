#include "network_manager.h"
#include "shdmk3_config.h"
#include <SPI.h>
#include <WiFiUdp.h>
#include <ArduinoHttpClient.h>

#ifndef PROTO_HARDWARE
//Default for release hardware
#define ENABLE_NTP  //debugging only, prevents repeative calls to NTP
#define ENABLE_METRIC_POST
#else
//Default for prototype
//#define ENABLE_NTP 
//#define ENABLE_METRIC_POST
#endif


char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;  // the WiFi radio's status

typedef enum {
  idle,
  started,
  getServerResponse,
} clientTask;
static clientTask client_task = idle;
static String postBody;

//char server[] = "www.google.com";  // name address for Google (using DNS)
char serverAddress[] = "192.168.5.193";
int port = 3000;
// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient WIFIclient;
HttpClient httpClient = HttpClient(WIFIclient, serverAddress, port);


//const char timeServer[] = "time.nist.gov";  // time.nist.gov NTP server
IPAddress timeServer(162, 159, 200, 123);  // pool.ntp.org NTP server
unsigned int localPort = 2390;             // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;            // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE];  //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;


bool NETMANAGER::init(SCRNDRV* scrn_ptr) {
  

  postBody.reserve(256);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.print("no module ");
    return false;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.print("Please upgrade the firmware: ");
    Serial.println(fv);
    return false;
  }
  
  return this->connect_to_WIFI_network(scrn_ptr);
}

bool NETMANAGER::connect_to_WIFI_network(SCRNDRV* scrn_ptr)
{
  int timeout = 10;
  // attempt to connect to WiFi network:
  while ((status != WL_CONNECTED) && (timeout > 0)) {
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(1000);
    timeout--;
  }

  if (status == WL_CONNECTED) {
    int a = 0;
    while (a < 4) {
      scrn_ptr->setShowNetConnect();
      a++;
      delay(100);
    }
    return true;
  } else {
    return false;
  }
}





void NETMANAGER::do_metrics_post(SHED_APP* shddata_ptr, String trigger) {

#ifndef ENABLE_METRIC_POST
  return; //dropout if not enabled.
#endif


  client_task = started;

  String ds = (shddata_ptr->door_status.current_state)?"open":"closed";
  String bl = (shddata_ptr->power_states.blower)?"on":"off"; 
  String fan = (shddata_ptr->power_states.fan)?"on":"off"; 
  String misc = (shddata_ptr->power_states.misc)?"on":"off"; 
  String lis = (shddata_ptr->power_states.lights)?"on":"off"; 

  postBody = "{\"Itemp\":\"" + String(shddata_ptr->environmentals.internal_temp)
             + "\",\"Ihumid\":\"" + String(shddata_ptr->environmentals.internal_humidity)
             + "\",\"DewPoint\":\"" + String(shddata_ptr->environmentals.internal_dewpoint)
             + "\",\"Etemp\":\"" + String(shddata_ptr->environmentals.external_temp) 
             + "\",\"DoorState\":\"" + ds
             + "\",\"Blower\":\"" + bl
             + "\",\"Fan\":\"" + fan
             + "\",\"Misc\":\"" + misc
             + "\",\"Lights\":\"" + lis
             + "\",\"trigger\":\"" + trigger
             + "\"}";
  Serial.print(postBody);
}



bool NETMANAGER::task() {


  //individual SM for web client tasks
  this->do_client_task();
}


void NETMANAGER::do_client_task() {
  switch (client_task) {
    case started:  //The web client has been requested
      this->CT_start_request();
      break;
    case getServerResponse:
      this->get_server_response();
      this->cancel_client_task();
      break;
    default:  //do nothing in these tasks
    case idle:
      break;
  }
}

void NETMANAGER::CT_start_request() {
  if (this->isConnected()) {
    //We are connected... crack on
    if (this->start_client_connection()) {
      client_task = getServerResponse;
    } else {
      //Client request failed.
      this->cancel_client_task();
    }
  } else {
    //cancel this request as we are not connected to internet
    this->cancel_client_task();
  }
}

/*
Can be called at any time to cancel an on going client task.*/
void NETMANAGER::cancel_client_task() {
  //metricsClient.stop();
  client_task = idle;
}


long NETMANAGER::getRSSI() {
  return WiFi.RSSI();
}

void NETMANAGER::getIP(String& ip_ptr) {
  if (this->isConnected()) {
    //IPAddress ip = WiFi.localIP();
    //String s = ip.toString();
    //ip_ptr = s;
    ip_ptr = WiFi.localIP().toString();
  }
}

bool NETMANAGER::isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

//String NETMANAGER::isConnected() {
//get ip and rssi for info screen
//}

/*
Attempts to get the time from the NTP, 
return secs since unis time (1970) or 0 if failed
*/
unsigned long NETMANAGER::getTime() {
#ifdef ENABLE_NTP
  Udp.begin(localPort);

  this->sendNTPpacket(timeServer);  // send an NTP packet to a time server
  delay(1000);
  unsigned long out = this->parseTimeFromPacket();
  Udp.stop();

  return out;
#else
  return 0;
#endif
}


// send an NTP request to the time server at the given address
void NETMANAGER::sendNTPpacket(IPAddress& address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123);  // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


bool NETMANAGER::start_client_connection() {
  bool result = true;

  Serial.println("connecting to server");

  //String postData = "{\"Itemp\":\"30\",\"Ihumid\":\"20\",\"DewPoint\":\"10\",\"Etemp\":\"-5\"}";
  //Serial.println(postData);


  httpClient.beginRequest();
  httpClient.post("/api/shdep/");
  httpClient.sendHeader("Content-Type", "application/json");
  httpClient.sendHeader("Content-Length", postBody.length());
  httpClient.sendHeader("X-Custom-Header", "custom-header-value");
  httpClient.beginBody();
  httpClient.print(postBody);
  httpClient.endRequest();

  Serial.println("connecting to server; completed.");


  //  }
  return result;
}

void NETMANAGER::get_server_response() {

  // read the status code and body of the response
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();
#define OUTPUT_POST_REQ
#ifdef OUTPUT_POST_REQ
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
#endif
}

unsigned long NETMANAGER::parseTimeFromPacket() {
  //uint8_t t_packet[3];
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch % 86400L) / 3600);  // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    // t_packet[0] = ((epoch % 86400L) / 3600); //h
    //t_packet[1] = ((epoch % 3600) / 60); //m
    // t_packet[2] = (epoch % 60);


    Serial.print((epoch % 3600) / 60);  // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60);  // print the second


    return epoch;
  } else {
    return 0;
  }
}
