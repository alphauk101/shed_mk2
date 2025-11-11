#include "network_manager.h"
#include "shdmk3_config.h"
#include <SPI.h>
#include <WiFiUdp.h>


char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;      // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;    // the WiFi radio's status


//const char timeServer[] = "time.nist.gov";  // time.nist.gov NTP server
IPAddress timeServer(162, 159, 200, 123); // pool.ntp.org NTP server
unsigned int localPort = 2390;      // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE];  //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;


bool NETMANAGER::init(SCRNDRV *scrn_ptr) {
  int timeout = 10;

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

long NETMANAGER::getRSSI() {
  return WiFi.RSSI();
}

bool NETMANAGER::isConnected()
{
  return (WiFi.status() == WL_CONNECTED);
}


/*
Attempts to get the time from the NTP, 
return secs since unis time (1970) or 0 if failed
*/
unsigned long NETMANAGER::getTime()
{
  Udp.begin(localPort);

  this->sendNTPpacket(timeServer); // send an NTP packet to a time server
  delay(1000);
  unsigned long out = this->parseTimeFromPacket();
  Udp.stop();
  
  return out;
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
  }else{
    return 0;
  }
}




/*
void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

*/