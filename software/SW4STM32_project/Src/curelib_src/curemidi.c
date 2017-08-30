/*
 *******************************************************************************
 *  [curemidi.c]
 *
 *******************************************************************************
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */


#include "curelib_inc/curemidi.h"

#include "./curelib_inc/patchlist.h"
#include "./curelib_inc/curedbg.h"


/////const variables/////
extern const MidiPatch patch_melody[128];
extern const MidiPatch patch_drum[128];

////private variables////
MidiAnalysisStatus analyzed_status;
MIDIEvent midi_event;	//received midi data
ExtendCCEvent rpn_event;

////public variables////
MidiMasterChannel master;
RingBufferU8 rxbuf;
uint8_t midi_buf;

////private func////
void cureMidiSetNoteOn();
void cureMidiSetNoteOff();

bool cureMidiEventIsGenerated();
void cureMidiAnalyzeEvent();

void cureMidiMasterChannelInit(MidiMasterChannel* mtr);


void cureMidiMain()
{
	while( BUFFER_SUCCESS == cureMidiBufferDequeue() ){

	#if CUREDBG_UART
		//printf("[midi]:0x%x,front=%d, rear=%d, UsedArea=%d/%d\r\n", midi_buf, rxbuf.idx_front, rxbuf.idx_rear, _cureRingBufferU8GetUsedSize(&rxbuf), rxbuf.length);
		printf("[midi]:0x%x\r\n", midi_buf);
	#endif
		if( cureMidiEventIsGenerated() ){// Generate MIDI event from UART buffer.
	#if CUREDBG_UART
			printf("analyzed!\r\n");
	#endif
			//Analyze MIDI Message.
			cureMidiAnalyzeEvent();
		}
	}
}


FUNC_STATUS cureMidiInit()
{
	if( BUFFER_FAILURE == cureRingBufferU8Init(&rxbuf, MIDI_BUFFER_LENGTH) ){
		return FUNC_ERROR;
	}

	cureMidiMasterChannelInit(&master);

	cureSynthInit();

	//Initialize variables for RPN
	rpn_event.flag = 0x00;

	rpn_event.dataentry_lsb
	 = rpn_event.dataentry_msb
	 = rpn_event.param_lsb
	 = rpn_event.param_msb
	 = 0x00;

	return FUNC_SUCCESS;

}

void cureMidiChannelInit(MidiChannel* ch)
{
	uint8_t i;

	for(i=0; i<128; i++){
		ch->operator_to_note.stat[i] = OPERATOR_OFF;
	}

	for(i=0; i<OPNUM; i++){
		ch->operator_to_channel.ison[i] = false;
	}

	cureSynthOperatorInit( &(ch->operatorsettings) );

	ch->operatorsettings.wav.type = SAW;
	ch->operatorsettings.ringmod.multiply = 0.5;
	ch->operatorsettings.ringmod.gain = 100;
	ch->operatorsettings.env.release = 10;
	ch->operatorsettings.env.attack = 1;
}

void cureMidiMasterChannelInit(MidiMasterChannel* mtr)
{
	uint8_t i;

	mtr->front_opnum = 0;
	for(i=0; i<16; i++){
		cureMidiChannelInit(&(mtr->channel[i]));
	}
}


//TODO: use function macros.
BUFFER_STATUS cureMidiBufferEnqueue(uint8_t* inputc)
{
	return cureRingBufferU8Enqueue(&rxbuf, inputc);
}

BUFFER_STATUS cureMidiBufferDequeue()
{
	return cureRingBufferU8Dequeue(&rxbuf, &midi_buf);
}

bool cureMidiBufferIsEmpty()
{
	if( 0 == _cureRingBufferU8GetUsedSize(&rxbuf) ){
		return true;
	}

	return false;
}


bool cureMidiEventIsGenerated()
{
	uint8_t upper_half_byte= (midi_buf) & 0xF0;
	uint8_t lower_half_byte= (midi_buf) & 0x0F;

	if( upper_half_byte & 0x80 ){// status byte.
		if( 0xF0 == upper_half_byte ){// MIDI System Message

			switch(lower_half_byte){

			case 0x00://SysEx Start
				midi_event.type = analyzed_status.type = MSG_SYSEX;
				analyzed_status.data_idx = 0;
				analyzed_status.stat = WAIT_SYSTEM_DATA;
				break;

			case 0x07://SysEx End
				analyzed_status.stat = END_ANALYSIS;
			}

		}else{// MIDI Channel Message.

			switch(upper_half_byte){

			case 0x90://Note On Message.
				midi_event.type = analyzed_status.type = MSG_NOTE_ON;
				analyzed_status.stat = WAIT_DATA1;
				midi_event.channel = lower_half_byte;
				analyzed_status.channel = lower_half_byte;
				break;

			case 0x80://Note Off Message.
				midi_event.type = analyzed_status.type = MSG_NOTE_OFF;
				analyzed_status.stat = WAIT_DATA1;
				midi_event.channel = lower_half_byte;
				analyzed_status.channel = lower_half_byte;
				break;

			case 0xE0://Pitch Bend.
				midi_event.type = analyzed_status.type = MSG_PITCH;
				analyzed_status.stat = WAIT_DATA1;
				midi_event.channel = lower_half_byte;
				analyzed_status.channel = lower_half_byte;
				break;

			case 0xB0://Control Change
				midi_event.type = analyzed_status.type = MSG_CC;
				analyzed_status.stat = WAIT_DATA1;
				midi_event.channel = lower_half_byte;
				analyzed_status.channel = lower_half_byte;
				break;

			case 0xC0://Program Change
				midi_event.type = analyzed_status.type = MSG_PROG;
				analyzed_status.stat = WAIT_DATA1;
				midi_event.channel = lower_half_byte;
				analyzed_status.channel = lower_half_byte;
				break;

			default:
				midi_event.type = analyzed_status.type = MSG_NOTHING;
				analyzed_status.stat = START_ANALYSIS;
				break;
			}
		}
	}else{//data byte
		switch(analyzed_status.stat){

		case WAIT_DATA1:
			midi_event.data_byte[0] = (midi_buf);
			if(MSG_NOTE_ON == analyzed_status.type || MSG_NOTE_OFF == analyzed_status.type || MSG_PITCH  == analyzed_status.type || MSG_CC == analyzed_status.type ){
				analyzed_status.stat = WAIT_DATA2;
			}else if( MSG_PROG == analyzed_status.type ){
				analyzed_status.stat = END_ANALYSIS;
			}else{
				analyzed_status.stat = START_ANALYSIS;
			}
			break;

		case WAIT_DATA2:
			midi_event.data_byte[1] = (midi_buf);
			analyzed_status.stat = END_ANALYSIS;
			break;

		case WAIT_SYSTEM_DATA:
			midi_event.data_byte[analyzed_status.data_idx++] = (midi_buf);

			if(analyzed_status.data_idx > (MIDI_DATABYTE_MAX - 1) ){
				analyzed_status.stat = END_ANALYSIS;
			}
			break;

		case END_ANALYSIS://running status
			midi_event.data_byte[0] = (midi_buf);
			analyzed_status.stat = WAIT_DATA2;
			break;

		case START_ANALYSIS:
			break;

		default:
			break;
		}
	}

	if(END_ANALYSIS == analyzed_status.stat){
		return true;
	}else{
		return false;
	}

}

float cureMidiRingmodMultofloat(uint8_t dat_int)
{
	float ret;

	if(dat_int != 0){
		ret = ( (float)(dat_int & 0x7F) + 1.0f ) / 32.0f;
	}else{
		ret = ( 1.0f / 512.0f );
	}

	return ret;
}

void cureMidiSetDrumParam()
{
	master.channel[midi_event.channel].operatorsettings.wav.type 				= patch_drum[ midi_event.data_byte[0] ].wtype;
	master.channel[midi_event.channel].operatorsettings.ntype 					= patch_drum[ midi_event.data_byte[0] ].ntype;
	master.channel[midi_event.channel].operatorsettings.ringmod.multiply 		=cureMidiRingmodMultofloat(patch_drum[ midi_event.data_byte[0] ].ringmod_multiply);
	master.channel[midi_event.channel].operatorsettings.ringmod.gain 			= patch_drum[ midi_event.data_byte[0] ].ringmod_gain;
	master.channel[midi_event.channel].operatorsettings.wav.pitch_sweep_type 	= patch_drum[ midi_event.data_byte[0] ].pitch_sweep_type;
	master.channel[midi_event.channel].operatorsettings.wav.pitch_sweep_spd 	= patch_drum[ midi_event.data_byte[0] ].pitch_sweep_spd;
	master.channel[midi_event.channel].operatorsettings.env.attack 				= patch_drum[ midi_event.data_byte[0] ].attack * 2;
	master.channel[midi_event.channel].operatorsettings.env.decay 				= patch_drum[ midi_event.data_byte[0] ].decay * 2;
	master.channel[midi_event.channel].operatorsettings.env.sustainLevel 		= patch_drum[ midi_event.data_byte[0] ].sustainLevel * 2;
	master.channel[midi_event.channel].operatorsettings.env.sustainRate 		= patch_drum[ midi_event.data_byte[0] ].sustainRate * 2;
	master.channel[midi_event.channel].operatorsettings.env.release 			= patch_drum[ midi_event.data_byte[0] ].release * 2;
	master.channel[midi_event.channel].operatorsettings.out_gain	 			= patch_drum[ midi_event.data_byte[0] ].out_gain;

	cureSynthOperatorCopy(&master.channel[midi_event.channel].operatorsettings, &au_operator[master.front_opnum]);

}

void cureMidiSetNoteOn()
{
	uint8_t i;

	// Judge the note is ON or OFF. Ignore Note-on message if notes are already ON
	if( OPERATOR_OFF != master.channel[midi_event.channel].operator_to_note.stat[midi_event.data_byte[0]] ){
		return;
	}

	//search unused operator.
	if( ADSR_END != au_operator[master.front_opnum].env.stat ){
		for(i=0; i<OPNUM; i++){
			if( ADSR_END == au_operator[i].env.stat ){
				master.front_opnum = i;
				break;
			}
		}
	}


	//set note on
	master.channel[midi_event.channel].operator_to_note.stat[midi_event.data_byte[0]] = master.front_opnum; //set operator number to note number structure.
	master.channel[midi_event.channel].operator_to_channel.ison[master.front_opnum] = true;//set operator number to channel structure.



	if( TRACK_MELODY == synth_settings.tr_type[midi_event.channel] ){//If sound is melody track.
		cureSynthOperatorCopy(&master.channel[midi_event.channel].operatorsettings, &au_operator[master.front_opnum]);
		cureSynthSetNoteON(&au_operator[master.front_opnum], midi_event.data_byte[0], midi_event.data_byte[1], midi_event.channel);
	}else{//If sound is drum track.
		cureMidiSetDrumParam();
		cureSynthSetNoteON(&au_operator[master.front_opnum], patch_drum[ midi_event.data_byte[0] ].notenum, midi_event.data_byte[1], midi_event.channel);
	}

	master.front_opnum = (master.front_opnum + 1) % OPNUM;
}

void cureMidiSetNoteOff()
{
	OperatorStatus *opstat;

	opstat = &(master.channel[midi_event.channel].operator_to_note.stat[midi_event.data_byte[0]]) ;

	if( OPERATOR_OFF != *opstat ){
		cureSynthSetNoteOFF(&au_operator[*opstat]);
		master.channel[midi_event.channel].operator_to_channel.ison[*opstat] = false;
		*opstat = OPERATOR_OFF;
	}else{
		return;
	}
}

void cureMidiSetPitchBend()
{
	uint16_t lsb = midi_event.data_byte[0];
	uint16_t msb = midi_event.data_byte[1];
	uint16_t pitchbend = ( ( (msb) & 0x7F) << 7) + (lsb);
	uint8_t i;

	//for pitchbend
	master.channel[midi_event.channel].operatorsettings.wav.pitch = pitchbend;

	//Calculate PitchBend
	cureSynthOperatorApplyPitchBend( &(master.channel[midi_event.channel].operatorsettings) );

	for(i=0; i<OPNUM; i++){
		if( true == master.channel[midi_event.channel].operator_to_channel.ison[i] ){
			au_operator[i].wav.pitch = pitchbend;
			au_operator[i].wav.pitchScaleFactor = master.channel[midi_event.channel].operatorsettings.wav.pitchScaleFactor;
		}
	}

}

void cureMidiSetWaveType(uint8_t ch, uint8_t typ)
{
	switch(typ){
		case 0:
			master.channel[ch].operatorsettings.wav.type = SINE;
			break;
		case 1:
			master.channel[ch].operatorsettings.wav.type = SQUARE;
			break;
		case 2:
			master.channel[ch].operatorsettings.wav.type = TRIANGLE;
			break;
		case 3:
			master.channel[ch].operatorsettings.wav.type = SAW;
			break;
		case 4:
			master.channel[ch].operatorsettings.wav.type = NOISE;
			break;
		default:
			break;
	}
}

void cureMidiSetSweepType(uint8_t ch, uint8_t typ)
{
	switch(typ){
		case 0:
			master.channel[ch].operatorsettings.wav.pitch_sweep_type = SWEEP_UP;
			break;
		case 1:
			master.channel[ch].operatorsettings.wav.pitch_sweep_type = SWEEP_DOWN;
			break;
		case 2:
			master.channel[ch].operatorsettings.wav.pitch_sweep_type = SWEEP_NONE;
			break;
		default:
			break;
	}
}

void cureMidiSetDistortionType(uint8_t ch, uint8_t typ)
{
	switch(typ){
		case 0:
			synth_settings.dst_mode[ch] = DIST_OFF;
			break;
		case 1:
			synth_settings.dst_mode[ch] = DIST_ON;
			break;
		default:
			break;
	}
}

//TODO:Now under construction.
void cureMidiSetRPN()
{
	switch(midi_event.data_byte[0]){

		case 100:
			rpn_event.param_lsb = (midi_event.data_byte[1] & 0x7F);
			if( (0b0011 & rpn_event.flag) ){
				rpn_event.flag = 0b0000;
			}
			rpn_event.flag |= 0b0100;
			break;
		case 101:
			rpn_event.param_msb = (midi_event.data_byte[1] & 0x7F);
			if( (0b0011 & rpn_event.flag) ){
				rpn_event.flag = 0b0000;
			}
			rpn_event.flag |= 0b1000;
			break;
		case 6:
			rpn_event.dataentry_msb = (midi_event.data_byte[1] & 0x7F);
			rpn_event.flag |= 0b0010;
			break;
		case 38:
			rpn_event.dataentry_lsb = (midi_event.data_byte[1] & 0x7F);
			rpn_event.flag |= 0b0001;
			break;
		default:
			break;
	}

//	if( false != (  (0b0011 & rpn_event.flag) && (0b0011 & rpn_event.flag)  ) ){
//		rpn_event.flag = 0b0000;
//	}

	if(0b1110 == rpn_event.flag){
		if( (0 == rpn_event.param_msb) && (0 == rpn_event.param_msb) ){
			//set pitchbend sensitivity
			master.channel[midi_event.channel].operatorsettings.wav.pitchbendsensitivity = midi_event.data_byte[1] & 0x7F;
			rpn_event.flag = 0b0000;
		}
	}else if(0b1111 == rpn_event.flag){
		rpn_event.flag = 0b0000;
	}


}

void cureMidiSetControlChange()
{
	uint8_t i;

	for(i=0; i<OPNUM; i++){

		switch(midi_event.data_byte[0]){
			case 1://Modulation
				synth_settings.modulation[midi_event.channel] = (midi_event.data_byte[1] & 0x7F);
				break;
			case 7://Volume
				synth_settings.volume[midi_event.channel] = (midi_event.data_byte[1] & 0x7F);
				break;
			case 10://Pan
				synth_settings.pan_ref[midi_event.channel] = (midi_event.data_byte[1] & 0x7F);
				break;
			case 11://Expression
				synth_settings.expression[midi_event.channel] = (midi_event.data_byte[1] & 0x7F);
				break;
			case 20://Cure Wave Type
				cureMidiSetWaveType( midi_event.channel, (midi_event.data_byte[1] & 0x7F) );
				break;
			case 23://Cure Output Gain
				master.channel[midi_event.channel].operatorsettings.out_gain = (midi_event.data_byte[1] & 0x7F);
				break;
			case 25 ://Cure Distortion Type
				cureMidiSetDistortionType( midi_event.channel, (midi_event.data_byte[1] & 0x7F) );
				break;
			case 26 ://Cure Distortion Type
				synth_settings.dst_level[midi_event.channel] = (midi_event.data_byte[1] & 0x7F);
				break;
			case 27 ://Cure Distortion Type
				synth_settings.dst_gain[midi_event.channel] = (midi_event.data_byte[1] & 0x7F);
				break;
			case 71 ://Resonance
				synth_settings.resonance[midi_event.channel] = (midi_event.data_byte[1] & 0x7F);
				cureSynthSetFilter(midi_event.channel);
				break;
			case 72 ://Release time
				GM1_EXCEPT(master.channel[midi_event.channel].operatorsettings.env.release = (midi_event.data_byte[1] & 0x7F) * 2);
				break;
			case 73 ://Attack time
				GM1_EXCEPT(master.channel[midi_event.channel].operatorsettings.env.attack = (midi_event.data_byte[1] & 0x7F) * 2);
				break;
			case 74 ://Cut off
				synth_settings.cutoff[midi_event.channel] = (midi_event.data_byte[1] & 0x7F);
				cureSynthSetFilter(midi_event.channel);
				break;
			case 75 ://Decay time
				GM1_EXCEPT(master.channel[midi_event.channel].operatorsettings.env.decay = (midi_event.data_byte[1] & 0x7F) * 2);
				break;
			case 76 ://Modulation Rate
				synth_settings.modulation_rate[midi_event.channel] = (midi_event.data_byte[1] & 0x7F);
				break;
			case 94 ://Delay
				synth_settings.delay_level[midi_event.channel] = (midi_event.data_byte[1] & 0x7F) * 2;
				break;
			case 102 ://SustainRate(original)
				GM1_EXCEPT(master.channel[midi_event.channel].operatorsettings.env.sustainRate = (midi_event.data_byte[1] & 0x7F) * 2);
				break;
			case 103 ://SustainLevel(original)
				GM1_EXCEPT(master.channel[midi_event.channel].operatorsettings.env.sustainLevel = (midi_event.data_byte[1] & 0x7F) * 2);
				break;
			case 104 ://RingModulator Multiply(original)
				GM1_EXCEPT(master.channel[midi_event.channel].operatorsettings.ringmod.multiply = au_operator[i].ringmod.multiply = cureMidiRingmodMultofloat(midi_event.data_byte[1]) );
				break;
			case 105 ://RingModulator Gain(original)
				GM1_EXCEPT(master.channel[midi_event.channel].operatorsettings.ringmod.gain = au_operator[i].ringmod.gain = (midi_event.data_byte[1] & 0x7F));
				break;
			case 108 ://Sweep type(original)
				GM1_EXCEPT(cureMidiSetSweepType( midi_event.channel, (midi_event.data_byte[1] & 0x7F) ));
				break;
			case 109 ://Sweep speed(original)
				GM1_EXCEPT(master.channel[midi_event.channel].operatorsettings.wav.pitch_sweep_spd  = (midi_event.data_byte[1] & 0x7F));
				break;
			case 120 ://All Sound Off
				cureSynthAllSoundOff(midi_event.channel);
				break;
			default:
				break;
		}
	}

	//resolve RPN message
	//TODO:Now under construction.
	cureMidiSetRPN();

}

void cureMidiSetProgramChange()
{
	uint8_t i;

	if( MODE_ORIGINAL == synth_settings.mode ){
		for(i=0; i<OPNUM; i++){
			if( false == master.channel[midi_event.channel].operator_to_channel.ison[i] ){ // if operator is not processing.

				switch(midi_event.data_byte[0]){
					case 0://SINE
						master.channel[midi_event.channel].operatorsettings.wav.type = SINE;
						break;
					case 1://Volume
						master.channel[midi_event.channel].operatorsettings.wav.type = SQUARE;
						break;
					case 2://Volume
						master.channel[midi_event.channel].operatorsettings.wav.type = TRIANGLE;
						break;
					case 3://Volume
						master.channel[midi_event.channel].operatorsettings.wav.type = SAW;
						break;
					case 4://Volume
						master.channel[midi_event.channel].operatorsettings.wav.type = NOISE;
						break;
				}

			}
		}
	}else if( (MODE_GM1 == synth_settings.mode) || (MODE_HYBRID == synth_settings.mode) ){

			master.channel[midi_event.channel].operatorsettings.wav.type 				= patch_melody[ midi_event.data_byte[0] ].wtype;
			master.channel[midi_event.channel].operatorsettings.ntype 					= patch_melody[ midi_event.data_byte[0] ].ntype;
			master.channel[midi_event.channel].operatorsettings.ringmod.multiply 		= cureMidiRingmodMultofloat(patch_melody[ midi_event.data_byte[0] ].ringmod_multiply);
			master.channel[midi_event.channel].operatorsettings.ringmod.gain 			= patch_melody[ midi_event.data_byte[0] ].ringmod_gain;
			master.channel[midi_event.channel].operatorsettings.wav.pitch_sweep_type 	= patch_melody[ midi_event.data_byte[0] ].pitch_sweep_type;
			master.channel[midi_event.channel].operatorsettings.wav.pitch_sweep_spd 	= patch_melody[ midi_event.data_byte[0] ].pitch_sweep_spd;
			master.channel[midi_event.channel].operatorsettings.env.attack 				= patch_melody[ midi_event.data_byte[0] ].attack * 2;
			master.channel[midi_event.channel].operatorsettings.env.decay 				= patch_melody[ midi_event.data_byte[0] ].decay * 2;
			master.channel[midi_event.channel].operatorsettings.env.sustainLevel 		= patch_melody[ midi_event.data_byte[0] ].sustainLevel * 2;
			master.channel[midi_event.channel].operatorsettings.env.sustainRate 		= patch_melody[ midi_event.data_byte[0] ].sustainRate * 2;
			master.channel[midi_event.channel].operatorsettings.env.release 			= patch_melody[ midi_event.data_byte[0] ].release * 2;
			master.channel[midi_event.channel].operatorsettings.out_gain	 			= patch_melody[ midi_event.data_byte[0] ].out_gain;
	}

}

FUNC_STATUS cureMidiCompareSysEx(const uint8_t * dat, uint8_t num)
{
	uint32_t i;

	if(num != analyzed_status.data_idx){
		return FUNC_ERROR;
	}

	for(i=0; i<num; i++){
		if( dat[i] != midi_event.data_byte[i]){
			return FUNC_ERROR;
		}
	}

	return FUNC_SUCCESS;

}

//TODO:Now under construction.
void cureMidiSetSystemExclusive()
{
	uint32_t i;

	const uint8_t gm_system_on[] = {0x7E, 0x7F, 0x09, 0x01};
	const uint8_t original_mode[] = {0x7D, 0x01, 0x02, 0x03, 0x04};
	const uint8_t hybrid_mode[] = {0x7D, 0x01, 0x02, 0x03, 0x05};
//	const uint8_t drum_track_melody[] = {0x41,0x10,0x42,0x12,0x40,0x1A,0x15,0x00,0x11};
//	const uint8_t drum_track_on_1[] = {0x41,0x10,0x42,0x12,0x40,0x1A,0x15,0x01,0x10};
//	const uint8_t drum_track_on_2[] = {0x41,0x10,0x42,0x12,0x40,0x1A,0x15,0x02,0x0F};

	if( FUNC_SUCCESS == cureMidiCompareSysEx(gm_system_on, sizeof(gm_system_on)) ){

		//Initialize
		for(i=0; i<CHNUM; i++){
			cureSynthOperatorInit( &master.channel[i].operatorsettings );
		}
		for(i=0; i<OPNUM; i++){
			cureSynthOperatorInit(&au_operator[i]);
		}
		cureSynthSettingInit();

		//set gm system on
		synth_settings.mode = MODE_GM1;
		return;

	}else if(FUNC_SUCCESS == cureMidiCompareSysEx(original_mode, sizeof(original_mode))){

		//Initialize
		for(i=0; i<CHNUM; i++){
			cureSynthOperatorInit( &master.channel[i].operatorsettings );
		}
		for(i=0; i<OPNUM; i++){
			cureSynthOperatorInit(&au_operator[i]);
		}
		cureSynthSettingInit();

		//set original system on
		synth_settings.mode = MODE_ORIGINAL;
		return;

	}else if(FUNC_SUCCESS == cureMidiCompareSysEx(hybrid_mode, sizeof(hybrid_mode))){

		//Initialize
		for(i=0; i<CHNUM; i++){
			cureSynthOperatorInit( &master.channel[i].operatorsettings );
		}
		for(i=0; i<OPNUM; i++){
			cureSynthOperatorInit(&au_operator[i]);
		}
		cureSynthSettingInit();

		//set hybrid system on
		synth_settings.mode = MODE_HYBRID;
		return;

	}

}


void cureMidiAnalyzeEvent()
{
	switch(midi_event.type){

		case MSG_NOTE_ON:
			cureMidiSetNoteOn();
			break;

		case MSG_NOTE_OFF:
			cureMidiSetNoteOff();
			break;

		case MSG_PITCH:
			cureMidiSetPitchBend();
			break;

		case MSG_CC:
			cureMidiSetControlChange();
			break;

		case MSG_PROG:
			cureMidiSetProgramChange();
			break;

		case MSG_SYSEX:
			cureMidiSetSystemExclusive();
			break;

		default:
			break;
	}
}

