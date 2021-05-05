/*
 * test.h
 *
 *  Created on: Apr 29, 2021
 *      Author: jorre
 */

#ifndef INC_TEST_H_
#define INC_TEST_H_

#include <stdbool.h>
#include "stdint.h"
#include "main.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>

void Test_program(void);
void Test_wifi_connect(void);
void Test_connection(void);
void Test_sendData(void);

#endif /* INC_TEST_H_ */
