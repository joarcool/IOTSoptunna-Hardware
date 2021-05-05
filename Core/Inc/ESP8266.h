/**
******************************************************************************
@brief header for the ESP8266 wifi-module functions
@details This header has all the necessary commands and functions to use the
		 ESP8266 wifi-module, including relevant AT-commands and functions
		 to use them in regards to the Office Environment Monitor project.
@file ESP8266.h
@author  Jonatan Lundqvist Silins, jonls@kth.se
@date 06-04-2021
@version 1.0
*******************************************************************************/

/*----------Includes------------*/
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "networkinfo.h"

/*----------Defines------------*/
#define RX_BUFFER_SIZE 4096

/*----------Enums------------*/

/* djb2 hash keys
 * Each key maps to corresponding AT command string
 */
typedef enum COMMAND_KEYS {
	ESP8266_AT_KEY 						= 2088901425,
	ESP8266_AT_RST_KEY	 				= 617536853,
	ESP8266_AT_GMR_KEY	 				= 604273922,
	ESP8266_AT_CWMODE_STATION_MODE_KEY 	= 608151977,
	ESP8266_AT_CWMODE_TEST_KEY			= 4116713283,
	ESP8266_AT_CWQAP_KEY				= 445513592,
	ESP8266_AT_CWJAP_TEST_KEY			= 1543153456,
	ESP8266_AT_CWJAP_SET_KEY 			= 2616259383,
	ESP8266_AT_CIPMUX_KEY				= 423755967,
	ESP8266_AT_CIPMUX_TEST_KEY			= 3657056785,
	ESP8266_AT_START_KEY				= 3889879756,
	ESP8266_AT_SEND_KEY					= 898252904
} KEYS;

/*----------Strings------------*/

/* ESP8266 response codes */
static const char ESP8266_NOT_IMPLEMENTED[]		 = "NOT IMPLEMENTED";
static const char ESP8266_AT_OK_TERMINATOR[]     = "OK\r\n";
static const char ESP8266_AT_OK[] 				 = "OK";
static const char ESP8266_AT_ERROR[] 			 = "ERROR";
static const char ESP8266_AT_FAIL[] 			 = "FAIL";
static const char ESP8266_AT_GOT_IP[] 			 = "WIFI GOT IP";
static const char ESP8266_AT_WIFI_CONNECTED[] 	 = "WIFI CONNECTED";
static const char ESP8266_AT_WIFI_DISCONNECTED[] = "WIFI DISCONNECTED";
static const char ESP8266_AT_CONNECT[] 		 	 = "CONNECT";
static const char ESP8266_AT_CLOSED[] 			 = "CLOSED";
static const char ESP8266_AT_SEND_OK[] 			 = "SEND OK";
static const char ESP8266_AT_NO_AP[] 			 = "No AP\r\n";
static const char ESP8266_AT_UNKNOWN[]			 = "UNKNOWN";
static const char ESP8266_AT_CWMODE_1[]			 = "CWMODE_CUR:1";
static const char ESP8266_AT_CWMODE_2[]			 = "CWMODE_CUR:2";
static const char ESP8266_AT_CWMODE_3[]			 = "CWMODE_CUR:3";
static const char ESP8266_AT_CWJAP_1[]			 = "CWJAP:1";
static const char ESP8266_AT_CWJAP_2[]			 = "CWJAP:2";
static const char ESP8266_AT_CWJAP_3[]			 = "CWJAP:3";
static const char ESP8266_AT_CWJAP_4[]			 = "CWJAP:4";
static const char ESP8266_AT_TIMEOUT[]			 = "connection timeout";
static const char ESP8266_AT_WRONG_PWD[]		 = "wrong password";
static const char ESP8266_AT_NO_TARGET[]	     = "cannot find the target AP";
static const char ESP8266_AT_CONNECTION_FAIL[]	 = "connection failed";
static const char ESP8266_AT_CIPMUX_0[]	 		 = "CIPMUX:0";
static const char ESP8266_AT_CIPMUX_1[]	 		 = "CIPMUX:1";

/* HTTP request strings*/
// TODO: these should probably be hard coded as full requests later
static const char HTTP_GET[]	 		 		 = "GET ";
static const char HTTP_POST[]	 		 		 = "POST ";
static const char HTTP_VERSION[]	 		     = "HTTP/1.1";
static const char HTTP_HOST[]	 		         = "Host: ";
static const char HTTP_CONNECTION_CLOSE[]	     = "Connection: close";
static const char CRLF[] 						 = "\r\n";


/* AT Commands for the ESP8266, see
 * https://www.espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_en.pdf
 *
 * Commands are case sensitive and should end with /r/n
 * Commands may use 1 or more of these types:
 * Set = AT+<x>=<...> - Sets the value
 * Inquiry = AT+<x>? - See what the value is set at
 * Test = AT+<x>=? - See the possible options
 * Execute = AT+<x> - Execute a command
 *
 * Some commands seem to be outdated, and COMMAND_CUR and COMMAND_DEF should be used instead.
 * CUR will not write the value to flash, DEF will write the value to flash and be used as the default in the future.
 *
 **/

/* Tests AT startup.
 *
 * Returns: OK
 */
static const char ESP8266_AT[]						= "AT\r\n";


/* Restarts the module.
 *
 * Returns: OK
 */
static const char ESP8266_AT_RST[]					= "AT+RST\r\n";

/* Checks version information. */
static const char ESP8266_AT_GMR[]					= "AT+GMR\r\n";

/*Checks current wifi-mode.
 *
 * Returns: <mode>
 * 1: Station Mode
 * 2: SoftAP Mode
 * 3: SoftAP+Station Mode
 */
static const char ESP8266_AT_CWMODE_TEST[]			= "AT+CWMODE_CUR?\r\n";

/*Sets the wifi-mode to station.
 * The module will work as client.
 * Note: setting not saved in flash...
 */
static const char ESP8266_AT_CWMODE_STATION_MODE[]	= "AT+CWMODE=1\r\n";

/*Query the AP for connections*/
static const char ESP8266_AT_CWJAP_TEST[]			= "AT+CWJAP?\r\n";

/*Sets a connection to an AP
 *
 * Command format: AT+CWJAP_CUR=<ssid>,<pwd>
 * <ssid>: the SSID of the target AP.
 * <pwd>: password, MAX: 64-byte ASCII.
 * Note: the command needs Station Mode to be enabled.
 *
 * The command returns an error if:
 * connection times out
 * wrong password
 * cannot find the target AP
 * connection failed
 */
static const char ESP8266_AT_CWJAP_SET[]			= "AT+CWJAP="; // add "ssid","pwd" + CRLF

/* Disconnect connected AP */
static const char ESP8266_AT_CWQAP[]				= "AT+CWQAP\r\n";

/* Disable auto connect to AP
 * Writes to flash...
 * This setting is necessary when testing
 */
static const char ESP8266_AT_CWAUTOCONN[]			= "AT+CWAUTOCONN=0";

/* Set single connection */
static const char ESP8266_AT_CIPMUX_SINGLE[]		= "AT+CIPMUX=0\r\n";

/* Query CIPMUX setting */
static const char ESP8266_AT_CIPMUX_TEST[]			= "AT+CIPMUX?\r\n";

/* Establishes TCP connection
 *
 * Assumes AT+CIPMUX=0
 * Use following format:
 * AT+CIPSTART=<type>,<remote	IP>,<remote	port>[,<TCP	keep alive>]
 *
 * Example: AT+CIPSTART="TCP","iot.espressif.cn",8000
 */
static const char ESP8266_AT_START[]				= "AT+CIPSTART=";

/* Disconnect a connection */
static const char ESP8266_AT_STOP[]					= "AT+CIPCLOSE=0";

/* Send data of desired length */
static const char ESP8266_AT_SEND[]					= "AT+CIPSEND=";



/*============================================================================
								FUNCTIONS FOR ESP8266
==============================================================================*/


/**
 * @brief build the command for connection to AP
 * @param char* buffer, where the command is stored into
 * @return void
 */
//void esp8266_get_wifi_command(char*);

/**
 * @brief Sets up a uart-interrupt for ESP8266, Initializes ESP8266 using Station mode and connects to the
 * wifi using the giver information in "network.h".
 * @param void
 * @return void
 */
void esp8266_initialize(void);

/**
 * @brief Establishes a wifi-connection using the given information in "networkinfo.h"
 * @param void
 * @return void
 */
void esp8266_connectWifi(void);

/**
 * @brief Establishes a TCP, UDP or SSL connection using the given information in "networkinfo.h"
 * @param void
 * @return void
 */
void eps8266_connection(void);
/**
 * @brief build the command for connection to a website
 * @param char* ref, where the command is stored into
 * @param char* connection_type, type of connection "TCP", "UDP" or "SSL"
 * @param char* remote_ip, the ip to connect to, can also be a url
 * @param char* remote_port, port to connect
 * @return void
 */
void
esp8266_get_connection_command(char* ref, char* connection_type,
							   char* remote_ip, char* remote_port);


void
esp8266_get_at_send_command(char*, uint8_t);

uint8_t
esp8266_http_get_request(char*, const char*, char*, char*);

/**
 * @brief start RX interrupt for UART4
 * @param void
 * @return void
 */
void
init_uart_interrupt(void);

/**
 * @brief callback for UART4 RX interrupt
 * @param UART_HandleTypeDef* huart handle
 * @return void
 */
void
HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/**
 * @brief send command to ESP8266
 * @param char* command to send
 * @return ESP8266 response
 */
const char* esp8266_send_command(const char*);

/**
 * @brief send data to ESP8266, this is used after calling cipsend
 * where the length of the data that will be sent has been specified.
 * @param char* data to send
 * @return ESP8266 response
 */
const char* esp8266_send_data(const char*);

/**
 * @brief get hash number for string
 * @param const char* string to get hash number for
 * @return const unsigned long the hash number
 */
const unsigned long
hash(const char*);

/**
 * @brief Evaluate ESP8266 response, if any flags were set return "ERROR" else "OK".
 * Used for applicable AT commands that only return basic responses.
 * @return char* "OK" or "ERROR"
 */
const char*
evaluate(void);

/**
 * @brief matches command to ESP8266 return type. Looks up the hash of the command, and then looks for
 * the ESP8266 response which should be returned.
 * @param char* command to match to a return type
 * @return char* return ESP8266 response depending on command and its outcome
 */
const char*
get_return(const char*);
