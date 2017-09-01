/*
 *******************************************************************************
 *  [curedbg.h]
 *  This module is for debugging.
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#ifndef CUREDBG_H_
#define CUREDBG_H_


#include<stdlib.h>
#include<stm32f7xx_hal.h>

#include"./curelib_inc/curemisc.h"

#define CUREDBG_UART false	//UART debug mode
#define CUREDBG_CPU_USAGE false	//CPU USAGE OUTPUT (the pulse width ratio is CPU usage.)

extern UART_HandleTypeDef huart1;
void __io_putchar(uint8_t ch);

#endif /* CUREDBG_H_ */
