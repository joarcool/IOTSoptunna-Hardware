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

void ultrasonic_reset(void)
{
	/* Reset trigger port */
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
	microDelay(3);
}

void ultrasonic_pulse(void)
{
	/* Send pulse of 10 us to trigger port to activate ultrasonic pulses */
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
	microDelay(10);
	HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
}

void ultrasonic_distance(uint32_t distance)
{
	uint32_t ticks;

	/* While echo pin is held low (reset) */
	while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_RESET);

	/* While echo pin is held high (set) count no. of ticks during a set period of time */
	ticks = 0;
	while(HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET)
	{
		ticks++;
		microDelay(2);
	}

	/* Calculate distance */
	distance = ticks * 2.8 * (0.0343 / 2);
}
