/*
 * test.c
 *
 *  Created on: Apr 29, 2021
 *      Author: jorre
 */

#include "test.h"
#include "main.h"
#include "ESP8266.h"
#include "usart.h"
#include "gpio.h"
#include "networkinfo.h"
#include "ultrasonic.h"

void Test_program()
{
	//Test_wifi_connect();
	//Test_connection();
	//Test_sendData();
	Test_ultrasonic();
}

void Test_wifi_connect()
{
	/*char send_mode[] = "AT+CIPSEND=12\r\n";

	HAL_UART_Transmit(&huart4, (uint8_t*) send_mode, strlen(send_mode), 100);

	esp8266_send_data("hej");*/

	esp8266_initialize();
	return;
}

void Test_connection()
{
	/*eps8266_connection();
	return;*/
	char tcp_connect[30];
	sprintf(tcp_connect, "%s\"%s\",\"%s\",%s\r\n", ESP8266_AT_START, connection_type, remote_ip, remote_port);
	//HAL_UART_Transmit(&huart2, (uint8_t *) tcp_connect, strlen(tcp_connect), 100); //For testing
	esp8266_sendCommand(tcp_connect);

	char send_mode[] = "AT+CIPSEND=7\r\n";
	esp8266_sendCommand(send_mode);
	HAL_UART_Transmit(&huart4, (uint8_t*) "hej\r\n", strlen("hej\r\n"), 100);

	char send_stop[] = "AT+CIPCLOSE=0\r\n";
	esp8266_sendCommand(send_stop);
	return;
}

void Test_sendData()
{
	char send_mode[] = "AT+CIPSEND=7\r\n";

	esp8266_sendCommand(send_mode);

	HAL_UART_Transmit(&huart4, (uint8_t*) "hej\r\n", strlen("hej\r\n"), 100);
}

void Test_ultrasonic()
{
	ultrasonic_program();
}
