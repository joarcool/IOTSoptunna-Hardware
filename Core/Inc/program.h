/**
 * @brief program.h contains all the includes and defines of fungctions smartbin program.
 * @details
 * @file program.h
 * @author  Joar Edling, joaredl@kth.se , Axel Ström, axst@kth.se
 * @date 06-05-2021
 * @version 1
**/

#ifndef INC_PROGRAM_H_
#define INC_PROGRAM_H_

#include <stdbool.h>
#include "stdint.h"
#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "ESP8266.h"
#include "networkinfo.h"
#include "ultrasonic.h"
#include <string.h>

void program_main(void);

#endif /* INC_PROGRAM_H_ */
