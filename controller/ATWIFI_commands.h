#ifndef __ATCOMMANDS__
#define __ATCOMMANDS__


#define ATCMD_OK                  "AT\r\n"
#define ATCMD_RESET               "AT+RST\r\n"
#define ATCMD_GET_WIFI_STATUS     "AT+CWJAP?\r\n"

#define ATCMD_SET_WIFI_MODE       "AT+CWMODE=1\r\n"
#define ATCMD_SET_WIFI_CREDS      "AT+CWJAP=\"fudgemesh\",\"ancienthill347\"\r\n"
#define ATCMD_SET_WIFI_AUTOCONN   "AT+CWAUTOCONN=1\r\n"
#define ATCMD_STORE_WIFI_STATE    "AT+CIFSR\r\n"

#define ATCMD_SET_MUX_MODE        "AT+CIPMUX=0\r\n"
#define ATCMD_CONNECT_SERVER      "AT+CIPSTART=\"TCP\",\"83.217.160.46\",8080\r\n"
#define ATCMD_OPEN_DATASTREAM     "AT+CIPSEND="
#define ATCMD_CLOSE_CONNECT       "AT+CIPCLOSE\r\n"

#endif