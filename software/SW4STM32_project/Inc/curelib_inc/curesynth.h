/*
 *******************************************************************************
 *  [curesynth.h]
 *  This module is for sound generation.
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */

#ifndef CURESYNTH_H_
#define CURESYNTH_H_


#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "./curelib_inc/curebuffer.h"
#include "./curelib_inc/curemisc.h"

#define OPNUM (36)	// a number of operators.(= Maximum polyphony)
#define CHNUM (16)	//a number of output channels.

//Sound precision settings.
#define SAMPLE_NUM (4096) 	// Sampling number of wave tables.
#define SAMPLE_NUM_HALF (2048) 	// Half of sampling number of wave tables.
#define SAMPLE_RATE (32000)	// sampling rate of DAC interrupt.
#define OVERSAMPLING_NUM (4) //todo
#define AUDIO_BIT (16)		//16bit
#define AUDIO_MAX_NUM   (32767)		//16bit
#define AUDIO_MIN_NUM  (-32768)		//16bit

//Pitchbend
#define PITCHTABLE_NUM (768)

//PitchBend Control
#define PITCH_CTR_DIV (256) //need 2^n

//ADSR
#define FADESAMPLE_BIT (6)	//sampling number for generating fade effects.
#define ADSR_SCALEFACTOR (SAMPLE_RATE / 400) //to multiply adsr time.

//Pan
#define PANTABLE_NUM (128)

//Modulation
#define MODTABLE_NUM (256)
#define MOD_PLL_DIV (3000) // The base frequency [BF] is set to (SAMPLE_RATE / MOD_PLL_DIV) / MODTABLE, and Modulation frequency set to [BF] * modulation
#define MOD_PLL_OFFSET (33) // Offset number: modulation = modulation + 33  (e.g. When modulation = 64, Modulation frequency set to [BF] * (modulation + MOD_PLL_OFFSET)
#define MOD_AMPLITUDE (0.7f)
#define MOD_GAIN_SCALEFACTOR (0.8f)

//Delay
#define DELAYBUFFER_LENGTH (32768)//need 2^n

//Filter
#define FLT_FREQ_MIN (200.0f)	 //cutoff = 0
#define FLT_FREQ_MID (12000.0f) //cutoff = 64
#define FLT_FREQ_MAX (15000.0f) //cutoff = 127
#define FLT_QFACT_MIN (0.001f)   //resonance = 0
#define FLT_QFACT_MID (0.85f)   //resonance = 64
#define FLT_QFACT_MAX (15.00f)   //resonance = 127


//Output
#define OUTPUT_DIV_BIT (2) // output / 2^(this)

#define MASTER_OUTPUT_DIV_BIT (0) // output / 2^(this)

//Distortion
#define OUTPUT_DIST_DIV_BIT (9) // output / 2^(this)


typedef enum{
	SINE, SQUARE, SAW, TRIANGLE, NOISE
}WaveType;

typedef enum{
	ADSR_START, // Operator output process has been started.
	ATTACK,     // Processing Attack.
	DECAY,      // Processing Decay.
	SUSTAIN,    // Processing Sustain.
	RELEASE,    // Processing Release.
	ADSR_END    // Operator output process has been ended.
}EnvelopeStatus;

typedef enum{
	NOTE_OFF, NOTE_ON
}NoteStatus;

typedef enum{ //todo for internal sequencer.
	C0 = 24, C0_up, D0, D0_up, E0, F0, F0_up, G0, G0_up, A0, A0_up, B0,
	C1, C1_up, D1, D1_up, E1, F1, F1_up, G1, G1_up, A1, A1_up, B1,
	C2, C2_up, D2, D2_up, E2, F2, F2_up, G2, G2_up, A2, A2_up, B2,
	C3, C3_up, D3, D3_up, E3, F3, F3_up, G3, G3_up, A3, A3_up, B3,
	C4, C4_up, D4, D4_up, E4, F4, F4_up, G4, G4_up, A4, A4_up, B4,
	C5, C5_up, D5, D5_up, E5, F5, F5_up, G5, G5_up, A5, A5_up, B5,
	C6, C6_up, D6, D6_up, E6, F6, F6_up, G6, G6_up, A6, A6_up, B6,
	C7, C7_up, D7, D7_up, E7, F7, F7_up, G7, G7_up, A7, A7_up, B7, SEQUENCER_END = 999
}Scale;

typedef enum{
	LPF, HPF
}FilterType;

typedef enum{
	SWEEP_UP, SWEEP_DOWN, SWEEP_NONE
}PBSweepType;

typedef enum{
	MODE_ORIGINAL, // Original mode. WaveType can be controlled by program change message.
	MODE_GM1,      // General MIDI Level1 compatible mode. Some parameters (including WaveType) are set by [patchlist.h].
	MODE_HYBRID,   // Hybrid (Original + GM1) mode.
	MODE_GM2,      //todo
	MODE_GS,       //todo
	MODE_XG        //todo
}MidiMode;

typedef enum{
	NOTE_MELODY, NOTE_DRUM
}NoteType;

typedef enum{
	TRACK_MELODY, TRACK_DRUM
}TrackType;

typedef enum{
	DIST_OFF, DIST_ON
}DistortionMode;

typedef struct{
	uint16_t divider;
	uint32_t idx;
	EnvelopeStatus stat;
	uint16_t attack;
	uint16_t decay;
	uint16_t sustainLevel;
	uint16_t sustainRate;
	uint16_t release;
	uint16_t out;
}EnvelopeGen;

typedef struct{
	WaveType type;

	float freq;
	float freqbuf;

	int16_t pitch;
	int16_t pitchbendsensitivity;
	float pitchScaleFactor;

	float pitch_sweep_ScaleFactor;
	uint16_t pitch_sweep_idx;
	uint16_t pitch_sweep_divider;
	uint8_t pitch_sweep_spd;
	PBSweepType pitch_sweep_type;

	float modScaleFactor;

	uint32_t pointer;
	uint32_t angular;

	int16_t out;
}WaveGen;

typedef struct{
	WaveType type;
	float multiply;
	uint32_t pointer;
	uint32_t angular;
	uint16_t gain;
	int16_t out;
}RingMod;

typedef struct{
	WaveGen wav;
	RingMod ringmod;
	EnvelopeGen env;
	NoteStatus stat;
	NoteType ntype;
	uint16_t velocity;
	uint8_t ch;	//set 255 means "NO OUTPUT"
	int16_t rightout;
	int16_t leftout;
	int32_t out;
	uint8_t out_gain;
}Operator;


typedef struct{
	uint8_t volume[CHNUM];
	uint8_t expression[CHNUM];

	uint16_t delay_time;
	uint8_t delay_feedback_gain;
	uint8_t delay_feedback_ison;

	uint8_t delay_level[CHNUM];

	uint8_t pan[CHNUM];
	uint8_t pan_ref[CHNUM];

	uint32_t modulation_pointer[CHNUM];
	uint32_t modulation_angular[CHNUM];
	uint32_t modulation_PLL_idx[CHNUM];
	float modulation_scalefactor[CHNUM];
	uint8_t modulation[CHNUM];
	uint8_t modulation_rate[CHNUM];


	uint8_t cutoff[CHNUM];
	uint8_t resonance[CHNUM];
	FilterType ftype[CHNUM];

	MidiMode mode;
	TrackType tr_type[CHNUM];

	DistortionMode dst_mode[CHNUM];
	uint8_t dst_level[CHNUM];
	uint8_t dst_gain[CHNUM];

}SynthSettings;

typedef union{
	struct mono_out{
		int16_t left;
		int16_t right;
	}mono;
	uint32_t stereo_out;
}SynthOutput;


typedef struct{
	float fc, Q;
	int16_t in1, in2, out1, out2;
	float b0a0, b1a0, b2a0, a1a0, a2a0;
}SynthFilterSettings;


//Public variables
extern Operator au_operator[OPNUM];
extern SynthOutput au_out[CHNUM];
extern SynthSettings synth_settings;
extern SynthFilterSettings filter_settings[CHNUM];
extern uint8_t display_data[CHNUM];

//Public functions
extern void cureSynthOperatorInit(Operator *op);
extern void cureSynthInit();
extern void cureSynthSettingInit();

extern void cureSynthGetOutput(int16_t output[2]);

extern void cureSynthSetNoteON(Operator *op, uint8_t sc, uint8_t vel, uint8_t midich);
extern void cureSynthSetNoteOFF(Operator *op);
extern void cureSynthOperatorApplyPitchBend(Operator *op);
extern void cureSynthSetFilter(uint8_t ch);
extern void cureSynthAllSoundOff(uint8_t ch);

extern void cureSynthOperatorCopy(Operator *src, Operator *dest);

extern void cureSynthUpdateDisplayData();


#endif /* CURESYNTH_H_ */
