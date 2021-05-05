/*
 * test.c
 *
 *  Created on: Apr 29, 2021
 *      Author: jorre
 */

#include "test.h"
#include "ESP8266.h"
#include "usart.h"
#include "gpio.h"
/*static uint8_t rx_variable;
static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_buffer_index = 0;
static bool error_flag = false;
static bool fail_flag = false;*/

void Test_program()
{
	Test_wifi_connect();
	Test_connection();
	Test_sendData();
}

void Test_wifi_connect()
{
	/*char send_mode[] = "AT+CIPSEND=12\r\n";

	HAL_UART_Transmit(&huart4, (uint8_t*) send_mode, strlen(send_mode), 100);

	esp8266_send_data("hej");*/

	esp8266_initialize();
}

void Test_connection()
{
	eps8266_connection();
}

void Test_sendData()
{
	char send_mode[] = "AT+CIPSEND=12\r\n";

	HAL_UART_Transmit(&huart4, (uint8_t*) send_mode, strlen(send_mode), 100);

	//HAL_UART_Transmit(&huart2, (uint8_t *) set_ESP8266_mode, strlen(set_ESP8266_mode), 100);
	//HAL_UART_Transmit(&huart4, (uint8_t*) "hej", strlen("hej"), 100);
}
