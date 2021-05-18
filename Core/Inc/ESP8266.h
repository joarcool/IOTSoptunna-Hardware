/**
 * @brief header for the ESP8266 wifi-module functions
 * @details This header has all the necessary commands and functions to use the ESP8266 wifi-module

		 The code uses AT Commands to operate the ESP8266, see
		 https://www.espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_en.pdf

		 Commands are case sensitive and should end with /r/n
		 Commands may use 1 or more of these types:
		 Set = AT+<x>=<...> - Sets the value
		 Inquiry = AT+<x>? - See what the value is set at
		 Test = AT+<x>=? - See the possible options
		 Execute = AT+<x> - Execute a command
 * @file ESP8266.h
 * @author  Joar Edling, joaredl@kth.se
 * @date 06-05-2021
 * @version 1.0
**/
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "networkinfo.h"

#define RX_BUFFER_SIZE 4096


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


/* Used for comparing command that was sent */
static const char ESP8266_AT_CWJAP_SET[]			= "AT+CWJAP=";
static const char ESP8266_AT_START[]				= "AT+CIPSTART=";
static const char ESP8266_AT_SEND[]					= "AT+CIPSEND=";

/* FUNCTIONS */
/**
 * @brief start RX interrupt for UART4
 * @param void
 * @return void
 */
void init_uart_interrupt(void);

/**
 * @brief callback for UART4 RX interrupt
 * @param UART_HandleTypeDef* huart handle
 * @return void
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/**
 * @brief Gets the hash number for string
 * @param const char* string to get hash number for
 * @return const unsigned long the hash number
 */
const unsigned long hash(const char*);

/**
 * @brief send command to ESP8266
 * @param char* command to send
 * @return ESP8266 response
 */
const char* esp8266_sendCommand(const char*);

/**
 * @brief Sets up a uart-interrupt for ESP8266, Initializes ESP8266 using Station mode and connects to the
 * WiFi using the giver information in "network.h".
 * @param void
 * @return 1 if successful init, 0 if not successful
 */
int esp8266_initialize(void);

/**
 * @brief Deliveres the message via tcp connection to the API site
 * @param float distance
 * @return 1 if successful TCP connection and send, 0 if not successful
 */
int esp8266_sendDistance(float);;

/**
 * @brief matches command to ESP8266 return type. Looks up the hash of the command, and then looks for
 * the ESP8266 response which should be returned.
 * @param char* command to match to a return type
 * @return char* return ESP8266 response depending on command and its outcome
 */
const char* get_return(const char*);

/**
 * @brief Evaluate ESP8266 response, if any flags were set return "ERROR" else "OK".
 * Used for applicable AT commands that only return basic responses.
 * @return char* "OK" or "ERROR"
 */
const char* evaluate(void);
