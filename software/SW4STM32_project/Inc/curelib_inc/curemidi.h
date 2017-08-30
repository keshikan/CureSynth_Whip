/*
 *******************************************************************************
 *  [curemidi.h]
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#ifndef _CUREMIDI_H_
#define _CUREMIDI_H_


#define MIDI_BUFFER_LENGTH (1024)
#define MIDI_DATABYTE_MAX (32)

#include <stdlib.h>
#include <stdint.h>

#include "./curelib_inc/curesynth.h"
#include "./curelib_inc/curebuffer.h"
#include "./curelib_inc/curemisc.h"

//debug
#include "curelib_inc/curedbg.h"

#define GM1_EXCEPT(n) if(MODE_GM1 != synth_settings.mode){(n);}

//todo: for inline
//#define cureMidiBufferEnqueue(f1) ( cureRingBufferU8Enqueue(&rxbuf, f1) )
//#define cureMidiBufferDequeue() ( cureRingBufferU8Dequeue(&rxbuf, &midi_buf) )

////public typedef////
typedef enum{
	START_ANALYSIS,    // Initial Status, including exception.
	WAIT_DATA1,        // Waiting data byte(1st byte)
	WAIT_DATA2,        // Waiting data byte(2nd byte)
	WAIT_SYSTEM_DATA,  // Waiting data byte(system exclusive)
	END_ANALYSIS       // Analysis is ended.
}AnalysisStatus;

typedef enum{
	EXCC_START, EXCC_PARAM, EXCC_DAT
}ExtendCCStatus;

typedef enum{
	MSG_NOTHING,    // Exception(can't resolved, missing data, etc.)
	MSG_NOTE_ON,    // Note-on message
	MSG_NOTE_OFF,   // Note-off message
	MSG_PITCH,      // PitchBend message
	MSG_SYSEX,      // System Exclusive message
	MSG_CC,         // Control Change message
	MSG_PROG,       // Program Change message
	MSG_RPN,        // RPN message
	MSG_NRPN        // NRPN message
}EventType;

typedef struct{
	EventType type;
	uint8_t channel;
	uint8_t data_byte[MIDI_DATABYTE_MAX]; //data_byte[0]=MSB, [1]=LSB, [2]=OTHER...(e.g. sysEx, Control Change...)
}MIDIEvent;

typedef struct{
	AnalysisStatus stat;
	EventType type;
	uint8_t channel;
	uint8_t data_idx;
}MidiAnalysisStatus;

typedef struct{
	ExtendCCStatus stat;
	uint8_t flag; // RPN Received flag. half lower byte means :param_msb, param_lsb, dataentry_msb, dataentry_lsb (e.g. 0b00000010: dataentry_msb is received)
	uint8_t param_msb, param_lsb;
	uint8_t dataentry_msb, dataentry_lsb;
}ExtendCCEvent;

typedef enum{
 OPERATOR_OFF = 255
}OperatorStatus;

typedef struct{
	OperatorStatus stat[128];//status of note number (is ON or OFF)
}MidiNoteOnStatus;

typedef struct{
	NoteStatus ison[OPNUM];
}MidiOperatorOnStatus;

typedef struct{
	MidiNoteOnStatus operator_to_note;//status of ch1 to 16
	MidiOperatorOnStatus operator_to_channel;//status of ch1 to 16
	Operator operatorsettings;
}MidiChannel;

typedef struct{
	MidiChannel channel[16];
	uint8_t front_opnum;	//A operator number to use when it received Note-on message.
}MidiMasterChannel;

typedef struct{

	NoteType ntype;
	WaveType wtype;

	uint8_t ringmod_multiply;
	uint8_t ringmod_gain;

	PBSweepType pitch_sweep_type;
	uint8_t pitch_sweep_spd;

	uint8_t attack;
	uint8_t decay;
	uint8_t sustainLevel;
	uint8_t sustainRate;
	uint8_t release;

	uint8_t notenum;// Drum Only
	uint8_t out_gain;

}MidiPatch;


////public variables////
extern MidiMasterChannel master;
extern RingBufferU8 rxbuf;
extern uint8_t midi_buf;


////public func////
extern void cureMidiMain();
extern FUNC_STATUS cureMidiInit();
extern BUFFER_STATUS cureMidiBufferEnqueue(uint8_t* inputc);
extern BUFFER_STATUS cureMidiBufferDequeue();
extern bool cureMidiBufferIsEmpty();


#endif /* CUREMIDI_H_ */
