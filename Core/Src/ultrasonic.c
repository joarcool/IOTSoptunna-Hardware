/*
 * @brief	C file of functions for ultrasonic sensor HC-SR04
 * @file	ultrasonic.c
 * @author	Axel Ström
 * @date	05/05/21
 * */

#include "stdio.h"
#include "ultrasonic.h"
#include "main.h"
#include "gpio.h"
#include "usart.h"

#define usTIM TIM4

void microDelay(uint32_t uSec)
{
	if(uSec < 2)
	uSec = 2;
	usTIM -> ARR = uSec - 1;
	usTIM -> EGR = 1;
	usTIM -> SR &= ~1;
	usTIM -> CR1 |= 1;
	while((usTIM -> SR & 0x0001) != 1);
	usTIM -> SR &= ~(0x0001);
}

/*
 * @brief	Function that resets ultrasonic sensor just in case.
 * @file	ultrasonic.c
 * @author	Axel Ström
 * @date	11/05/21
 * */
void ultrasonic_reset(void)
{
	/* Reset trigger port */
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
	microDelay(3);
}

/*
 * @brief	Sends 10 microseconds long pulse to HC-SR04 trigger pin to activate sensor.
 * @file	ultrasonic.c
 * @author	Axel Ström
 * @date 11/05/21
 * */
void ultrasonic_pulse(void)
{
	/* Send pulse of 10 us to trigger port to activate ultrasonic pulses */
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
	microDelay(10);
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
}

/*
 * @brief	Simple algorithm that checks if the measured distance has changed +-3 cm.
 * @file	ultrasonic.c
 * @author	Axel Ström
 * @date	11/05/21
 * */
int ultrasonic_checkDist(float measuredDist, float newDist)
{

	float dist = abs(measuredDist - newDist);

	if(dist > 3)
		return 1;
	else
		return 0;
}

/*
 * @brief	Whole logic of the distance measuring program. Calculates distance from sensor to object.
 * @file	ultrasonic.c
 * @author	Axel Ström
 * @date	11/05/21
 * */
void ultrasonic_program(void)
{
	char uart_buf[50];
	float distance;
	float distanceStart;

	ultrasonic_reset();
	ultrasonic_pulse();


	/* Measure distance only once. Same as in while(1) loop */
	uint32_t ticks;
	while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET)

	ticks = 0;
	while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET)
	{
		ticks++;
		microDelay(2);
	}

	/* Calculate distance */
	distanceStart = ticks * 2.8 * (0.0343 / 2);

	/* Print to test that it works */
	sprintf(uart_buf, "Start distance (cm): %.1f!\r\n", distanceStart);
	HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);

	while (1)
	  {
		ultrasonic_reset();
		ultrasonic_pulse();

		uint32_t ticks;
		/* While echo pin is held low (reset) */
		while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET)

		ticks = 0;
		while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET)
		{
			ticks++;
			microDelay(2);
		}

		/* Calculate distance */
		distance = ticks * 2.8 * (0.0343 / 2);

		if(ultrasonic_checkDist(distanceStart, distance) == 1)
		{
			/* Print result via UART to computer */
			sprintf(uart_buf, "Distance update (cm): %.1f!\r\n", distance);
			HAL_UART_Transmit(&huart2, (uint8_t *) uart_buf, strlen(uart_buf), 100);

			distanceStart = distance;
		}

		/* Do this measurement every 5 seconds */
		HAL_Delay(5000);

	  }
}
