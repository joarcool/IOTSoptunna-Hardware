/**
******************************************************************************
@brief functions for the ESP8266 wifi-module
@details
@file ESP8266.c
@author  Jonatan Lundqvist Silins, jonls@kth.se
@date 06-04-2021
@version 2
*******************************************************************************/
#include "ESP8266.h"

/* Global variables */
static uint8_t rx_variable;
static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_buffer_index = 0;
static bool error_flag = false;
static bool fail_flag = false;
static int retryConection = 0;


void init_uart_interrupt(void){
	HAL_UART_Receive_IT(&huart4, &rx_variable, 1);
}

/* Probably not the most efficient solution */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

   if (huart->Instance == UART4) {
      rx_buffer[rx_buffer_index++] = rx_variable;    // Add 1 byte to rx_Buffer
    }
      HAL_UART_Receive_IT(&huart4, &rx_variable, 1); // Clear flags and read next byte
}

/* djb2 hashing algorithm which is used in mapping sent commands to the right ESP8266 response code */
const unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

const char* esp8266_send_command(const char* command)
{
	rx_buffer_index = 0;
	error_flag = false;
	fail_flag = false;

	memset(rx_buffer, 0, RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart4, (uint8_t*) command, strlen(command), 100);

	// wait for OK or ERROR/FAIL
	while((strstr(rx_buffer, ESP8266_AT_OK_TERMINATOR) == NULL)){
		if(strstr(rx_buffer, ESP8266_AT_ERROR) != NULL){
			error_flag = true;
			break;
		}
		if(strstr(rx_buffer, ESP8266_AT_FAIL) != NULL){
			fail_flag = true;
			break;
		}
	}

	//HAL_UART_Transmit(&huart4, (uint8_t*) rx_buffer, strlen(rx_buffer), 100);
	//return evaluate(); would more efficient but not as clear in testing
	return get_return(command);
}

const char* esp8266_send_data(const char* data){

	/* if the function is called after an error, cancel */
	if(error_flag || fail_flag)
		return ESP8266_AT_ERROR;
	rx_buffer_index = 0;

	memset(rx_buffer, 0, RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart4, (uint8_t*) data, strlen(data), 100);

	while((strstr(rx_buffer, ESP8266_AT_CLOSED) == NULL));

	return ESP8266_AT_CLOSED;
}

/*void esp8266_get_wifi_command(char* ref){
	sprintf (ref, "%s\"%s\",\"%s\"\r\n", ESP8266_AT_CWJAP_SET, SSID, PWD);
}*/

void esp8266_initialize()
{
	//Step 1, enable interrupts
	init_uart_interrupt();

	//Step 2, initialize station mode
	char set_ESP8266_mode[] = "AT+CWMODE=1\r\n";
	//HAL_UART_Transmit(&huart2, (uint8_t *) set_ESP8266_mode, strlen(set_ESP8266_mode), 100); //For testing
	esp8266_send_command(set_ESP8266_mode);

	//Step 3, connect to wifi using "networkinfo.h"
	char connect_wifi[30];
	sprintf (connect_wifi, "%s\"%s\",\"%s\"\r\n", ESP8266_AT_CWJAP_SET, SSID, PWD);
	//HAL_UART_Transmit(&huart2, (uint8_t *) connect_wifi, strlen(connect_wifi), 100); //For testing
	esp8266_send_command(connect_wifi);

	return;
}

void esp8266_get_connection_command(char* ref, char* connection_type, char* remote_ip, char* remote_port){
	sprintf(ref, "%s\"%s\",\"%s\",%s\r\n", ESP8266_AT_START, connection_type, remote_ip, remote_port);
}

void eps8266_connection()
{
	char tcp_connect[30];
	sprintf(tcp_connect, "%s\"%s\",\"%s\",%s\r\n", ESP8266_AT_START, connection_type, remote_ip, remote_port);
	//HAL_UART_Transmit(&huart2, (uint8_t *) tcp_connect, strlen(tcp_connect), 100); //For testing
	esp8266_send_command(tcp_connect);
}

void esp8266_get_at_send_command(char* ref, uint8_t len){
	sprintf(ref, "%s%d\r\n", ESP8266_AT_SEND, len);
}

uint8_t esp8266_http_get_request(char* ref, const char* http_type, char* uri, char* host){
	sprintf(ref, "%s%s %s\r\n%s%s\r\n%s\r\n\r\n", http_type, uri, HTTP_VERSION, HTTP_HOST, host, HTTP_CONNECTION_CLOSE);
	return (strlen(ref));
}

/* Returns the ESP8266 response code that is in the rx_buffer as a string,
 * this makes debugging and verification through testing easier, at the
 * cost of simplicity.
 */
const char* get_return(const char* command){

	if(strstr(command, ESP8266_AT_CWJAP_SET) != NULL)
		command = ESP8266_AT_CWJAP_SET;
	else if(strstr(command, ESP8266_AT_START) != NULL)
		command = ESP8266_AT_START;
	else if(strstr(command, ESP8266_AT_SEND) != NULL)
		command = ESP8266_AT_SEND;

	KEYS return_type = hash(command);
	switch (return_type) {

		case ESP8266_AT_KEY:

		case ESP8266_AT_GMR_KEY:

		case ESP8266_AT_RST_KEY:

		case ESP8266_AT_CWMODE_STATION_MODE_KEY:

		case ESP8266_AT_CIPMUX_KEY:

		case ESP8266_AT_CWQAP_KEY:
			return evaluate();

		case ESP8266_AT_CWMODE_TEST_KEY:
			if(error_flag || fail_flag)
				return ESP8266_AT_ERROR;
			else {
				if (strstr(rx_buffer, ESP8266_AT_CWMODE_1) != NULL)
					return ESP8266_AT_CWMODE_1;
				else if(strstr(rx_buffer, ESP8266_AT_CWMODE_2) != NULL)
					return ESP8266_AT_CWMODE_2;
				else if(strstr(rx_buffer, ESP8266_AT_CWMODE_3) != NULL)
					return ESP8266_AT_CWMODE_3;
				else
					return ESP8266_AT_UNKNOWN;
			}

		case ESP8266_AT_CWJAP_TEST_KEY:
			if(error_flag || fail_flag)
				return ESP8266_AT_ERROR;
			else {
				if(strstr(rx_buffer, ESP8266_AT_NO_AP))
					return ESP8266_AT_WIFI_DISCONNECTED;
				else
					return ESP8266_AT_WIFI_CONNECTED;
			}

		case ESP8266_AT_CWJAP_SET_KEY:
			if(fail_flag || error_flag){
				if (strstr(rx_buffer, ESP8266_AT_CWJAP_1) != NULL)
					return ESP8266_AT_TIMEOUT;
				else if((strstr(rx_buffer, ESP8266_AT_CWJAP_2) != NULL))
					return ESP8266_AT_WRONG_PWD;
				else if((strstr(rx_buffer, ESP8266_AT_CWJAP_3) != NULL))
					return ESP8266_AT_NO_TARGET;
				else if((strstr(rx_buffer, ESP8266_AT_CWJAP_4) != NULL))
					return ESP8266_AT_CONNECTION_FAIL;
				else
					return ESP8266_AT_ERROR;
			}
			else
				return ESP8266_AT_WIFI_CONNECTED;

		case ESP8266_AT_CIPMUX_TEST_KEY:
			if(error_flag || fail_flag)
				return ESP8266_AT_ERROR;
			else {
				if (strstr(rx_buffer, ESP8266_AT_CIPMUX_0) != NULL)
					return ESP8266_AT_CIPMUX_0;
				else
					return ESP8266_AT_CIPMUX_1;
			}

		case ESP8266_AT_START_KEY:
			if(error_flag || fail_flag)
				return ESP8266_AT_ERROR;
			return ESP8266_AT_CONNECT;

		case ESP8266_AT_SEND_KEY:
			if(error_flag || fail_flag)
				return ESP8266_AT_ERROR;
			return ESP8266_AT_SEND_OK;

		default:
			return ESP8266_NOT_IMPLEMENTED;
			break;
	}
}

const char* evaluate(void){
	if(error_flag || fail_flag)
		return ESP8266_AT_ERROR;
	return ESP8266_AT_OK;
}
