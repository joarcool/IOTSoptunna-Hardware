/**
 * @brief program.c contains the main program for the smarbin module.
 * @details
 * @file program.c
 * @author  Joar Edling, joaredl@kth.se , Axel Ström, axst@kth.se
 * @date 06-05-2021
 * @version 1
**/
#include "program.h"


void program_main()
{
	char uart_buf[50];
	float distance;
	float distanceStart;
	int failedToSend = 0;

	/* Initialize ESP8266 and print via UART to computer*/
	sprintf(uart_buf, "Initializing ESP8266...\r\n");
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);

	int goodInit = esp8266_initialize();

	if(goodInit)
	{
		sprintf(uart_buf, "Initialization of ESP8266 OK!\r\n\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);

		distanceStart = ultrasonic_mesuareDist();

		/* Print the start distance */
		sprintf(uart_buf, "Start distance (cm): %.1f!\r\n", distanceStart);
		HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
		if(esp8266_sendDistance(distanceStart) == 0)
		{
			sprintf(uart_buf, "Could not update website!\r\n\r\n");
			HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
			failedToSend++;
		}

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
					if(esp8266_sendDistance(distance) == 0)
					{
						sprintf(uart_buf, "Could not update website!\r\n\r\n");
						HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
						failedToSend++;
					}
					distanceStart = distance;
				}
			}
			if(failedToSend >= 5)
			{
				sprintf(uart_buf, "Could not update website to many times, shutting down program...\r\n");
				HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
				break;
			}

			/* Do this measurement every 5 seconds */
			HAL_Delay(5000);

		}
	}
	else
	{
		sprintf(uart_buf, "Initialization of ESP8266 FAILED!\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
		sprintf(uart_buf, "Pls check the userconfig.h file and see if all the information is correct\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
		sprintf(uart_buf, "Shutting down program...\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);
	}
	return;
}
