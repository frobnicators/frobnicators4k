#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include "windows.h"
#include "mmsystem.h"
#include "mmreg.h"

// define this if you have a multicore cpu and can spare ~10 bytes for realtime playback
// undef for sound precalc
//#define USE_SOUND_THREAD

////////////////////////////////////////////////
// sound
////////////////////////////////////////////////

// some song information
#include "4klang.h"

// MAX_SAMPLES gives you the number of samples for the whole song. we always produce stereo samples, so times 2 for the buffer
SAMPLE_TYPE	lpSoundBuffer[MAX_SAMPLES*2];  
HWAVEOUT	hWaveOut;

/////////////////////////////////////////////////////////////////////////////////
// initialized data
/////////////////////////////////////////////////////////////////////////////////

#pragma data_seg(".wavefmt")
WAVEFORMATEX WaveFMT =
{
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
WAVEHDR WaveHDR = 
{
	(LPSTR)lpSoundBuffer, 
	MAX_SAMPLES*sizeof(SAMPLE_TYPE)*2,			// MAX_SAMPLES*sizeof(float)*2(stereo)
	0, 
	0, 
	0, 
	0, 
	0, 
	0
};

MMTIME MMTime = 
{ 
	TIME_SAMPLES,
	0
};

/////////////////////////////////////////////////////////////////////////////////
// crt emulation
/////////////////////////////////////////////////////////////////////////////////

extern "C" 
{
	int _fltused = 1;
}

/////////////////////////////////////////////////////////////////////////////////
// Initialization
/////////////////////////////////////////////////////////////////////////////////

#pragma code_seg(".initsnd")
void  InitSound()
{
#ifdef USE_SOUND_THREAD
	// thx to xTr1m/blu-flame for providing a smarter and smaller way to create the thread :)
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, lpSoundBuffer, 0, 0);
#else
	_4klang_render(lpSoundBuffer);
#endif
	waveOutOpen			( &hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL );
	waveOutPrepareHeader( hWaveOut, &WaveHDR, sizeof(WaveHDR) );
	waveOutWrite		( hWaveOut, &WaveHDR, sizeof(WaveHDR) );	
}

/////////////////////////////////////////////////////////////////////////////////
// entry point for the executable if msvcrt is not used
/////////////////////////////////////////////////////////////////////////////////
#pragma code_seg(".main")
void mainCRTStartup(void)
{
	InitSound();
	do
	{
		// get sample position for timing
		waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));

#if 0
		// access to envelope buffer for sync. only works when the song was exported with that option
		// envelopes are recorded for each voice (max 2) in each instrument (max 16) every 256 samples. range [0..1]
		// so, e.g. to get envelope of instrument 5, voice 0 do:
		//float aha = (&_4klang_envelope_buffer)[((MMTime.u.sample >> 8) << 5) + 2*5+0];
		// so, e.g. to get envelope of instrument 3, voice 1 do:
		//float oho = (&_4klang_envelope_buffer)[((MMTime.u.sample >> 8) << 5) + 2*3+1];

		// small example playing a sound each time the bass in the example song is at an envelope level > 0.9
		float aha = (&_4klang_envelope_buffer)[((MMTime.u.sample >> 8) << 5) + 2*2+0]; 
		
		static bool on = 0;
		if (!on && aha > 0.9f)
		{
			on = true;
			MessageBeep(MB_OK);
		}
		if (aha < 0.9f)
			on = false;
#elif 0
		// access to note buffer for sync. only works when the song was exported with that option
		// notes are recorded for each voice (max 2) in each instrument (max 16) every 256 samples.
		// so, e.g. to get note of instrument 5, voice 0 do:
		//int aha = (&_4klang_note_buffer)[((MMTime.u.sample >> 8) << 5) + 2*5+0];
		// so, e.g. to get note of instrument 3, voice 1 do:
		//int oho = (&_4klang_note_buffer)[((MMTime.u.sample >> 8) << 5) + 2*3+1];

		// small example showing how to access the current note for a selected instrument (instrument 1 in this case)
		int curNote = (&_4klang_note_buffer)[((MMTime.u.sample >> 8) << 5) + 2*1+0]; 
		
		static bool on = 0;
		if (!on && (curNote == 52))
		{
			on = true;
			MessageBeep(MB_ICONERROR);
		}
		if (curNote != 52)
			on = false;
#endif

		// do your intro mainloop here
		// RenderIntro(MMTime.u.sample);

	} while (MMTime.u.sample < MAX_SAMPLES && !GetAsyncKeyState(VK_ESCAPE));
	ExitProcess(0);
}