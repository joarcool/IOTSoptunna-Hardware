/*
 * program.c
 *
 *  Created on: May 17, 2021
 *      Author: jorre
 */
#include "program.h"


void program_main()
{
	char uart_buf[50];
	float distance;
	float distanceStart;

	/* Initialize ESP8266 and print via UART to computer*/
	sprintf(uart_buf, "Initialising ESP8266...\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);

	esp8266_initialize();

	sprintf(uart_buf, "Initialisation of ESP8266 OK!\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);


	distanceStart = ultrasonic_mesuareDist();

	/* Print the start distance */
	sprintf(uart_buf, "Start distance (cm): %.1f!\r\n", distanceStart);
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
	esp8266_sendDistance(distanceStart);

	while (1)
	{
		/* Mesuare distance */
		distance = ultrasonic_mesuareDist();

		/* Check if the distance have changed compared to the "start" distance" */
		if(ultrasonic_checkDist(distanceStart, distance))
		{
			/* A loop that checks if it was not a temporary change (like a hand in the way) */
			char send = 1;
			for(int i = 0; i < 10; i++)
			{
				distance = ultrasonic_mesuareDist();
				if(ultrasonic_checkDist(distanceStart, distance) == 0)
				{
					send = 0;
					break;
				}
				HAL_Delay(1000);
			}

			/*If it was not temporary then send the result to computer and website */
			if(send == 1)
			{
				/* Print result via UART to computer */
				sprintf(uart_buf, "Distance update (cm): %.1f!\r\n", distance);
				HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
				esp8266_sendDistance(distance);
				distanceStart = distance;
			}
		}

		/* Do this measurement every 5 seconds */
		HAL_Delay(5000);

	  }
}
