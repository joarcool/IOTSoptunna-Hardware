/*
 * @brief	Header file for functions for ultrasonic sensor
 * @file	ultrasonic.h
 * @author	Axel Ström
 * @date	05/05/21
 * */

#include "stdio.h"
#include "stdlib.h"

void microDelay(uint32_t uSec);
void ultrasonic_reset(void);
void ultrasonic_pulse(void);
void ultrasonic_program(void);
int ultrasonic_checkDist(float measuredDist, float newDist);
float ultrasonic_mesuareDist(void);

