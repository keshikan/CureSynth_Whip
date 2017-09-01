/*
 *******************************************************************************
 *  [curedisplay.h]
 *  This module is making display pattern for SH1103 OLED module (using I2C mode) .
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#ifndef CUREDISPLAY_H_
#define CUREDISPLAY_H_

#include"./curelib_Inc/curemisc.h"

//table
extern uint8_t disp_settings[];

extern uint8_t disp_memory[1048][2];
extern bool disp_transmitted;

//display memory
#define DISP_INITSIZE (16)
#define DISP_MEMORY_SIZE (1025)

//display appearance
#define DISP_PEAK_DOWN_SPEED (4) //set to 1-15
#define DISP_DOT_WIDTH (7)
#define DISP_BAR_MULTIPLY (3.5f)

//public func
extern void displayInit();
extern void displaySetData(uint8_t *dat);


#endif /* CUREDISPLAY_H_ */

