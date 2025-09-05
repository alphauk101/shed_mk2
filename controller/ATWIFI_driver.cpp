#include "ATWIFI_driver.h"
#include "ATWIFI_commands.h"
#include <string.h>

#define BLOCKING_MAX_WAIT 15000
#define UART_BUFF_SIZE 128

#define WD_TASK_TIMEOUT_MS 5000

#define WIFI_WAIT_FOR_WARMUP_MS 1000
#define WIFI_WAITFOR_CONNECTION_MS 10000

#define AT_COMMAND_CONNECT_TIME 1000

//Typical response time taken by the modem to respond to a AT command, may not apply to all requests
#define AT_COMMAND_RESP_TIME_MS 100

#define MCR_HAS_OP_TIMER_EXP millis() > g_wifidata.op_timer

//Watchdog helpers
#define WATCHDOG_DISABLED 0xFFFFFFFF
#define MCR_DISABLE_WATCHDOG g_wifidata.watchdog_timer = WATCHDOG_DISABLED
#define MCR_IS_WATCHDOG_ENABLED g_wifidata.watchdog_timer != WATCHDOG_DISABLED
#define MCR_SET_WATCHDOG_TASK_TIMEOUT(X) g_wifidata.watchdog_timer = (millis() + X)
#define MCR_HAS_WATCHDOG_EXPIRED millis() > g_wifidata.watchdog_timer

typedef enum {
  idle,
  check_for_wifi,
  waitfor_wifi_status,
  validate_conn_data,

  //Modem init ops
  init_modem_stage_0,
  init_modem_stage_1,
  init_modem_stage_2,
  init_modem_stage_3,

  //Connection ops
  start_connection_op,
  start_connection_op_stage_1,
  start_connection_op_stage_1_5,
  start_connection_op_stage_2,
  start_connection_op_stage_3,
  start_connection_op_stage_4,

  //Do HTTP post ops
  http_post_stage_1,
  http_post_stage_2
} networkSMstate;

typedef struct {
  unsigned long op_timer;

  unsigned long watchdog_timer;

  void (*event_CB)(int);

  char rx_buffer[UART_BUFF_SIZE];
  uint16_t rx_cursor;

  networkSMstate state;

  String rx_string;

  String httpPOST_cmd_string;

  WifiStatus network_status_data;

  String* http_json_ptr;
  unsigned int http_json_length;
} WIFI_APP;
static WIFI_APP g_wifidata;


/*Handles the incoming serial events*/
void atwifi_driver::serialEventHandler() {
  while (Serial.available()) {
    if (g_wifidata.rx_cursor < UART_BUFF_SIZE)
      g_wifidata.rx_buffer[g_wifidata.rx_cursor++] = (char)Serial.read();
  }
}


/*
Initialises all components of this module.
*/
void atwifi_driver::init(void (*event_CB)(int)) {
  g_wifidata.op_timer = 0;
  g_wifidata.event_CB = event_CB;
  g_wifidata.state = idle;
  g_wifidata.rx_string.reserve(UART_BUFF_SIZE);
  g_wifidata.httpPOST_cmd_string.reserve(300);
  this->flush_rx_buffer();
  MCR_DISABLE_WATCHDOG;

  //Start setup
  this->init_modem();
}

//Starts the non blocking get connection status
bool atwifi_driver::startGetConnectionStatus() {

#ifdef NON_BLOCKING
  if (g_wifidata.state == idle) {
    g_wifidata.state = check_for_wifi;
    return true;
  }
  return false;
#else
  this->flush_rx_buffer();
  //Start the wifi setup....
  this->send_AT_command(ATCMD_GET_WIFI_STATUS);

  if (this->blockWaitForCommand("fudgemesh")) {
    g_wifidata.event_CB(EVNT_CONNECTION_OK);
  } else {
    //Serial.println(g_wifidata.rx_cursor);
    g_wifidata.event_CB(EVNT_CONNECTION_FAILED);
  }

  return true;
#endif
}


/*
Blocking - polls for serial data, if given command is found then 
true is returned, if false then timeout exp with no command found.

- This fxn does not flush the rx buffer this must be done by the calling fxn.
*/
bool atwifi_driver::blockWaitForCommand(const char* cmd_ptr) {
  String tmpstr;
  unsigned long timeout = (millis() + BLOCKING_MAX_WAIT);
  bool cmd_found = false;
  while ((millis() < timeout) && (!cmd_found) && (g_wifidata.rx_cursor < UART_BUFF_SIZE)) {
    this->serialEventHandler();
    if (g_wifidata.rx_cursor > 1) {
      tmpstr = String(g_wifidata.rx_buffer);
      if (tmpstr.indexOf(cmd_ptr) != -1) {
        cmd_found = true;
      }
    }
  }
  return cmd_found;
}



bool atwifi_driver::startHTTPPost(String* json_ptr, unsigned int len) {
  //if (g_wifidata.state == idle) {

  g_wifidata.http_json_ptr = json_ptr;
  g_wifidata.http_json_length = len;

  this->createHTTPPostCommand(g_wifidata.http_json_length);

  return true;
  // }
  // return false;
}


bool atwifi_driver::startConnectToWifi() {
  if (g_wifidata.state == idle) {
    g_wifidata.state = start_connection_op;
    return true;
  }
  return false;
}


void atwifi_driver::flush_rx_buffer() {
  g_wifidata.rx_cursor = 0;
  memset(g_wifidata.rx_buffer, 0, UART_BUFF_SIZE);
}

void atwifi_driver::init_modem() {
  g_wifidata.state = init_modem_stage_0;

  //Set a holdoff timer.
  g_wifidata.op_timer = (millis() + WIFI_WAIT_FOR_WARMUP_MS);
}

void atwifi_driver::getATOK() {
  this->flush_rx_buffer();
  this->send_AT_command(ATCMD_OK);
  g_wifidata.op_timer = (millis() + AT_COMMAND_RESP_TIME_MS);

  if (this->blockWaitForCommand("OK")) {
    g_wifidata.event_CB(EVNT_SETUP_OK);
  } else {
    g_wifidata.event_CB(EVNT_SETUP_FAIL);
  }
}

/*
Performs any operations related to the this module.

important! should be called in the main loop regularly!
*/
void atwifi_driver::task() {

  switch (g_wifidata.state) {
    case init_modem_stage_0:
      //wait for timeout
      if (MCR_HAS_OP_TIMER_EXP) {
        g_wifidata.state = init_modem_stage_1;
      }
      break;
    case init_modem_stage_1:

#ifdef NON_BLOCKING
      //Send an AT to check OK back
      //Flush the buffer
      this->flush_rx_buffer();
      this->send_AT_command(ATCMD_OK);
      g_wifidata.op_timer = (millis() + AT_COMMAND_RESP_TIME_MS);
      g_wifidata.state = init_modem_stage_2;
#else
      g_wifidata.state = idle;
      this->getATOK();
#endif

      break;
    case init_modem_stage_2:
      if (MCR_HAS_OP_TIMER_EXP) {
        g_wifidata.state = idle;
        //Serial.println(g_wifidata.rx_cursor,DEC);
        if (g_wifidata.rx_cursor > 0) {
          String tmpstr = String(g_wifidata.rx_buffer);
          if (tmpstr.indexOf("OK") > -1) {
            g_wifidata.event_CB(EVNT_SETUP_OK);
          } else {
            g_wifidata.event_CB(EVNT_SETUP_FAIL);
          }
        } else {
          //no data from modem
          g_wifidata.event_CB(EVNT_SETUP_FAIL);
        }
        //Ensure the op is idled
      }
      break;

    //Check WIFI status operation.
    case check_for_wifi:
      //Flush the buffer
      this->flush_rx_buffer();
      //Start the wifi setup....
      this->send_AT_command(ATCMD_GET_WIFI_STATUS);
      g_wifidata.op_timer = (millis() + AT_COMMAND_RESP_TIME_MS);
      g_wifidata.state = waitfor_wifi_status;
      //start watchdog
      MCR_SET_WATCHDOG_TASK_TIMEOUT(WD_TASK_TIMEOUT_MS);
      break;
    case waitfor_wifi_status:
      if (MCR_HAS_OP_TIMER_EXP) {
        if (g_wifidata.rx_cursor > 25) {
          String tmpstr = String(g_wifidata.rx_buffer);
          g_wifidata.state = idle;
          MCR_DISABLE_WATCHDOG;
          if (tmpstr.indexOf("fudgemesh") != -1) {
            g_wifidata.event_CB(EVNT_CONNECTION_OK);
          } else {
            //Serial.println(g_wifidata.rx_cursor);
            g_wifidata.event_CB(EVNT_CONNECTION_FAILED);
          }
        }
      }
      break;
    case validate_conn_data:
      //Report back to main that the connection is ok or not.
      //its the mains responsibility to deal with the next operation
      //go to idle.
      g_wifidata.state = idle;
      MCR_DISABLE_WATCHDOG;

      if (g_wifidata.network_status_data.isConnected) {
        g_wifidata.event_CB(EVNT_CONNECTION_OK);
      } else {
        g_wifidata.event_CB(EVNT_CONNECTION_FAILED);
      }

      break;

    //Connection ops
    case start_connection_op:
      //We dont care about the response, just need to send the command.
      this->send_AT_command(ATCMD_SET_WIFI_MODE);
      g_wifidata.op_timer = (millis() + AT_COMMAND_RESP_TIME_MS);
      g_wifidata.state = start_connection_op_stage_1;

      //start watchdog
      //MCR_SET_WATCHDOG_TASK_TIMEOUT(WD_TASK_TIMEOUT_MS);
      break;
    case start_connection_op_stage_1:
      if (MCR_HAS_OP_TIMER_EXP) {
        this->send_AT_command(ATCMD_SET_WIFI_CREDS);
        g_wifidata.op_timer = (millis() + WIFI_WAITFOR_CONNECTION_MS);
        g_wifidata.state = start_connection_op_stage_1_5;
      }
      break;
      //Wait for busy...
    case start_connection_op_stage_1_5:
      if (MCR_HAS_OP_TIMER_EXP) {
        if (g_wifidata.rx_cursor > 0) {
          String tmpstr = String(g_wifidata.rx_buffer);
          if (tmpstr.indexOf("busy") != -1) {
            //still busy wait some more
            g_wifidata.op_timer = (millis() + WIFI_WAITFOR_CONNECTION_MS);
          } else {
            //Were good carry on as usual
            g_wifidata.state = start_connection_op_stage_2;
          }
          this->flush_rx_buffer();
        } else {
          this->send_AT_command(ATCMD_OK);
          g_wifidata.op_timer = (millis() + AT_COMMAND_RESP_TIME_MS);
        }
      }
      break;
    case start_connection_op_stage_2:
      if (MCR_HAS_OP_TIMER_EXP) {
        this->send_AT_command(ATCMD_SET_WIFI_AUTOCONN);
        g_wifidata.op_timer = (millis() + AT_COMMAND_RESP_TIME_MS);
        g_wifidata.state = start_connection_op_stage_3;
      }
      break;
    case start_connection_op_stage_3:
      if (MCR_HAS_OP_TIMER_EXP) {
        this->send_AT_command(ATCMD_STORE_WIFI_STATE);
        g_wifidata.op_timer = (millis() + AT_COMMAND_RESP_TIME_MS);
        g_wifidata.state = start_connection_op_stage_4;

        //Disable WD as this may trigger
        MCR_DISABLE_WATCHDOG;
      }
      break;
    case start_connection_op_stage_4:
      //Once the timer expires, simply check the wifi state
      if (MCR_HAS_OP_TIMER_EXP) {
        g_wifidata.state = check_for_wifi;
      }
      break;

    case http_post_stage_1:
      if (MCR_HAS_OP_TIMER_EXP) {
        /*
        if (g_wifidata.rx_cursor > 0) {
          String tmpstr = String(g_wifidata.rx_buffer);
          if ((tmpstr.indexOf("CONNECT") != -1) || (tmpstr.indexOf("CONNECTED") != -1)) {
            //Post is ready to be sent
            g_wifidata.state = http_post_stage_2;
          } else {
            Serial.print("count");
            Serial.println(tmpstr);
            //its failed
            this->kill_operation();
            g_wifidata.event_CB(EVNT_OP_ABORTED_ERROR);
          }
          }
          */
        g_wifidata.state = http_post_stage_2;
      }

      break;
    case http_post_stage_2:
      //Send out the post
      this->sendPostBody();
      //goto idle as we are done
      g_wifidata.state = idle;
      break;
    case idle:  //do nothing
    default:
      break;
  }

  if ((MCR_IS_WATCHDOG_ENABLED) && (MCR_HAS_WATCHDOG_EXPIRED)) {
    this->kill_operation();
    MCR_DISABLE_WATCHDOG;
    g_wifidata.event_CB(EVNT_OP_ABORTED_ERROR);
  }
}


void atwifi_driver::kill_operation() {
  //resets everything... operation aborted.
  g_wifidata.state = idle;
  flush_rx_buffer();
}


void atwifi_driver::send_AT_command(const char* at_command) {
  Serial.write(at_command);
  //delay(50);
}



/**
 * @brief Parses the response from an ESP32's "AT+CWJAP?" command.
 * * @param response The raw string response from the ESP32.
 * @return A WifiStatus struct containing the parsed information.
 */
WifiStatus atwifi_driver::parseWifiStatus(const String& response) {
  WifiStatus status;           // Create an instance of the struct to hold our data.
  status.isConnected = false;  // Assume not connected by default.

  // Check if the response indicates a successful connection.
  // The key identifier is the "+CWJAP:" prefix.
  if (response.startsWith("+CWJAP:")) {
    // Find the starting position of the SSID (after the first quote).
    int ssidStart = response.indexOf('"') + 1;
    if (ssidStart == 0) return status;  // indexOf returns -1 on failure, +1 makes it 0.

    // Find the ending position of the SSID (at the next quote).
    int ssidEnd = response.indexOf('"', ssidStart);
    if (ssidEnd == -1) return status;  // Malformed string.

    // Extract the SSID substring.
    status.ssid = response.substring(ssidStart, ssidEnd);

    // Find the starting position of the BSSID.
    int bssidStart = response.indexOf('"', ssidEnd + 1) + 1;
    if (bssidStart == 0) return status;

    // Find the ending position of the BSSID.
    int bssidEnd = response.indexOf('"', bssidStart);
    if (bssidEnd == -1) return status;

    // Extract the BSSID substring.
    status.bssid = response.substring(bssidStart, bssidEnd);

    // Find the starting position of the channel number.
    // It's after the BSSID's closing quote and a comma.
    int channelStart = bssidEnd + 2;
    if (channelStart >= response.length()) return status;

    // Find the ending position of the channel number (at the next comma).
    int channelEnd = response.indexOf(',', channelStart);
    if (channelEnd == -1) return status;

    // Extract the channel substring and convert it to an integer.
    status.channel = response.substring(channelStart, channelEnd).toInt();

    // The RSSI value starts right after the channel's comma.
    int rssiStart = channelEnd + 1;
    if (rssiStart >= response.length()) return status;

    // The rest of the relevant part of the string is the RSSI.
    // We can read the substring until the next carriage return.
    int rssiEnd = response.indexOf('\r', rssiStart);
    if (rssiEnd == -1) {  // If no CR, just take the rest of the string.
      rssiEnd = response.length();
    }

    // Extract the RSSI substring and convert it to an integer.
    status.rssi = response.substring(rssiStart, rssiEnd).toInt();

    // If we've gotten this far, parsing was successful.
    status.isConnected = true;

  } else if (response.indexOf("No AP") != -1) {
    // This handles the case where the ESP32 responds with "No AP".
    status.isConnected = false;
  }

  return status;
}

// A helper function to print the status details to the Serial Monitor.
void atwifi_driver::printWifiStatus(const WifiStatus& status) {
  if (status.isConnected) {
    Serial.println("--- Wi-Fi Status: Connected ---");
    Serial.print("SSID: ");
    Serial.println(status.ssid);
    Serial.print("BSSID: ");
    Serial.println(status.bssid);
    Serial.print("Channel: ");
    Serial.println(status.channel);
    Serial.print("RSSI: ");
    Serial.println(status.rssi);
  } else {
    Serial.println("--- Wi-Fi Status: Not Connected ---");
  }
  Serial.println("-----------------------------");
}

void atwifi_driver::createHTTPPostCommand(unsigned int post_length) {
  g_wifidata.httpPOST_cmd_string = "POST /shed HTTP/1.1\r\nHost: 83.217.160.46:8080\r\nContent-Type: application/json\r\nConnection: close\r\nContent-Length:"
                                   + String(g_wifidata.http_json_length) + "\r\n\r\n" + *g_wifidata.http_json_ptr;

  //Start the connection!
  this->send_AT_command(ATCMD_SET_MUX_MODE);
  delay(AT_COMMAND_RESP_TIME_MS);

  this->flush_rx_buffer();

  this->send_AT_command(ATCMD_CONNECT_SERVER);

#ifdef NON_BLOCKING
  g_wifidata.op_timer = (millis() + AT_COMMAND_CONNECT_TIME);
  g_wifidata.state = http_post_stage_1;
#else  //blocking
  bool cmd_found = this->blockWaitForCommand("CONNECT");

  //delay to allow the command to complete
  delay(50);

  if (cmd_found) 
  {
    this->sendPostBody();
    delay(AT_COMMAND_RESP_TIME_MS);

    //Close the connection
    this->send_AT_command(ATCMD_CLOSE_CONNECT);
    delay(AT_COMMAND_RESP_TIME_MS);

    g_wifidata.state = idle;
  } else {
    g_wifidata.event_CB(EVNT_OP_ABORTED_ERROR);
  }
#endif
}

void atwifi_driver::sendPostBody() {

  String cip_len = ATCMD_OPEN_DATASTREAM + String(g_wifidata.httpPOST_cmd_string.length()) + "\r\n ";
  //String cip_len = ATCMD_OPEN_DATASTREAM + String(162) + "\r\n ";
  char tmp[cip_len.length()];
  cip_len.toCharArray(tmp, cip_len.length());
  this->send_AT_command(tmp);

  delay(AT_COMMAND_RESP_TIME_MS);

  //send the post http data
  for (int a = 0; a < g_wifidata.httpPOST_cmd_string.length(); a++) {
    Serial.write(g_wifidata.httpPOST_cmd_string[a]);
  }
}
