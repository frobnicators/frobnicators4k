#include "music.h"
#include "mmsystem.h"
#include "mmreg.h"

/* Define to have realtime playback, costs ~10 bytes extra */
#define USE_SOUND_THREAD

/* Song information */
#include "4klang.h"

/* Always stereo */
static SAMPLE_TYPE	sound_buffer[MAX_SAMPLES*2];  
static HWAVEOUT	hWaveOut;

/* initialized data */
#pragma data_seg(".wavefmt")
static WAVEFORMATEX waveFMT = {
#ifdef FLOAT_32BIT	
	WAVE_FORMAT_IEEE_FLOAT,
#else
	WAVE_FORMAT_PCM,
#endif	
    2, // channels
    SAMPLE_RATE, // samples per sec
    SAMPLE_RATE*sizeof(SAMPLE_TYPE)*2, // bytes per sec
    sizeof(SAMPLE_TYPE)*2, // block alignment;
    sizeof(SAMPLE_TYPE)*8, // bits per sample
    0 // extension not needed
};

#pragma data_seg(".wavehdr")
static WAVEHDR waveHDR = {
	(LPSTR)sound_buffer, 
	MAX_SAMPLES*sizeof(SAMPLE_TYPE)*2,			// MAX_SAMPLES*sizeof(float)*2(stereo)
	0, 
	0, 
	0, 
	0, 
	0, 
	0
};

MMTIME MMTime = { 
	TIME_SAMPLES,
	0
};

void init_music() {
#ifdef USE_SOUND_THREAD
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, sound_buffer, 0, 0);
#else
	_4klang_render(sound_buffer);
#endif
	waveOutOpen			( &hWaveOut, WAVE_MAPPER, &waveFMT, (DWORD_PTR)NULL, 0, CALLBACK_NULL );
	waveOutPrepareHeader( hWaveOut, &waveHDR, sizeof(waveHDR) );
	waveOutWrite		( hWaveOut, &waveHDR, sizeof(waveHDR) );	

}

const DWORD * music_time() {
	waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
	return &(MMTime.u.sample);
}

#include <stdio.h>

float envelope(int instrument, int voice, INT32 time_offset) {
	INT64 time = (INT64)*music_time() + time_offset;
	if(time < SAMPLE_RATE ) time = SAMPLE_RATE;
	return (&_4klang_envelope_buffer)[(( time >> 8) << 5) + 2*instrument+voice];
}

