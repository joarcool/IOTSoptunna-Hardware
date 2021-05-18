/**
 * @brief ESP8266.c contains all the functions for the ESP8266 wifi-module used for smartbin
 * @details
 * @file ESP8266.c
 * @author  Joar Edling, joaredl@kth.se
 * @date 06-05-2021
 * @version 1
**/
#include "ESP8266.h"

/* Global variables */
static uint8_t rx_variable;
static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_buffer_index = 0;
static bool error_flag = false;
static bool fail_flag = false;

/* Initialize interrupts for ESP8266 */
void init_uart_interrupt(void){
	HAL_UART_Receive_IT(&huart4, &rx_variable, 1);
}

/* Used for reading the response message sent back from ESP8266 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

   if (huart->Instance == UART4) {
      rx_buffer[rx_buffer_index++] = rx_variable;    // Add 1 byte to rx_Buffer
    }
      HAL_UART_Receive_IT(&huart4, &rx_variable, 1); // Clear flags and read next byte
}

/* djb2 hashing algorithm which is used in mapping sent commands to the right ESP8266 response code */
const unsigned long hash(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

/* Used for sending commands to ESP8266 and send back the response */
const char* esp8266_sendCommand(const char* command)
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
	return get_return(command);
}

/* Used for initialize the ESP8266 */
int esp8266_initialize()
{
	char uart_buf[50];

	//Step 1, enable interrupts
	init_uart_interrupt();

	//Step 2, initialize station mode
	char set_ESP8266_mode[] = "AT+CWMODE=1\r\n";
	//HAL_UART_Transmit(&huart2, (uint8_t *) set_ESP8266_mode, strlen(set_ESP8266_mode), 100); //For testing
	esp8266_sendCommand(set_ESP8266_mode);

	//Step 3, connect to wifi using "networkinfo.h"
	char connect_wifi[30];
	char* response;
	int count = 0;
	sprintf (connect_wifi, "%s\"%s\",\"%s\"\r\n", ESP8266_AT_CWJAP_SET, SSID, PWD);
	//HAL_UART_Transmit(&huart2, (uint8_t *) connect_wifi, strlen(connect_wifi), 100); //For testing
	sprintf(uart_buf, "Connecting to: %s\r\n", SSID);
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
	while(response != ESP8266_AT_WIFI_CONNECTED)
	{
		response = esp8266_sendCommand(connect_wifi);
		sprintf(uart_buf, "Response from ESP8266: %s\r\n", response);
		HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
		if(response != ESP8266_AT_WIFI_CONNECTED)
		{
			sprintf(uart_buf, "Trying again...\r\n");
			HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
			HAL_Delay(1000);
			count++;
			if(count >= 5)
				return 0;
		}
	}

	return 1;
}

/* Used for sending distance to the smartbin website */
int esp8266_sendDistance(float distance)
{
	char uart_buf[60];
	char* response;
	int count = 0;

	//Step 1, establish a TCP connection with smartbin webiste
	char tcp_connect[300];
	sprintf(uart_buf, "Establishing a TCP connection with smartbin website...\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
	sprintf(tcp_connect, "%s\"%s\",\"%s\",%s\r\n", ESP8266_AT_START, connection_type, remote_ip, remote_port);
	while(response != ESP8266_AT_CONNECT)
	{
		response = esp8266_sendCommand(tcp_connect);
		sprintf(uart_buf, "Response from ESP8266: %s\r\n", response);
		HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
		if(response != ESP8266_AT_CONNECT)
		{
			sprintf(uart_buf, "Trying again...\r\n");
			HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
			/*sprintf(uart_buf, "Response from ESP8266: %s\r\n", response);
			HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
			*/HAL_Delay(1000);
			count++;
			if(count >= 5)
				return 0;
		}
	}


	//Step 2, send the HTTP message containing distance to smartbin website and close the TCP connection
	sprintf(uart_buf, "Sending update message to smartbin website...\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
	char http_request[200];
	sprintf(http_request, "GET /api/data?code=Dx7Hf/AoJhpxh3mVtnWh94wXNe6a2ImWuNtIYVEGlB8v3a3GVj2F5w==&deviceId=%s&distance=%.1f HTTP/1.1\r\n"
			"Host: smartbin.azurewebsites.net\r\n"
			"Connection: close\r\n\r\n", deviceID, distance);

	char tcp_send[100];
	sprintf(tcp_send, "AT+CIPSEND=%d\r\n", strlen(http_request));
	response = esp8266_sendCommand(tcp_send);
	sprintf(uart_buf, "Response from ESP8266: %s\r\n", response);
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);

	HAL_UART_Transmit(&huart4, (uint8_t*) http_request, strlen(http_request), 100);
	sprintf(uart_buf, "Update message sent!\r\n\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);

	return 1;
}
/* Returns the ESP8266 response code that is in the rx_buffer as a string,
 * this makes debugging and verification through testing easier.
 */
const char* get_return(const char* command)
{

	if(strstr(command, ESP8266_AT_CWJAP_SET) != NULL)
		command = ESP8266_AT_CWJAP_SET;
	else if(strstr(command, ESP8266_AT_START) != NULL)
		command = ESP8266_AT_START;
	else if(strstr(command, ESP8266_AT_SEND) != NULL)
		command = ESP8266_AT_SEND;

	KEYS return_type = hash(command);
	switch (return_type)
	{

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
			else
			{
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
			else
			{
				if(strstr(rx_buffer, ESP8266_AT_NO_AP))
					return ESP8266_AT_WIFI_DISCONNECTED;
				else
					return ESP8266_AT_WIFI_CONNECTED;
			}

		case ESP8266_AT_CWJAP_SET_KEY:
			if(fail_flag || error_flag)
			{
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
			else
			{
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

/* Shortly used for evaluating if error or not */
const char* evaluate(void)
{
	if(error_flag || fail_flag)
		return ESP8266_AT_ERROR;
	return ESP8266_AT_OK;
}
