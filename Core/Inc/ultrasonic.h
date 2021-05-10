/*
 * @brief	Header file for functions for ultrasonic sensor
 * @file	ultrasonic.h
 * @author	Axel Ström
 * @date	05/05/21
 * */

#include "stdio.h"

void microDelay(uint32_t uSec);
void ultrasonic_reset(void);
void ultrasonic_pulse(void);
void ultrasonic_distance(uint32_t distance);
