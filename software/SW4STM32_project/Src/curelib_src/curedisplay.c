/*
 *******************************************************************************
 *  [curedisplay.c]
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#include <stdio.h>

#include "curelib_inc/curedisplay.h"
#include "curelib_inc/curemidi.h"
#include "curelib_inc/curemisc.h"


uint8_t disp_memory[1048][2];
bool disp_transmitted=true;

uint8_t peak_data[16];

//table
const uint8_t disp_pat[4] = {0x00, 0x07, 0x70, 0x77};


uint8_t disp_settings[] = {
		0x00,	//set command byte (following data is all for command.)
		0xA8, 0x3F,// set multiplex ratio to default
		0xD3, 0x00 , //set offset to 0
		//0x2E, //deactivate scroll ONLYSSD1306
		0xA1, // set remap  to
		0xC0, // set com scan direction
		//0xDA, 0x22, // set com pin conf
		0x81, 0xFF, // set contrast
		0xA4, //set display on
		0xA6, // set normal
		0xD5, 0x80, // set freq
		//0x20, 0x00, // set display mode to horizonal addressing mode. ONLYSSD1306
		//0x21, 0x00, 0x7F,
		//0x22, 0x00, 0x07,
		0x40, // set start line to 0
		//0x8D, 0x14, //Charge Pump setting
		0x30, //SH1106 charge pump setting
		0xAF
};

void displayInit()
{

	uint32_t i,j;

	for (i=0; i<8; i++) {
		for (j=0; j<3; j++) {
			disp_memory[i * 131 + j][0] = 0x80;
		}

		for (j=3; j<131; j++) {
			disp_memory[i * 131 + j][0] = 0xC0;
		}
	}

	for (i=0; i<8; i++) {

		disp_memory[i * 131   ][1] = 0x02;
		disp_memory[i * 131 +1][1] = 0x10;
		disp_memory[i * 131 +2][1] = 0xB0 + i;

		for (j = 3; j < 131; j++) {
			disp_memory[i * 131 + j][1] = 0x00;
		}
	}

    disp_transmitted = true;

    for(i=0; i<16; i++){
    	peak_data[i] = 0;
    }
}


void _displaySetDotGraph(uint8_t ch, uint8_t dat)
{
	uint8_t j;
	uint8_t dat_norm;
	uint8_t div, rem;

	dat_norm = dat >> 4; //normalize to 0-15

	div = (dat_norm + 1) / 2;
	rem = (dat_norm + 1) & 1;

	if(rem !=0){

		for(j=0; j<DISP_DOT_WIDTH; j++){		//x-axis
			disp_memory[3 + div *131 + ch*8 + j ][1] |= 0x07;
		}

	}else{

		for(j=0; j<DISP_DOT_WIDTH; j++){		//x-axis
			disp_memory[3 + (div - 1) *131 + ch*8 + j ][1] |= 0x70;
		}
	}

}

void _displaySetBarGraph(uint8_t ch, uint8_t dat)
{
	uint8_t i,j;
	uint8_t dat_norm;
	uint8_t div, rem;


	dat_norm = dat >> 4; //normalize to 0-15

	div = (dat_norm + 1) / 2;
	rem = (dat_norm + 1) & 1;


	for(i=0; i<div; i++){			//y-axis

		for(j=0; j<DISP_DOT_WIDTH; j++){		//x-axis
			disp_memory[3 + i*131 + ch*8 + j ][1] = 0x77;
		}
	}

	if(rem !=0){
		for(j=0; j<DISP_DOT_WIDTH; j++){		//x-axis
			disp_memory[3 + i*131 + ch*8 + j ][1] = 0x07;
		}
		i++;
	}

	for( ; i<8; i++){			//y-axis
		for(j=0; j<DISP_DOT_WIDTH; j++){		//x-axis
			disp_memory[3 + i*131 + ch*8 + j ][1] = 0x00;
		}
	}
}


void displaySetData(uint8_t *dat)
{
	uint8_t i;


//Draw bar graph.
	for(i=0; i<16; i++){
		dat[i] = __USAT(dat[i] * DISP_BAR_MULTIPLY, 8);
		_displaySetBarGraph(i, dat[i]);
	}


//Draw peak meter.
	for(i=0; i<16; i++){
		if( peak_data[i] < dat[i] )
		peak_data[i] = dat[i];
	}

	for(i=0; i<16; i++){
		_displaySetDotGraph(i, peak_data[i]);
	}

	for(i=0; i<16; i++){
		if( peak_data[i] > DISP_PEAK_DOWN_SPEED)
		peak_data[i] -= DISP_PEAK_DOWN_SPEED;
	}

}

