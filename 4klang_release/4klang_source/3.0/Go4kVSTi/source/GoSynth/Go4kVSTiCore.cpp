#include "Go4kVSTiCore.h"

#include <math.h>

#include "Go4kVSTiGUI.h"
#include "..\..\win\resource.h"
#include <stdio.h>

#include <vector>
#include <map>
#include <list>
#include <string>

DWORD versiontag10 = 0x30316b34; // 4k10
DWORD versiontag11 = 0x31316b34; // 4k11
DWORD versiontag12 = 0x32316b34; // 4k12
DWORD versiontag   = 0x33316b34; // 4k13

static SynthObject SynthObj;

extern "C" void __stdcall go4kENV_func();
extern "C" void __stdcall go4kVCO_func();
extern "C" void __stdcall go4kVCF_func();
extern "C" void __stdcall go4kDST_func();
extern "C" void __stdcall go4kDLL_func();
extern "C" void __stdcall go4kFOP_func();
extern "C" void __stdcall go4kFST_func();
extern "C" void __stdcall go4kPAN_func();
extern "C" void __stdcall go4kOUT_func();
extern "C" void __stdcall go4kACC_func();
extern "C" void __stdcall go4kFLD_func();
extern "C" DWORD go4k_delay_buffer_ofs;
extern "C" float go4k_delay_buffer;
extern "C" WORD go4k_delay_times;
extern "C" float LFO_NORMALIZE;

typedef void (__stdcall *go4kFunc)(void); 

void __stdcall NULL_func()
{
};

static go4kFunc SynthFuncs[] =
{
	NULL_func,
	go4kENV_func,
	go4kVCO_func,
	go4kVCF_func,
	go4kDST_func,
	go4kDLL_func,
	go4kFOP_func,
	go4kFST_func,
	go4kPAN_func,
	go4kOUT_func,
	go4kACC_func,
	go4kFLD_func
};

static float BeatsPerMinute = 120.0f;

// solo mode handling
static int SoloChannel = 0;
static int Solo = 0;

// stream structures for recording sound
static DWORD samplesProcessed = 0;
static bool Recording = false;
static bool FirstRecordingEvent = false;
static DWORD PatternSize = 16;
static float SamplesPerTick;
static float TickScaler = 1.0f;
static DWORD MaxTicks;
static int InstrumentOn[MAX_INSTRUMENTS]; 
static std::vector<int> InstrumentRecord[MAX_INSTRUMENTS];
static std::vector<int> ReducedPatterns;
static std::vector<int> PatternIndices[MAX_INSTRUMENTS];
static int NumReducedPatterns = 0;

// init synth
void Go4kVSTi_Init()
{
	static bool initialized = false;
	// do one time initialisation here (e.g. wavtable generation ...)
	if (!initialized)
	{
		memset(&SynthObj, 0, sizeof(SynthObj));
		BeatsPerMinute = 120.0f;
		SoloChannel = 0;
		Solo = 0;
		Go4kVSTi_ResetPatch();
		initialized = true;
	}
}

// reset synth

void Go4kVSTi_ClearInstrumentSlot(char channel, int slot)
{
	memset(SynthObj.InstrumentValues[channel][slot], 0, MAX_SLOT_VALUES);
	for (int i = 0; i < MAX_POLYPHONY; i++)
	{
		float* w = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY+i].workspace[slot*8]);
		memset(w, 0, 8*4);
	}
}

void Go4kVSTi_ClearInstrumentWorkspace(char channel)
{
	// clear workspace
	InstrumentWorkspace* w = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY]);
	memset(w, 0, sizeof(InstrumentWorkspace)*MAX_POLYPHONY);
}

void Go4kVSTi_ResetInstrument(char channel)
{
	char name[128];
	sprintf(name, "Instrument %d", channel+1);
	memcpy(SynthObj.InstrumentNames[channel], name, strlen(name));

	// clear values
	BYTE* v = SynthObj.InstrumentValues[channel][0];
	memset(v, 0, MAX_SLOTS*MAX_SLOT_VALUES);

	// clear workspace
	InstrumentWorkspace* w = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY]);
	memset(w, 0, sizeof(InstrumentWorkspace)*MAX_POLYPHONY);

	// set default units
	Go4kVSTi_InitSlot(SynthObj.InstrumentValues[channel][0], channel, M_ENV);
	Go4kVSTi_InitSlot(SynthObj.InstrumentValues[channel][1], channel, M_VCO);
	Go4kVSTi_InitSlot(SynthObj.InstrumentValues[channel][2], channel, M_FOP); ((FOP_valP)(SynthObj.InstrumentValues[channel][2]))->flags = FOP_MULP;
	Go4kVSTi_InitSlot(SynthObj.InstrumentValues[channel][22], channel, M_DLL);
	Go4kVSTi_InitSlot(SynthObj.InstrumentValues[channel][23], channel, M_PAN);
	Go4kVSTi_InitSlot(SynthObj.InstrumentValues[channel][24], channel, M_OUT);

	SynthObj.InstrumentSignalValid[channel] = 1;
	SynthObj.SignalTrace[channel] = 0.0f;
	SynthObj.ControlInstrument[channel] = 0;
	SynthObj.VoiceIndex[channel] = 0;

	Go4kVSTi_ClearDelayLines();
}

void Go4kVSTi_ClearGlobalSlot(int slot)
{
	memset(SynthObj.GlobalValues[slot], 0, MAX_SLOT_VALUES);
	float* w = &(SynthObj.GlobalWork.workspace[slot*8]);
	memset(w, 0, 8*4);
}

void Go4kVSTi_ClearGlobalWorkspace()
{
	// clear workspace
	memset(&(SynthObj.GlobalWork), 0, sizeof(InstrumentWorkspace));
}

void Go4kVSTi_ResetGlobal()
{
	// clear values
	memset(SynthObj.GlobalValues, 0, MAX_SLOTS*MAX_SLOT_VALUES);

	// clear workspace
	memset(&(SynthObj.GlobalWork), 0, sizeof(InstrumentWorkspace));

	// set default units
	Go4kVSTi_InitSlot(SynthObj.GlobalValues[0], 16, M_ACC); ((ACC_valP)(SynthObj.GlobalValues[0]))->flags = ACC_AUX;
	Go4kVSTi_InitSlot(SynthObj.GlobalValues[1], 16, M_DLL); 
		((DLL_valP)(SynthObj.GlobalValues[1]))->reverb = 1; 
		((DLL_valP)(SynthObj.GlobalValues[1]))->leftreverb = 1; 
		((DLL_valP)(SynthObj.GlobalValues[1]))->feedback = 125; 
		((DLL_valP)(SynthObj.GlobalValues[1]))->pregain = 40; 
	Go4kVSTi_InitSlot(SynthObj.GlobalValues[2], 16, M_FOP); ((FOP_valP)(SynthObj.GlobalValues[2]))->flags = FOP_XCH;
	Go4kVSTi_InitSlot(SynthObj.GlobalValues[3], 16, M_DLL);
		((DLL_valP)(SynthObj.GlobalValues[3]))->reverb = 1; 
		((DLL_valP)(SynthObj.GlobalValues[3]))->leftreverb = 0; 
		((DLL_valP)(SynthObj.GlobalValues[3]))->feedback = 125; 
		((DLL_valP)(SynthObj.GlobalValues[3]))->pregain = 40; 
	Go4kVSTi_InitSlot(SynthObj.GlobalValues[4], 16, M_FOP); ((FOP_valP)(SynthObj.GlobalValues[4]))->flags = FOP_XCH;
	Go4kVSTi_InitSlot(SynthObj.GlobalValues[5], 16, M_ACC); 
	Go4kVSTi_InitSlot(SynthObj.GlobalValues[6], 16, M_FOP); ((FOP_valP)(SynthObj.GlobalValues[6]))->flags = FOP_ADDP2;
	Go4kVSTi_InitSlot(SynthObj.GlobalValues[24], 16, M_OUT); 
	
	SynthObj.GlobalSignalValid = 1;

	PatternSize = 16;

	Go4kVSTi_ClearDelayLines();
}

// reset synth
void Go4kVSTi_ResetPatch()
{
	for (int i = 0; i < MAX_INSTRUMENTS; i ++)
	{
		Go4kVSTi_ResetInstrument(i);
	}
	// reset global settings
	Go4kVSTi_ResetGlobal();

	SynthObj.Polyphony = 1;
}

void Go4kVSTi_FlipInstrumentSlots(char channel, int a, int b)
{
	DWORD temp[MAX_SLOT_VALUES];
	BYTE* v1 = SynthObj.InstrumentValues[channel][a];
	BYTE* v2 = SynthObj.InstrumentValues[channel][b];
	memcpy(temp, v2, MAX_SLOT_VALUES);
	memcpy(v2, v1, MAX_SLOT_VALUES);
	memcpy(v1, temp, MAX_SLOT_VALUES);
	for (int i = 0; i < MAX_POLYPHONY; i++)
	{
		float* w1 = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY+i].workspace[a*8]);
		float* w2 = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY+i].workspace[b*8]);
		memcpy(temp, w2, 8*4);
		memcpy(w2, w1, 8*4);
		memcpy(w1, temp, 8*4);
	}
	// reset dll workspaces, they are invalid now
	if (v1[0] == M_DLL && v2[0] == M_DLL)
	{
		Go4kVSTi_ClearDelayLines();
		Go4kVSTi_UpdateDelayTimes();
	}
}

void Go4kVSTi_FlipGlobalSlots(int a, int b)
{
	DWORD temp[MAX_SLOT_VALUES];
	BYTE* v1 = SynthObj.GlobalValues[a];
	BYTE* v2 = SynthObj.GlobalValues[b];
	memcpy(temp, v2, MAX_SLOT_VALUES);
	memcpy(v2, v1, MAX_SLOT_VALUES);
	memcpy(v1, temp, MAX_SLOT_VALUES);
	for (int i = 0; i < MAX_POLYPHONY; i++)
	{
		float* w1 = &(SynthObj.GlobalWork.workspace[a*8]);
		float* w2 = &(SynthObj.GlobalWork.workspace[b*8]);
		memcpy(temp, w2, 8*4);
		memcpy(w2, w1, 8*4);
		memcpy(w1, temp, 8*4);
	}
	// reset dll workspaces, they are invalid now
	if (v1[0] == M_DLL && v2[0] == M_DLL)
	{
		Go4kVSTi_ClearDelayLines();
		Go4kVSTi_UpdateDelayTimes();
	}
}

// init a unit slot
void Go4kVSTi_InitSlot(BYTE* slot, int channel, int type)
{
		// set default values
	slot[0] = type;
	if (type == M_ENV)
	{
		ENV_valP v = (ENV_valP)slot;
		v->attac		=  64;
		v->decay		=  64;
		v->sustain		=  64;
		v->release		=  64;
		v->gain			= 128;
	}
	if (type == M_VCO)
	{
		VCO_valP v = (VCO_valP)slot;
		v->transpose	=  64;
		v->detune		=  64;
		v->phaseofs		=   0;
		v->gain			=0x55;
		v->color		=  64;
		v->shape		=  64;
		v->gain			= 128;
		v->flags		= VCO_SINE;
	}
	if (type == M_VCF)
	{
		VCF_valP v = (VCF_valP)slot;
		v->freq			=  64;
		v->res			=  64;
		v->type			= VCF_LOWPASS;
	}
	if (type == M_DST)
	{
		DST_valP v = (DST_valP)slot;
		v->drive		=  64;
		v->snhfreq		= 128;
	}
	if (type == M_DLL)
	{
		DLL_valP v = (DLL_valP)slot;
		v->reverb		=   0;
		v->delay		=   0;
		v->count		=   1;
		v->pregain		=  64;
		v->dry			=  128;
		v->feedback		=  64;
		v->damp			=  64;
		v->guidelay		=  40;
		v->synctype		=   1;
		v->leftreverb	=   0;
		v->depth		=	0;
		v->freq			=	0;
	}
	if (type == M_FOP)
	{
		FOP_valP v = (FOP_valP)slot;
		v->flags		= FOP_MULP;
	}
	if (type == M_FST)
	{
		FST_valP v = (FST_valP)slot;
		v->amount		=  64;
		v->dest_stack	= -1;
		v->dest_unit	= -1;
		v->dest_slot	= -1;
		v->dest_id		= -1;
	}
	if (type == M_PAN)
	{
		PAN_valP v = (PAN_valP)slot;
		v->panning		=  64;
	}
	if (type == M_OUT)
	{
		OUT_valP v = (OUT_valP)slot;
		v->gain			=  64;
		v->auxsend		=   0;
	}
	if (type == M_ACC)
	{
		ACC_valP v = (ACC_valP)slot;
		v->flags		= ACC_OUT;
	}
	if (type == M_FLD)
	{
		FLD_valP v = (FLD_valP)slot;
		v->value		=  64;
	}
}

// init a instrument slot
void Go4kVSTi_InitInstrumentSlot(char channel, int s, int type)
{
	// clear values and workspace
	Go4kVSTi_ClearInstrumentSlot(channel, s);
	// init with default values
	Go4kVSTi_InitSlot(SynthObj.InstrumentValues[channel][s], channel, type);
	if (type == M_DLL)
	{
		Go4kVSTi_ClearDelayLines();
		Go4kVSTi_UpdateDelayTimes();
	}
}

// init a global slot
void Go4kVSTi_InitGlobalSlot(int s, int type)
{
	// clear values and workspace
	Go4kVSTi_ClearGlobalSlot(s);
	// init with default values
	Go4kVSTi_InitSlot(SynthObj.GlobalValues[s], 16, type);
	if (type == M_DLL)
	{
		Go4kVSTi_ClearDelayLines();
		Go4kVSTi_UpdateDelayTimes();
	}
}

// panic
void Go4kVSTi_Panic()
{
	for (int i = 0; i < MAX_INSTRUMENTS; i++)
	{
		// clear workspace
		InstrumentWorkspace* w = &(SynthObj.InstrumentWork[i*MAX_POLYPHONY]);
		memset(w, 0, sizeof(InstrumentWorkspace)*MAX_POLYPHONY);
		SynthObj.SignalTrace[i] = 0.0f;
	}
	// clear workspace
	memset(&(SynthObj.GlobalWork), 0, sizeof(InstrumentWorkspace));
	Go4kVSTi_ClearDelayLines();
}

static float delayTimeFraction[33] = 
{	
	4.0f * (1.0f/32.0f) * (2.0f/3.0f),
	4.0f * (1.0f/32.0f),
	4.0f * (1.0f/32.0f) * (3.0f/2.0f),
	4.0f * (1.0f/16.0f) * (2.0f/3.0f),
	4.0f * (1.0f/16.0f),
	4.0f * (1.0f/16.0f) * (3.0f/2.0f),
	4.0f * (1.0f/8.0f) * (2.0f/3.0f),
	4.0f * (1.0f/8.0f),
	4.0f * (1.0f/8.0f) * (3.0f/2.0f),
	4.0f * (1.0f/4.0f) * (2.0f/3.0f),
	4.0f * (1.0f/4.0f),
	4.0f * (1.0f/4.0f) * (3.0f/2.0f),
	4.0f * (1.0f/2.0f) * (2.0f/3.0f),
	4.0f * (1.0f/2.0f),
	4.0f * (1.0f/2.0f) * (3.0f/2.0f),
	4.0f * (1.0f) * (2.0f/3.0f),
	4.0f * (1.0f),
	4.0f * (1.0f) * (3.0f/2.0f),
	4.0f * (2.0f) * (2.0f/3.0f),
	4.0f * (2.0f),
	4.0f * (2.0f) * (3.0f/2.0f),
	4.0f * (3.0f/8.0f),
	4.0f * (5.0f/8.0f),
	4.0f * (7.0f/8.0f),
	4.0f * (9.0f/8.0f),
	4.0f * (11.0f/8.0f),
	4.0f * (13.0f/8.0f),
	4.0f * (15.0f/8.0f),
	4.0f * (3.0f/4.0f),
	4.0f * (5.0f/4.0f),
	4.0f * (7.0f/4.0f),
	4.0f * (3.0f/2.0f),
	4.0f * (3.0f/2.0f),
};

void Go4kVSTi_UpdateDelayTimes()
{
	int delayindex = 17;
	for (int i = 0; i <= MAX_INSTRUMENTS; i++)
	{
		for (int u = 0; u < MAX_SLOTS; u++)
		{
			DLL_valP v;
			if (i < MAX_INSTRUMENTS)
				v = (DLL_valP)(SynthObj.InstrumentValues[i][u]);
			else
				v = (DLL_valP)(SynthObj.GlobalValues[u]);

			if (v->id == M_DLL)
			{
				DLL_valP v = (DLL_valP)(SynthObj.InstrumentValues[i][u]);
				if (v->reverb)
				{
					if (v->leftreverb)
					{
						v->delay = 1;
						v->count = 8;
					}
					else
					{
						v->delay = 9;
						v->count = 8;
					}
				}
				else
				{
					int delay;
					if (v->synctype == 2)
					{
						v->delay = 0;
						v->count = 1;
					}
					else
					{
						if (v->synctype == 1)
						{
							float ftime;
							float quarterlength = 60.0f/Go4kVSTi_GetBPM();
							ftime = quarterlength*delayTimeFraction[v->guidelay>>2];
							delay = 44100.0f*ftime;
							if (delay >= 65536)
								delay = 65535;
						}
						else
						{
							delay = v->guidelay*16;
						}
						(&go4k_delay_times)[delayindex] = delay;
						v->delay = delayindex;
						v->count = 1;
					}
					delayindex++;
				}
			}
		}
	}
}

// clear delay lines
void Go4kVSTi_ClearDelayLines()
{
	memset((&go4k_delay_buffer), 0, 80*((65536+4)*4));
}

// set global bpm
void Go4kVSTi_SetBPM(float bpm)
{
	BeatsPerMinute = bpm;
	LFO_NORMALIZE = bpm/(44100.0*60.0);
	Go4kVSTi_UpdateDelayTimes();
}

// get bpm
float Go4kVSTi_GetBPM()
{
	return BeatsPerMinute;
}

// enable solo mode for a single channel only
void Go4kVSTi_Solo(int channel, int solo)
{
	if (solo)
	{
		SoloChannel = channel;
		Solo = true;
	}
	else
	{
		Solo = false;
	}
}

// sample times tick the whole synth pipeline. results are left and right output sample

void Go4kVSTi_Tick(float *oleft, float *oright, int samples)
{
	// do as many samples as requested
	int s = 0;
	while (s < samples)
	{
		float left=0.0f;
		float right=0.0f;
		
		go4k_delay_buffer_ofs = (DWORD)(&go4k_delay_buffer);
		// loop all instruments
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
		{
			// solo mode and not the channel we want?
			if (Solo && i != SoloChannel)
			{
				// loop all voices and clear outputs
				for (int p = 0; p < SynthObj.Polyphony; p++)
				{
					InstrumentWorkspaceP iwork = &(SynthObj.InstrumentWork[i*MAX_POLYPHONY+p]);
					iwork->dlloutl = 0.0f;
					iwork->dlloutr = 0.0f;
					iwork->outl = 0.0f;
					iwork->outr = 0.0f;
				}
				// adjust delay index
				for (int s = 0; s < MAX_SLOTS; s++)
				{		
					BYTE* val = SynthObj.InstrumentValues[i][s];
					if (val[0] == M_DLL)
						go4k_delay_buffer_ofs += (5+65536)*4*SynthObj.Polyphony;
				}
				// go to next instrument
				continue;
			}
			// if the instrument signal stack is valid and we still got a signal from that instrument
			if (SynthObj.InstrumentSignalValid[i] && (fabs(SynthObj.SignalTrace[i]) > 0.00001f))
			{
				float sumSignals = 0.0f;
				// loop all voices
				for (int p = 0; p < SynthObj.Polyphony; p++)
				{
					InstrumentWorkspaceP iwork = &(SynthObj.InstrumentWork[i*MAX_POLYPHONY+p]);
					float *lwrk = iwork->workspace;
					DWORD inote = iwork->note;
					// loop each slot
					for (int s = 0; s < MAX_SLOTS; s++)
					{		
						BYTE* val = SynthObj.InstrumentValues[i][s];
						float *wrk = &(iwork->workspace[s*8]);
						if (val[0] == M_FST && iwork->note)
						{
							FST_valP v = (FST_valP)val;
							// if a target slot is set
							if (v->dest_slot != -1)
							{
								InstrumentWorkspaceP mwork;
								int polyphonicStore = SynthObj.Polyphony;
								int stack = v->dest_stack;
								// local storage?
								if (stack == -1 || stack == i)
								{
									// only store the sample in the current workspace
									polyphonicStore = 1;
									mwork = iwork;
								}
								else if (stack == MAX_INSTRUMENTS)
									mwork = &(SynthObj.GlobalWork);
								else
									mwork = &(SynthObj.InstrumentWork[stack*MAX_POLYPHONY]);
								
								float* mdest = &(mwork->workspace[v->dest_unit*8 + v->dest_slot]);
								float amount = (2.0f*v->amount - 128.0f)*0.0078125f;
								for (int stc = 0; stc < polyphonicStore; stc++)
								{									
									__asm
									{
										push	eax
										fld		amount
										fmul	st(0), st(1)
										mov		eax, mdest
										fstp	dword ptr [eax]
										pop		eax
									}
									mdest += sizeof(InstrumentWorkspace)/4;
								}
							}
						}
						else
						{
							// only process if note active or dll unit
							if (val[0])
							{
								// set up and call synth core func
								__asm
								{
									pushad
									xor		eax, eax
									mov		esi, val
									lodsb
									mov		eax, dword ptr [SynthFuncs+eax*4]
									mov		ebx, inote
									mov		ecx, lwrk
									mov		ebp, wrk
									call	eax						
									popad
								}
							}
						}
					}
					// check for end of note
					DWORD envstate = *((BYTE*)(lwrk));
					if (envstate == ENV_STATE_OFF)
					{
						iwork->note = 0;
					}
					sumSignals += fabsf(iwork->outl) + fabsf(iwork->outr) + fabsf(iwork->dlloutl) + fabsf(iwork->dlloutr);
				}
				// update envelope follower only for non control instruments. (1s attack rate) for total instrument signal
				if (SynthObj.ControlInstrument[i])
					SynthObj.SignalTrace[i] = 1.0f;
				else
					SynthObj.SignalTrace[i] = sumSignals + 0.999977324f * ( SynthObj.SignalTrace[i] - sumSignals );	
			}
			// instrument stack invalid
			else
			{
				// adjust delay index
				for (int s = 0; s < MAX_SLOTS; s++)
				{		
					BYTE* val = SynthObj.InstrumentValues[i][s];
					if (val[0] == M_DLL)
						go4k_delay_buffer_ofs += (5+65536)*4*SynthObj.Polyphony;
				}
				// loop all voices
				for (int p = 0; p < SynthObj.Polyphony; p++)
				{
					InstrumentWorkspaceP iwork = &(SynthObj.InstrumentWork[i*MAX_POLYPHONY+p]);
					iwork->dlloutl = 0.0f;
					iwork->dlloutr = 0.0f;
					iwork->outl = 0.0f;
					iwork->outr = 0.0f;
				}
			}
		}
		// if the global stack is valid
		if (SynthObj.GlobalSignalValid)
		{
			InstrumentWorkspaceP gwork = &(SynthObj.GlobalWork);
			float *lwrk = gwork->workspace;
			DWORD gnote = 1;
			gwork->note = 1;
			// loop all global slots
			for (int s = 0; s < MAX_SLOTS; s++)
			{		
				BYTE* val = SynthObj.GlobalValues[s];
				float *wrk = &(lwrk[s*8]);
				// manually accumulate signals
				float ACCL = 0.0f;
				float ACCR = 0.0f;
				if (val[0] == M_ACC)
				{
					ACC_valP av = (ACC_valP)val;
					if (av->flags == ACC_OUT)
					{
						for (int i = 0; i < MAX_INSTRUMENTS; i++)
						{
							for (int p = 0; p < SynthObj.Polyphony; p++)
							{
								ACCL += SynthObj.InstrumentWork[i*MAX_POLYPHONY+p].outl;
								ACCR += SynthObj.InstrumentWork[i*MAX_POLYPHONY+p].outr;
							}
						}
					}
					else
					{
						for (int i = 0; i < MAX_INSTRUMENTS; i++)
						{
							for (int p = 0; p < SynthObj.Polyphony; p++)
							{
								ACCL += SynthObj.InstrumentWork[i*MAX_POLYPHONY+p].dlloutl;
								ACCR += SynthObj.InstrumentWork[i*MAX_POLYPHONY+p].dlloutr;
							}
						}
					}
					// push the accumulated signals on the fp stack
					__asm
					{
						fld		ACCR
						fld		ACCL
					}
				}
				// no ACC unit, check store
				else if (val[0] == M_FST && gwork->note)
				{
					FST_valP v = (FST_valP)val;
					// if a target slot is set
					if (v->dest_slot != -1)
					{
						InstrumentWorkspaceP mwork;
						int polyphonicStore = SynthObj.Polyphony;
						int stack = v->dest_stack;
						// local storage?
						if (stack == -1 || stack == MAX_INSTRUMENTS)
						{
							// only store the sample in the current workspace
							polyphonicStore = 1;
							mwork = &(SynthObj.GlobalWork);
						}
						else
							mwork = &(SynthObj.InstrumentWork[stack*MAX_POLYPHONY]);
						
						float* mdest = &(mwork->workspace[v->dest_unit*8 + v->dest_slot]);
						float amount = (2.0f*v->amount - 128.0f)*0.0078125f;;
						for (int stc = 0; stc < polyphonicStore; stc++)
						{
							__asm
							{
								push	eax
								fld		amount
								fmul	st(0), st(1)
								mov		eax, mdest
								fstp	dword ptr [eax]
								pop		eax
							}
							mdest += sizeof(InstrumentWorkspace)/4;
						}
					}
				}
				// just call synth core func
				else
				{
					if (val[0])
					{
						__asm
						{
							pushad
							xor		eax, eax
							mov		esi, val
							lodsb
							mov		eax, dword ptr [SynthFuncs+eax*4]
							mov		ebx, gnote
							mov		ecx, lwrk
							mov		ebp, wrk
							call	eax						
							popad
						}
					}
				}
			}
			left = gwork->outl;
			right = gwork->outr;
		}

		// clip 
		if (left < -1.0f)
			left = -1.0f;
		if (left > 1.0f)
			left = 1.0f;
		if (right < -1.0f)
			right = -1.0f;
		if (right > 1.0f)
			right = 1.0f;

		*(oleft++) = left;
		*(oright++) = right;

		s++;
	} // end sample loop

	if (Recording)
	{
		samplesProcessed += samples;
	}
}

////////////////////////////////////////////////////////////////////////////
// 
//	Synth input processing 
//
////////////////////////////////////////////////////////////////////////////

// prepare for recording the midi stream
void Go4kVSTi_Record(bool record, int patternsize, float patternquant)
{
	Recording = record;
	// if we started recording, clear all record streams
	if (Recording)
	{
		TickScaler = patternquant;
		PatternSize = (int)(patternsize*TickScaler);
		// set first recording event variable to true to enable start time reset on the first occuring event
		FirstRecordingEvent = true;
		// clear all buffers
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
		{
			InstrumentOn[i] = -1;
			InstrumentRecord[i].clear();
			for (int j = 0; j < 256*256; j++)
				InstrumentRecord[i].push_back(0);
		}
	}
	else
	{
		// nothing recorded?
		if (FirstRecordingEvent == true)
			return;
		Recording = true;
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
			Go4kVSTi_StopVoice(i, 0);
		Recording = false;
		// how many ticks did we record?
		MaxTicks = (int)(0.5f+((float)samplesProcessed/SamplesPerTick));
		int remainder = MaxTicks % 16;
		MaxTicks += remainder;
		// fold patterns if TickScaler < 1, this is needed because directly recording at half samplerate leads to information loss
		if (TickScaler < 1.0f)
		{	
			MaxTicks = (int)((float)MaxTicks*TickScaler);
			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				for (int j = 0; j < (int)(256*256*TickScaler); j++)
					InstrumentRecord[i][j] = InstrumentRecord[i][(int)(j/TickScaler)];
			}			
		}
		// init reduced patterns
		ReducedPatterns.clear();
		NumReducedPatterns = 0;
		// add NULL pattern
		for (int i = 0; i < PatternSize; i++)
		{
			ReducedPatterns.push_back(0);
		}
		// now add minimal reduced instrument patterns		
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
		{
			PatternIndices[i].clear();
			// loop all patterns
			for (int j = 0; j < MaxTicks/PatternSize; j++)
			{
				int pindex = j;
				bool found = false;
				// compare each tick of the current pattern to all already reduced patterns
				for (int k = 0; k < ReducedPatterns.size()/PatternSize; k++)
				{
					bool patternMatch = true;
					for (int l = 0; l < PatternSize; l++)
					{						
						if (InstrumentRecord[i][j*PatternSize+l] != ReducedPatterns[k*PatternSize+l])
						{
							patternMatch = false;
							break;
						}
					}
					if (patternMatch)
					{
						NumReducedPatterns++;
						pindex = k;
						found = true;
						break;
					}
				}
				// add new pattern if necessary
				if (!found)
				{
					for (int k = 0; k < PatternSize; k++)
					{
						ReducedPatterns.push_back(InstrumentRecord[i][j*PatternSize+k]);
					}			
					pindex = (ReducedPatterns.size()/PatternSize)-1;
				}
				// add new pattern index
				PatternIndices[i].push_back(pindex);
			}				
		}

		// open file dialog and save if desired ...
		// the save function called from this one is GoSynth_SaveByteStream
		GetStreamFileName();	
	} 
}

// add a voice with given parameters to synth
void Go4kVSTi_AddVoice(int channel, int note)
{
	InstrumentWorkspaceP work,work2;
	work = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY+0]);
	work->release = 1;
	work2 = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY+1]);
	work2->release = 1;
	// filp worspace
	if (SynthObj.Polyphony > 1)
	{
		work = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY+SynthObj.VoiceIndex[channel]]);
		SynthObj.VoiceIndex[channel] = SynthObj.VoiceIndex[channel] ^ 0x1;
	}
	// add new note
	memset(work, 0, 256*4+8);
	work->note = note;
	SynthObj.SignalTrace[channel] = 1.0f;
	// check if its a controll instrument which is played
	SynthObj.ControlInstrument[channel] = 1;
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		if (SynthObj.InstrumentValues[channel][i][0] == M_OUT)
		{
			SynthObj.ControlInstrument[channel] = 0;
			break;
		}
	}

	// record song
	if (Recording)
	{
		// check for the first recording event
		if (FirstRecordingEvent)
		{
			FirstRecordingEvent = false;
			// reset time stamp for very first event
			samplesProcessed = 0;
			// Set samples per tick for this recording
			if (TickScaler >= 1.0f)
				SamplesPerTick = 44100.0f*4.0f*60.0f/(BeatsPerMinute*16*TickScaler);
			else
				SamplesPerTick = 44100.0f*4.0f*60.0f/(BeatsPerMinute*16);
		}
		int CurrentTick = (int)(0.5f+((float)samplesProcessed/SamplesPerTick));
		if (InstrumentOn[channel] >= 0)
		{
			for (int i = CurrentTick-1; i > InstrumentOn[channel]; i--)
			{
				if (!InstrumentRecord[channel][i])
					InstrumentRecord[channel][i] = -1;
			}
		}
		if (!InstrumentRecord[channel][CurrentTick])
		{
			InstrumentRecord[channel][CurrentTick] = note;
			InstrumentOn[channel] = CurrentTick;
		}
	}
}

// stop a voice with given parameters in synth
void Go4kVSTi_StopVoice(int channel, int note)
{
	InstrumentWorkspaceP work,work2;
	// release notes
	work = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY+0]);
	work->release = 1;
	work2 = &(SynthObj.InstrumentWork[channel*MAX_POLYPHONY+1]);
	work2->release = 1;
	// record song
	if (Recording)
	{
		int CurrentTick = (int)(0.5f+((float)samplesProcessed/SamplesPerTick));
		if (InstrumentOn[channel] >= 0)
		{
			for (int i = CurrentTick-1; i > InstrumentOn[channel]; i--)
			{
				if (!InstrumentRecord[channel][i])
					InstrumentRecord[channel][i] = -1;
			}
		}
//		if (!InstrumentRecord[channel][CurrentTick])
			InstrumentOn[channel] = -1;
	}
}

//----------------------------------------------------------------------------------------------------
// convenience functions for loading and storing patches and instruments
//----------------------------------------------------------------------------------------------------

void FlipSlotModulations(int stack, int unit1, int unit2)
{
	// look in all instruments if a store unit had its target on one of the changed units
	for (int i = 0; i <= MAX_INSTRUMENTS; i++)
	{
		BYTE* values;
		if (i < MAX_INSTRUMENTS)
			values = SynthObj.InstrumentValues[i][0];
		else
			values = SynthObj.GlobalValues[0];
		for (int u = 0; u < MAX_SLOTS; u++)
		{
			if (values[u*MAX_SLOTS+0] == M_FST)
			{
				FST_valP v = (FST_valP)(&values[u*MAX_SLOTS+0]);
								
				int target_inst;
				if (v->dest_stack == -1)
					target_inst = i;
				else
					target_inst = v->dest_stack;

				// the store points to another stack, so continue
				if (target_inst != stack)
					continue;

				// a up/down process 
				if (unit2 != -1)
				{
					// if the target unit was unit1 or unit2 realign target unit
					if (v->dest_unit == unit1)
					{
						v->dest_unit = unit2;
					}
					else if (v->dest_unit == unit2)
					{
						v->dest_unit = unit1;
					}
				}
			}
		}
	}
}

// autoconvert 1.0 instrument stacks
bool Autoconvert10(int stack)
{
	// get desired stack
	BYTE* values;
	if (stack < MAX_INSTRUMENTS)
		values = SynthObj.InstrumentValues[stack][0];
	else
		values = SynthObj.GlobalValues[0];

	// replace the delay with the new one
	for (int u = 0; u < MAX_SLOTS; u++)
	{
		if (values[u*MAX_SLOTS+0] == M_DLL)
		{
			DLL10_val ov;
			memcpy(&ov, &values[u*MAX_SLOTS+0], sizeof(DLL10_val));
			DLL_valP nv = (DLL_valP)(&values[u*MAX_SLOTS+0]);
			nv->id = ov.id;
			nv->pregain = ov.pregain;
			nv->dry = ov.dry;
			nv->feedback = ov.feedback;
			nv->damp = ov.damp;
			nv->freq = 0;
			nv->depth = 0;
			nv->guidelay = ov.guidelay;
			nv->synctype = ov.synctype;
			nv->leftreverb = ov.leftreverb;
			nv->reverb = ov.reverb;
		}
	}
	return true;
}

// autoconvert 1.1 instrument stacks
bool Autoconvert11(int stack)
{
	// get desired stack
	BYTE* values;
	if (stack < MAX_INSTRUMENTS)
		values = SynthObj.InstrumentValues[stack][0];
	else
		values = SynthObj.GlobalValues[0];

	// replace the osc with the new one
	for (int u = 0; u < MAX_SLOTS; u++)
	{
		if (values[u*MAX_SLOTS+0] == M_VCO)
		{
			VCO11_val ov;
			memcpy(&ov, &values[u*MAX_SLOTS+0], sizeof(VCO11_val));
			VCO_valP nv = (VCO_valP)(&values[u*MAX_SLOTS+0]);
			nv->id = ov.id;
			nv->transpose = ov.transpose;
			nv->detune = ov.detune;
			nv->phaseofs = ov.phaseofs;
			nv->gate = 0x55;
			nv->color = ov.color;
			nv->shape = ov.shape;
			nv->gain = ov.gain;
			nv->flags = ov.flags;
		}
	}
	return true;
}

// load patch data
void Go4kVSTi_LoadPatch(char *filename)
{
	Go4kVSTi_ResetPatch();
	FILE *file = fopen(filename, "rb");
	if (file)
	{
		DWORD version;
		bool version10 = false;
		bool version11 = false;
		bool version12 = false;
		fread(&version, 1, 4, file);
		if (versiontag != version)
		{
			// version 1.2 file
			if (version == versiontag12)
			{
				// only fld unit added, no need for message
				//MessageBox(0,"Autoconvert. Please save file again", "1.2 File Format", MB_OK | MB_SETFOREGROUND);
				version12 = true;
			}
			// version 1.1 file
			else if (version == versiontag11)
			{
				MessageBox(0,"Autoconvert. Please save file again", "1.1 File Format", MB_OK | MB_SETFOREGROUND);
				version11 = true;
				version12 = true;
			}
			// version 1.0 file
			else if (version == versiontag10)
			{
				MessageBox(0,"Autoconvert. Please save file again", "1.0 File Format", MB_OK | MB_SETFOREGROUND);
				version10 = true;
				version11 = true;
				version12 = true;
			}
			// newer format than supported
			else
			{
				MessageBox(0,"The file was created with a newer version of 4klang.", "File Format Error", MB_ICONERROR | MB_SETFOREGROUND);
				fclose(file);
				return;
			}
		}

		int slots=MAX_SLOTS;
		// read data
		fread(&(SynthObj.Polyphony), 1, 4, file);
		fread(SynthObj.InstrumentNames, 1, MAX_INSTRUMENTS*64, file);
		for (int i=0; i<MAX_INSTRUMENTS; i++)
		{
			fread(SynthObj.InstrumentValues[i], 1, slots*MAX_SLOT_VALUES, file);
		}
		fread(SynthObj.GlobalValues, 1, slots*MAX_SLOT_VALUES, file);
		fclose(file);

		// convert 1.0 file format
		if (version10)
		{
			// convert all instruments
			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (!Autoconvert10(i))
				{
					char errmsg[64];
					sprintf(errmsg, "Instrument %d could not be converted", i+1);
					MessageBox(0, errmsg, "Error", MB_OK | MB_SETFOREGROUND);
				}
			}
			// convert global
			if (!Autoconvert10(MAX_INSTRUMENTS))
			{
				char errmsg[64];
				sprintf(errmsg, "Global could not be converted");
				MessageBox(0, errmsg, "Error", MB_OK | MB_SETFOREGROUND);
			}
		}
		// convert 1.1 file format
		if (version11)
		{
			// convert all instruments
			for (int i = 0; i < MAX_INSTRUMENTS; i++)
			{
				if (!Autoconvert11(i))
				{
					char errmsg[64];
					sprintf(errmsg, "Instrument %d could not be converted", i+1);
					MessageBox(0, errmsg, "Error", MB_OK | MB_SETFOREGROUND);
				}
			}
			// convert global
			if (!Autoconvert11(MAX_INSTRUMENTS))
			{
				char errmsg[64];
				sprintf(errmsg, "Global could not be converted");
				MessageBox(0, errmsg, "Error", MB_OK | MB_SETFOREGROUND);
			}
		}
		// convert 1.2 file format
		if (version12)
		{
			// nothing to do, only fld unit added at the end
		}
	}
	Go4kVSTi_UpdateDelayTimes();
}

// save patch data
void Go4kVSTi_SavePatch(char *filename)
{
	FILE *file = fopen(filename, "wb");
	if (file)
	{
		fwrite(&versiontag, 1, 4, file);
		fwrite(&(SynthObj.Polyphony), 1, 4, file);
		fwrite(SynthObj.InstrumentNames, 1, MAX_INSTRUMENTS*64, file);
		fwrite(SynthObj.InstrumentValues, 1, MAX_INSTRUMENTS*MAX_SLOTS*MAX_SLOT_VALUES, file);
		fwrite(SynthObj.GlobalValues, 1, MAX_SLOTS*MAX_SLOT_VALUES, file);
		fclose(file);
	}
}

// load instrumen data to specified channel
void Go4kVSTi_LoadInstrument(char* filename, char channel)
{
	FILE *file = fopen(filename, "rb");
	if (file)
	{
		DWORD version;
		bool version10 = false;
		bool version11 = false;
		bool version12 = false;
		fread(&version, 1, 4, file);
		if (versiontag != version) // 4k10
		{
			// version 1.2 file
			if (version == versiontag12)
			{
				// only fld unit added, no need for message
				//MessageBox(0,"Autoconvert. Please save file again", "1.2 File Format", MB_OK | MB_SETFOREGROUND);
				version12 = true;
			}
			// version 1.1 file
			else if (version == versiontag11)
			{
				MessageBox(0,"Autoconvert. Please save file again", "1.1 File Format", MB_OK | MB_SETFOREGROUND);
				version11 = true;
				version12 = true;
			}
			// version 1.0 file
			else if (version == versiontag10)
			{
				MessageBox(0,"Autoconvert. Please save file again", "1.0 File Format", MB_OK | MB_SETFOREGROUND);
				version10 = true;
				version11 = true;
				version12 = true;
			}
			// newer format than supported
			else
			{
				MessageBox(0,"The file was created with a newer version of 4klang.", "File Format Error", MB_ICONERROR | MB_SETFOREGROUND);
				fclose(file);
				return;
			}
		}
		
		int slots=MAX_SLOTS;
		if (channel < 16)
		{
			Go4kVSTi_ResetInstrument(channel);
			fread(SynthObj.InstrumentNames[channel], 1, 64, file);
			fread(SynthObj.InstrumentValues[channel], 1, slots*MAX_SLOT_VALUES, file);
		}
		else
		{
			Go4kVSTi_ResetGlobal();
			fread(SynthObj.InstrumentNames[channel], 1, 64, file);
			fread(SynthObj.GlobalValues, 1, slots*MAX_SLOT_VALUES, file);
		}
		fclose(file);
		// convert 1.0 file format
		if (version10)
		{
			// convert instruments
			if (channel < 16)
			{
				if (!Autoconvert10(channel))
				{
					char errmsg[64];
					sprintf(errmsg, "Instrument %d could not be converted", channel+1);
					MessageBox(0, errmsg, "Error", MB_OK | MB_SETFOREGROUND);
				}
			}
			// convert global
			else
			{
				if (!Autoconvert10(MAX_INSTRUMENTS))
				{
					char errmsg[64];
					sprintf(errmsg, "Global could not be converted");
					MessageBox(0, errmsg, "Error", MB_OK | MB_SETFOREGROUND);
				}
			}
		}
		// convert 1.1 file format
		if (version11)
		{
			// convert instruments
			if (channel < 16)
			{
				if (!Autoconvert11(channel))
				{
					char errmsg[64];
					sprintf(errmsg, "Instrument %d could not be converted", channel+1);
					MessageBox(0, errmsg, "Error", MB_OK | MB_SETFOREGROUND);
				}
			}
			// convert global
			else
			{
				if (!Autoconvert11(MAX_INSTRUMENTS))
				{
					char errmsg[64];
					sprintf(errmsg, "Global could not be converted");
					MessageBox(0, errmsg, "Error", MB_OK | MB_SETFOREGROUND);
				}
			}
		}
		// convert 1.2 file format
		if (version12)
		{
			// nothing to do, only fld unit added at the end
		}
	}
	Go4kVSTi_UpdateDelayTimes();
}

// save instrument data from current channel
void Go4kVSTi_SaveInstrument(char* filename, char channel)
{
	FILE *file = fopen(filename, "wb");
	if (file)
	{
		fwrite(&versiontag, 1, 4, file);
		if (channel < 16)
		{
			fwrite(SynthObj.InstrumentNames[channel], 1, 64, file);
			fwrite(SynthObj.InstrumentValues[channel], 1, MAX_SLOTS*MAX_SLOT_VALUES, file);
		}
		else
		{
			fwrite(SynthObj.InstrumentNames[channel], 1, 64, file);
			fwrite(SynthObj.GlobalValues, 1, MAX_SLOTS*MAX_SLOT_VALUES, file);
		}
		fclose(file);
	}
}

// load unit data into specified slot
void Go4kVSTi_LoadUnit(char* filename, BYTE* slot)
{
	FILE *file = fopen(filename, "rb");
	if (file)
	{
		fread(slot, 1, MAX_SLOT_VALUES, file);
		fclose(file);
		if (slot[0] == M_DLL)
		{
			Go4kVSTi_ClearDelayLines();
			Go4kVSTi_UpdateDelayTimes();
		}
	}
}

// save unit date from specified slot
void Go4kVSTi_SaveUnit(char* filename, BYTE* slot)
{
	FILE *file = fopen(filename, "wb");
	if (file)
	{
		fwrite(slot, 1, MAX_SLOT_VALUES, file);
		fclose(file);
	}
}

int GetShift2(int in)
{
	int ret = 0;
	while (in = (in >> 1))
	{
		ret++;
	}
	return ret;
}

struct SynthUses
{
	bool env_gm;
	bool env_adrm;

	bool vco_phaseofs;
	bool vco_shape;
	bool vco_fm;
	bool vco_pm;
	bool vco_tm;
	bool vco_dm;
	bool vco_cm;
	bool vco_gm;
	bool vco_sm;
	bool vco_gate;

	bool vcf_fm;
	bool vcf_rm;

	bool dst_use;
	bool dst_snh;
	bool dst_dm;
	bool dst_sm;

	bool dll_use;
	bool dll_notesync;
	bool dll_damp;
	bool dll_chorus;
	bool dll_use_mod;
	bool dll_pm;
	bool dll_fm;
	bool dll_im;
	bool dll_dm;
	bool dll_sm;
	bool dll_am;

	bool fop_loadnote;

	bool pan_use;
	bool pan_pm;

	bool fstg_use;
	
	bool out_am;
	bool out_gm;

	bool fld_use;
	bool fld_vm;
};

void GetUses(SynthUses *uses, bool InstrumentUsed[])
{
	for (int i = 0; i <= MAX_INSTRUMENTS; i++)
	{
		// skip usage checks for non used instruments
		if (i < MAX_INSTRUMENTS)
			if (!InstrumentUsed[i])
				continue;

		for (int u = 0; u < MAX_SLOTS; u++)
		{
			BYTE *v;
			if (i < MAX_INSTRUMENTS)
				v = SynthObj.InstrumentValues[i][u];
			else
				v = SynthObj.GlobalValues[u];

			if (v[0] == M_VCO)
			{
				if (((VCO_valP)v)->phaseofs != 0)
					uses->vco_phaseofs = true;
				if (((VCO_valP)v)->shape != 64)
					uses->vco_shape = true;
				if (((VCO_valP)v)->flags & VCO_GATE)
					uses->vco_gate = true;
			}
			if (v[0] == M_DST)
			{
				uses->dst_use = true;
				if (((DST_valP)v)->snhfreq != 128)
					uses->dst_snh = true;
			}
			if (v[0] == M_DLL)
			{
				uses->dll_use = true;
				if (((DLL_valP)v)->synctype == 2)
					uses->dll_notesync = true;
				if (((DLL_valP)v)->damp > 0)
					uses->dll_damp = true;
				if ((((DLL_valP)v)->freq > 0) && (((DLL_valP)v)->depth > 0))
					uses->dll_chorus = true;
			}
			if (v[0] == M_PAN)
			{
				if (((PAN_valP)v)->panning != 64)
					uses->pan_use = true;
			}
			if (v[0] == M_FOP)
			{
				if (((FOP_valP)v)->flags == FOP_LOADNOTE)
					uses->fop_loadnote = true;
			}
			if (v[0] == M_FLD)
			{				
				uses->fld_use = true;
			}
			if (v[0] == M_FST)
			{
				if ((((FST_valP)v)->dest_stack != -1) && (((FST_valP)v)->dest_stack != i))
					uses->fstg_use = true;

				InstrumentWorkspaceP mwork;
				int stack = ((FST_valP)v)->dest_stack;
				int unit = ((FST_valP)v)->dest_unit;
				int slot = ((FST_valP)v)->dest_slot;
				// local storage?
				if (stack == -1 || stack == i)
				{
					stack = i;
				}
				else
				{
					uses->fstg_use = true;
				}
				BYTE *v2;
				if (stack < MAX_INSTRUMENTS)
					v2 = SynthObj.InstrumentValues[stack][unit];
				else
					v2 = SynthObj.GlobalValues[unit];

				if (v2[0] == M_ENV)
				{
					if (slot == 2)
						uses->env_gm = true;
					if (slot == 3)
						uses->env_adrm = true;
					if (slot == 4)
						uses->env_adrm = true;
					if (slot == 6)
						uses->env_adrm = true;
				}
				if (v2[0] == M_VCO)
				{
					if (slot == 1)
						uses->vco_tm = true;
					if (slot == 2)
						uses->vco_dm = true;
					if (slot == 3)
						uses->vco_fm = true;
					if (slot == 4)
						uses->vco_pm = true;
					if (slot == 5)
						uses->vco_cm = true;
					if (slot == 6)
						uses->vco_sm = true;
					if (slot == 7)
						uses->vco_gm = true;
				}
				if (v2[0] == M_VCF)
				{
					if (slot == 4)
						uses->vcf_fm = true;
					if (slot == 5)
						uses->vcf_rm = true;
				}
				if (v2[0] == M_DST)
				{
					if (slot == 2)
						uses->dst_dm = true;
					if (slot == 3)
						uses->dst_sm = true;
				}
				if (v2[0] == M_DLL)
				{
					if (slot == 0)
					{
						uses->dll_pm = true;
						uses->dll_use_mod = true;
					}
					if (slot == 1)
					{
						uses->dll_fm = true;
						uses->dll_use_mod = true;
					}
					if (slot == 2)
					{
						uses->dll_im = true;
						uses->dll_use_mod = true;
					}
					if (slot == 3)
					{
						uses->dll_dm = true;
						uses->dll_damp = true;
						uses->dll_use_mod = true;
					}
					if (slot == 4)
					{
						uses->dll_sm = true;
						uses->dll_use_mod = true;
					}					
					if (slot == 5)
					{
						uses->dll_am = true;
						uses->dll_use_mod = true;
					}
				}
				if (v2[0] == M_PAN)
				{
					if (slot == 0)
						uses->pan_pm = true;
				}
				if (v2[0] == M_OUT)
				{
					if (slot == 0)
						uses->out_am = true;
					if (slot == 1)
						uses->out_gm = true;
				}
				if (v2[0] == M_FLD)
				{
					if (slot == 0)
						uses->fld_vm = true;
				}
			}
		}
	}
}

#ifdef EXPORT_OBJECT_FILE
int randiSeed = 23;
int randi()
{	
	return( ((randiSeed = randiSeed * 214013L + 2531011L) >> 16)  & 0x7fff );
}
#endif

void Go4kVSTi_SaveByteStream(HINSTANCE hInst, char* filename, int useenvlevels, int useenotevalues, int clipoutput, int undenormalize, int objformat, int output16)
{
	std::string fpath = filename;
#ifdef EXPORT_OBJECT_FILE
	unsigned char* CryptTable = (unsigned char*)malloc(1024*1024);
	randiSeed = 23;
	for (int i = 0; i < 1024*1024; i++)
		CryptTable[i] = randi() & 0xff;
	char temppath[1024];
	GetTempPath(1024, temppath);
	std::string path = temppath;
	// construct temp strings
	std::string incfile = path + "4klang.inc";
#else
	std::string incfile = filename;
#endif
	int maxinst = 0;
	FILE *file = fopen(incfile.c_str(), "w");
	if (file)
	{
		bool InstrumentUsed[MAX_INSTRUMENTS];
		// determine max instrument count
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
		{
			InstrumentUsed[i] = false;
			for (int j = 0; j < PatternIndices[i].size(); j++)
			{
				if (PatternIndices[i][j])
				{
					InstrumentUsed[i] = true;
					break;
				}
			}
		}
		// determine max instrument count and create a mapping instrument index -> used instrument index
		int InstrumentIndex[MAX_INSTRUMENTS];
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
		{
			// dummy init
			InstrumentIndex[i] = -1;
			if (InstrumentUsed[i])
			{
				InstrumentIndex[i] = maxinst;
				maxinst++;
			}

		}
		std::vector<WORD> delay_times;
		std::vector<int> delay_indices;
		bool hasReverb = false;
		bool hasNoteSync = false;
		// add notesync and reverb times
		for (int i = 0; i < 17; i++)
		{
			DWORD times = (&go4k_delay_times)[i];
			delay_times.push_back(times);
			delay_indices.push_back(i);
		}
		delay_times[0] = 0;
		for (int i = 0; i <= MAX_INSTRUMENTS; i++)
		{
			for (int u = 0; u < MAX_SLOTS; u++)
			{
				//	// used instrument or global?
				if (InstrumentUsed[i] || i == MAX_INSTRUMENTS)
				{
					DLL_valP v;
					if (i < MAX_INSTRUMENTS)
						v = (DLL_valP)(SynthObj.InstrumentValues[i][u]);
					else
						v = (DLL_valP)(SynthObj.GlobalValues[u]);

					if (v->id == M_DLL)
					{
						if (!v->reverb)
						{
							// if not notesync
							if (v->synctype != 2)
							{
								DWORD times = (&go4k_delay_times)[delay_indices.size()];
								// check if indexed value already existed
								int found = -1;
								for (int j = 17; j < delay_times.size(); j++)
								{
									if (delay_times[j] == times)
									{
										found = j;
										break;
									}
								}
								if (found != -1)
								{
									// already in list, so let index point to that one
									delay_indices.push_back(found);
								}
								else
								{
									// new value, so push it
									delay_times.push_back(times);
									delay_indices.push_back(delay_times.size()-1);
								}
							}
							else
							{
								hasNoteSync = true;
								delay_indices.push_back(0);
							}
						}
						else
						{
							hasReverb = true;
						}
					}
				}
				// no used instrument
				else
				{
					DLL_valP v;
					v = (DLL_valP)(SynthObj.InstrumentValues[i][u]);
					if (v->id == M_DLL)
					{
						if (!v->reverb)
						{
							// if not notesync
							if (v->synctype != 2)
							{
								// just push a dummy index
								delay_indices.push_back(-1);
							}
						}
					}
				}
			}
		}
		// if we dont have reverb, remove values and adjust indices
		if (!hasReverb)
		{
			// move values
			for (int i = 17; i < delay_times.size(); i++)
			{
				delay_times[i-16] = delay_times[i];
			}
			// remove obsolete values
			for (int i = 0; i < 16; i++)
			{
				delay_times.pop_back();
			}
			// adjust indices
			for (int i = 0; i < delay_indices.size(); i++)
			{
				if (delay_indices[i] != 0)
					delay_indices[i] -= 16;
			}
		}
		// if we dont have notesync, remove values and adjust indices
		if (!hasNoteSync)
		{
			// move values
			for (int i = 1; i < delay_times.size(); i++)
			{
				delay_times[i-1] = delay_times[i];
			}
			// remove obsolete values
			delay_times.pop_back();
			// adjust indices
			for (int i = 0; i < delay_indices.size(); i++)
			{
				delay_indices[i] -= 1;
			}
		}

		SynthUses uses;
		memset(&uses, 0, sizeof(SynthUses));
		GetUses(&uses, InstrumentUsed);

		// write inc file
		fprintf(file, "%%macro export_func 1\n");
//		fprintf(file, "   %%define %%1 _%%1\n");
		fprintf(file, "   global _%%1\n");
		fprintf(file, "   _%%1:\n");
		fprintf(file, "%%endmacro\n");

//		fprintf(file, "%%macro export_dword_array 2\n");
//		fprintf(file, "   %%define %%1 _%%1\n");
//		fprintf(file, "   global %%1\n");
//		fprintf(file, "   %%1 resd %%2\n");
//		fprintf(file, "%%endmacro\n");

/*		if (objformat == 0)
			MessageBox(0,"WINDOWS","",MB_OK);
		if (objformat == 1)
			MessageBox(0,"LINUX","",MB_OK);
		if (objformat == 2)
			MessageBox(0,"MACOSX","",MB_OK);
*/

	if (objformat == 0) // 0:windows, 1:linux, 2:osx
	{
		// use sections only for windows, 
		// linux doesnt have any crinkler like packer to take advantage of multiple sections
		// and osx export has a bug in current nasm and is not able to export custom sections
		fprintf(file, "%%define USE_SECTIONS\n");
	}

		fprintf(file, "%%define SAMPLE_RATE	%d\n", 44100);
		fprintf(file, "%%define MAX_INSTRUMENTS	%d\n", maxinst);
		fprintf(file, "%%define MAX_VOICES %d\n", SynthObj.Polyphony);
		fprintf(file, "%%define HLD 1\n");
		fprintf(file, "%%define BPM %f\n", BeatsPerMinute);
		fprintf(file, "%%define MAX_PATTERNS %d\n", PatternIndices[0].size());
		fprintf(file, "%%define PATTERN_SIZE_SHIFT %d\n", GetShift2(PatternSize));
		fprintf(file, "%%define PATTERN_SIZE (1 << PATTERN_SIZE_SHIFT)\n");
		fprintf(file, "%%define	MAX_TICKS (MAX_PATTERNS*PATTERN_SIZE)\n");
		fprintf(file, "%%define	SAMPLES_PER_TICK %d\n", (int)(44100.0f*4.0f*60.0f/(BeatsPerMinute*16*TickScaler)));
		fprintf(file, "%%define DEF_LFO_NORMALIZE %.10f\n", LFO_NORMALIZE);
		fprintf(file, "%%define	MAX_SAMPLES	(SAMPLES_PER_TICK*MAX_TICKS)\n");
		fprintf(file, "%s%%define 	GO4K_USE_16BIT_OUTPUT\n",output16?"":";");
		fprintf(file, ";%%define 	GO4K_USE_GROOVE_PATTERN\n");
		fprintf(file, "%s%%define 	GO4K_USE_ENVELOPE_RECORDINGS\n",useenvlevels?"":";");
		fprintf(file, "%s%%define 	GO4K_USE_NOTE_RECORDINGS\n",useenotevalues?"":";");
	if (undenormalize)
		fprintf(file, "%%define 	GO4K_USE_UNDENORMALIZE\n");
	if (clipoutput)
		fprintf(file, "%%define 	GO4K_CLIP_OUTPUT\n");
	if (uses.dst_use)
		fprintf(file, "%%define 	GO4K_USE_DST\n");
	if (uses.dll_use)
		fprintf(file, "%%define 	GO4K_USE_DLL\n");
	if (uses.pan_use)
		fprintf(file, "%%define 	GO4K_USE_PAN\n");
		fprintf(file, "%%define 	GO4K_USE_GLOBAL_DLL\n");
	if (uses.fstg_use)
		fprintf(file, "%%define 	GO4K_USE_FSTG\n");
	if (uses.fld_use)
		fprintf(file, "%%define 	GO4K_USE_FLD\n");
		fprintf(file, "%%define 	GO4K_USE_ENV_CHECK\n");
	if (uses.env_gm)
		fprintf(file, "%%define 	GO4K_USE_ENV_MOD_GM\n");
	if (uses.env_adrm)
		fprintf(file, "%%define		GO4K_USE_ENV_MOD_ADR\n");
		fprintf(file, "%%define 	GO4K_USE_VCO_CHECK\n");
	if (uses.vco_phaseofs)
		fprintf(file, "%%define 	GO4K_USE_VCO_PHASE_OFFSET\n");
	if (uses.vco_shape)
		fprintf(file, "%%define 	GO4K_USE_VCO_SHAPE\n");
	if (uses.vco_gate)
		fprintf(file, "%%define		GO4K_USE_VCO_GATE\n");
	if (uses.vco_fm)
		fprintf(file, "%%define 	GO4K_USE_VCO_MOD_FM\n");
	if (uses.vco_pm)
		fprintf(file, "%%define 	GO4K_USE_VCO_MOD_PM\n");
	if (uses.vco_tm)
		fprintf(file, "%%define 	GO4K_USE_VCO_MOD_TM\n");
	if (uses.vco_dm)
		fprintf(file, "%%define 	GO4K_USE_VCO_MOD_DM\n");
	if (uses.vco_cm)
		fprintf(file, "%%define 	GO4K_USE_VCO_MOD_CM\n");
	if (uses.vco_gm)
		fprintf(file, "%%define 	GO4K_USE_VCO_MOD_GM\n");
	if (uses.vco_sm)
		fprintf(file, "%%define 	GO4K_USE_VCO_MOD_SM\n");
		fprintf(file, "%%define 	GO4K_USE_VCF_CHECK\n");
	if (uses.vcf_fm)
		fprintf(file, "%%define 	GO4K_USE_VCF_MOD_FM\n");
	if (uses.vcf_rm)
		fprintf(file, "%%define 	GO4K_USE_VCF_MOD_RM\n");
		fprintf(file, "%%define 	GO4K_USE_VCF_HIGH\n");
		fprintf(file, "%%define 	GO4K_USE_VCF_BAND\n");
		fprintf(file, "%%define 	GO4K_USE_VCF_PEAK\n");
		fprintf(file, "%%define 	GO4K_USE_DST_CHECK\n");
	if (uses.dst_snh)
		fprintf(file, "%%define 	GO4K_USE_DST_SH\n");
	if (uses.dst_dm)
		fprintf(file, "%%define 	GO4K_USE_DST_MOD_DM\n");
	if (uses.dst_sm)
		fprintf(file, "%%define 	GO4K_USE_DST_MOD_SH\n");
	if (uses.dll_notesync)
		fprintf(file, "%%define		GO4K_USE_DLL_NOTE_SYNC\n");
	if (uses.dll_chorus)
		fprintf(file, "%%define		GO4K_USE_DLL_CHORUS\n");
		fprintf(file, "%%define		GO4K_USE_DLL_CHORUS_CLAMP\n");
	if (uses.dll_damp)
		fprintf(file, "%%define 	GO4K_USE_DLL_DAMP\n");
		fprintf(file, "%%define 	GO4K_USE_DLL_DC_FILTER\n");
	if (uses.fop_loadnote)
		fprintf(file, "%%define		GO4K_USE_FOP_LOADNOTE\n");
		fprintf(file, "%%define 	GO4K_USE_FSTG_CHECK\n");
	if (uses.pan_pm)
		fprintf(file, "%%define 	GO4K_USE_PAN_MOD\n");
	if (uses.out_am)
		fprintf(file, "%%define 	GO4K_USE_OUT_MOD_AM\n");
	if (uses.out_gm)
		fprintf(file, "%%define 	GO4K_USE_OUT_MOD_GM\n");
		fprintf(file, "%%define		GO4K_USE_WAVESHAPER_CLIP\n");
	if (uses.fld_vm)
		fprintf(file, "%%define 	GO4K_USE_FLD_MOD_VM\n");
	if (uses.dll_use_mod)
		fprintf(file, "%%define 	GO4K_USE_DLL_MOD\n");
	if (uses.dll_pm)
		fprintf(file, "%%define 	GO4K_USE_DLL_MOD_PM\n");
	if (uses.dll_fm)
		fprintf(file, "%%define 	GO4K_USE_DLL_MOD_FM\n");
	if (uses.dll_im)
		fprintf(file, "%%define 	GO4K_USE_DLL_MOD_IM\n");	
	if (uses.dll_dm)
		fprintf(file, "%%define 	GO4K_USE_DLL_MOD_DM\n");
	if (uses.dll_sm)
		fprintf(file, "%%define 	GO4K_USE_DLL_MOD_SM\n");
	if (uses.dll_am)
		fprintf(file, "%%define 	GO4K_USE_DLL_MOD_AM\n");

		fprintf(file, "%%define	MAX_DELAY			65536\n");
		fprintf(file, "%%define MAX_WORKSPACE_SLOTS	8\n");
		fprintf(file, "%%define GO4K_BEGIN_CMDDEF(def_name)\n");
		fprintf(file, "%%define GO4K_END_CMDDEF db 0\n");
		fprintf(file, "%%define GO4K_BEGIN_PARAMDEF(def_name)\n");
		fprintf(file, "%%define GO4K_END_PARAMDEF\n");

		fprintf(file, "GO4K_ENV_ID		equ		1\n");
		fprintf(file, "%%macro	GO4K_ENV 5\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "	db	%%2\n");
		fprintf(file, "	db	%%3\n");
		fprintf(file, "	db	%%4\n");
		fprintf(file, "	db	%%5\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define	ATTAC(val)		val	\n");
		fprintf(file, "%%define	DECAY(val)		val	\n");
		fprintf(file, "%%define	SUSTAIN(val)	val	\n");
		fprintf(file, "%%define	RELEASE(val)	val	\n");
		fprintf(file, "%%define	GAIN(val)		val	\n");
		fprintf(file, "struc	go4kENV_val\n");
		fprintf(file, "	.attac		resd	1\n");
		fprintf(file, "	.decay		resd	1\n");
		fprintf(file, "	.sustain	resd	1\n");
		fprintf(file, "	.release	resd	1\n");
		fprintf(file, "	.gain		resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kENV_wrk\n");
		fprintf(file, "	.state		resd	1\n");
		fprintf(file, "	.level		resd	1\n");
		fprintf(file, "	.gm			resd	1\n");
		fprintf(file, "	.am			resd	1\n");
		fprintf(file, "	.dm			resd	1\n");
		fprintf(file, "	.sm			resd	1\n");
		fprintf(file, "	.rm			resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "%%define ENV_STATE_ATTAC		0\n");
		fprintf(file, "%%define ENV_STATE_DECAY		1\n");
		fprintf(file, "%%define ENV_STATE_SUSTAIN	2\n");
		fprintf(file, "%%define ENV_STATE_RELEASE	3\n");
		fprintf(file, "%%define ENV_STATE_OFF		4\n");

		fprintf(file, "GO4K_VCO_ID		equ		2\n");
		fprintf(file, "%%macro	GO4K_VCO 8\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "	db	%%2\n");
		fprintf(file, "%%ifdef GO4K_USE_VCO_PHASE_OFFSET	\n");
		fprintf(file, "	db	%%3\n");
		fprintf(file, "%%endif	\n");
		fprintf(file, "%%ifdef GO4K_USE_VCO_GATE	\n");
		fprintf(file, "	db	%%4\n");
		fprintf(file, "%%endif	\n");
		fprintf(file, "	db	%%5\n");
		fprintf(file, "%%ifdef GO4K_USE_VCO_SHAPE	\n");
		fprintf(file, "	db	%%6\n");
		fprintf(file, "%%endif	\n");
		fprintf(file, "	db	%%7\n");
		fprintf(file, "	db	%%8\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define	TRANSPOSE(val)	val	\n");
		fprintf(file, "%%define	DETUNE(val)		val	\n");
		fprintf(file, "%%define	PHASE(val)		val	\n");
		fprintf(file, "%%define	GATES(val)		val	\n");
		fprintf(file, "%%define	COLOR(val)		val	\n");
		fprintf(file, "%%define	SHAPE(val)		val \n");
		fprintf(file, "%%define	FLAGS(val)		val	\n");
		fprintf(file, "%%define SINE		0x01\n");
		fprintf(file, "%%define TRISAW		0x02\n");
		fprintf(file, "%%define PULSE		0x04\n");
		fprintf(file, "%%define NOISE		0x08\n");
		fprintf(file, "%%define LFO			0x10\n");
		fprintf(file, "%%define GATE		0x20\n");
		fprintf(file, "struc	go4kVCO_val\n");
		fprintf(file, "	.transpose	resd	1\n");
		fprintf(file, "	.detune		resd	1\n");
		fprintf(file, "%%ifdef GO4K_USE_VCO_PHASE_OFFSET	\n");
		fprintf(file, "	.phaseofs	resd	1\n");
		fprintf(file, "%%endif	\n");	
		fprintf(file, "%%ifdef GO4K_USE_VCO_GATE	\n");
		fprintf(file, "	.gate		resd	1\n");
		fprintf(file, "%%endif	\n");
		fprintf(file, "	.color		resd	1\n");
		fprintf(file, "%%ifdef GO4K_USE_VCO_SHAPE	\n");
		fprintf(file, "	.shape		resd	1\n");
		fprintf(file, "%%endif	\n");
		fprintf(file, "	.gain		resd	1\n");
		fprintf(file, "	.flags		resd	1	\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kVCO_wrk\n");
		fprintf(file, "	.phase		resd	1\n");
		fprintf(file, "	.tm			resd	1\n");
		fprintf(file, "	.dm			resd	1\n");
		fprintf(file, "	.fm			resd	1\n");
		fprintf(file, "	.pm			resd	1\n");
		fprintf(file, "	.cm			resd	1\n");
		fprintf(file, "	.sm			resd	1\n");
		fprintf(file, "	.gm			resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "GO4K_VCF_ID		equ		3\n");
		fprintf(file, "%%macro	GO4K_VCF 3\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "	db	%%2\n");
		fprintf(file, "	db	%%3\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define LOWPASS		0x1\n");
		fprintf(file, "%%define HIGHPASS	0x2\n");
		fprintf(file, "%%define BANDPASS	0x4\n");
		fprintf(file, "%%define	BANDSTOP	0x3\n");
		fprintf(file, "%%define ALLPASS		0x7\n");
		fprintf(file, "%%define	PEAK		0x8\n");
		fprintf(file, "%%define	FREQUENCY(val)	val\n");
		fprintf(file, "%%define	RESONANCE(val)	val\n");
		fprintf(file, "%%define	VCFTYPE(val)	val\n");
		fprintf(file, "struc	go4kVCF_val\n");
		fprintf(file, "	.freq		resd	1\n");
		fprintf(file, "	.res		resd	1\n");
		fprintf(file, "	.type		resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kVCF_wrk\n");
		fprintf(file, "	.low		resd	1\n");
		fprintf(file, "	.high		resd	1\n");
		fprintf(file, "	.band		resd	1\n");
		fprintf(file, "	.freq		resd	1\n");
		fprintf(file, "	.fm			resd	1\n");
		fprintf(file, "	.rm			resd	1	\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "GO4K_DST_ID		equ		4\n");
		fprintf(file, "%%macro	GO4K_DST 2\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "%%ifdef GO4K_USE_DST_SH	\n");
		fprintf(file, "	db	%%2\n");
		fprintf(file, "%%endif	\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define	DRIVE(val)		val\n");
		fprintf(file, "%%define	SNHFREQ(val)	val\n");
		fprintf(file, "struc	go4kDST_val\n");
		fprintf(file, "	.drive		resd	1\n");
		fprintf(file, "%%ifdef GO4K_USE_DST_SH	\n");
		fprintf(file, "	.snhfreq	resd	1\n");
		fprintf(file, "%%endif	\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kDST_wrk\n");
		fprintf(file, "%%ifdef GO4K_USE_DST_SH	\n");
		fprintf(file, "	.out		resd	1\n");
		fprintf(file, "	.snhphase	resd	1\n");
		fprintf(file, "%%endif	\n");
		fprintf(file, "	.dm			resd	1\n");
		fprintf(file, "	.sm			resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "GO4K_DLL_ID		equ		5\n");
		fprintf(file, "%%macro	GO4K_DLL 8\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "	db	%%2\n");
		fprintf(file, "	db	%%3\n");
		fprintf(file, "%%ifdef GO4K_USE_DLL_DAMP	\n");
		fprintf(file, "	db	%%4\n");
		fprintf(file, "%%endif		\n");
		fprintf(file, "%%ifdef GO4K_USE_DLL_CHORUS	\n");
		fprintf(file, "	db	%%5\n");
		fprintf(file, "	db	%%6\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "	db	%%7\n");
		fprintf(file, "	db	%%8\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define PREGAIN(val)	val\n");
		fprintf(file, "%%define	DRY(val)		val\n");
		fprintf(file, "%%define	FEEDBACK(val)	val\n");
		fprintf(file, "%%define	DEPTH(val)		val\n");
		fprintf(file, "%%define DAMP(val)		val\n");
		fprintf(file, "%%define	DELAY(val)		val\n");
		fprintf(file, "%%define	COUNT(val)		val\n");
		fprintf(file, "struc	go4kDLL_val\n");
		fprintf(file, "	.pregain	resd	1\n");
		fprintf(file, "	.dry		resd	1\n");
		fprintf(file, "	.feedback	resd	1\n");
		fprintf(file, "%%ifdef GO4K_USE_DLL_DAMP	\n");
		fprintf(file, "	.damp		resd	1	\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "%%ifdef GO4K_USE_DLL_CHORUS\n");
		fprintf(file, "	.freq		resd	1\n");
		fprintf(file, "	.depth\n");
		fprintf(file, "%%endif\n");	
		fprintf(file, "	.delay		resd	1\n");
		fprintf(file, "	.count		resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kDLL_wrk\n");
		fprintf(file, "	.index		resd	1\n");
		fprintf(file, "	.store		resd	1\n");
		fprintf(file, "	.dcin		resd	1\n");
		fprintf(file, "	.dcout		resd	1\n");
		fprintf(file, "%%ifdef GO4K_USE_DLL_CHORUS\n");
		fprintf(file, "	.phase		resd	1\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "	.buffer		resd	MAX_DELAY\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "GO4K_FOP_ID	equ			6\n");
		fprintf(file, "%%macro	GO4K_FOP 1\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define	OP(val)			val\n");
		fprintf(file, "%%define FOP_POP			0x1\n");
		fprintf(file, "%%define FOP_ADDP		0x2\n");
		fprintf(file, "%%define FOP_MULP		0x3\n");
		fprintf(file, "%%define FOP_PUSH		0x4\n");
		fprintf(file, "%%define FOP_XCH			0x5\n");
		fprintf(file, "%%define FOP_ADD			0x6\n");
		fprintf(file, "%%define FOP_MUL			0x7\n");
		fprintf(file, "%%define FOP_ADDP2		0x8\n");
		fprintf(file, "%%define FOP_LOADNOTE	0x9\n");
		fprintf(file, "struc	go4kFOP_val\n");
		fprintf(file, "	.flags		resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kFOP_wrk\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "GO4K_FST_ID		equ		7\n");
		fprintf(file, "%%macro	GO4K_FST 2\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "	db	%%2\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define	AMOUNT(val)		val\n");
		fprintf(file, "%%define	DEST(val)		val\n");
		fprintf(file, "struc	go4kFST_val\n");
		fprintf(file, "	.amount		resd	1\n");
		fprintf(file, "	.op1		resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kFST_wrk\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "GO4K_PAN_ID		equ		8\n");
		fprintf(file, "%%macro	GO4K_PAN 1\n");
		fprintf(file, "%%ifdef GO4K_USE_PAN\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define	PANNING(val)	val\n");
		fprintf(file, "struc	go4kPAN_val\n");
		fprintf(file, "%%ifdef GO4K_USE_PAN\n");
		fprintf(file, "	.panning	resd	1\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kPAN_wrk\n");
		fprintf(file, "	.pm			resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "GO4K_OUT_ID		equ		9\n");
		fprintf(file, "%%macro	GO4K_OUT 2\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "%%ifdef GO4K_USE_GLOBAL_DLL	\n");
		fprintf(file, "	db	%%2\n");
		fprintf(file, "%%endif	\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define	AUXSEND(val)	val\n");
		fprintf(file, "struc	go4kOUT_val\n");
		fprintf(file, "	.gain		resd	1\n");
		fprintf(file, "%%ifdef GO4K_USE_GLOBAL_DLL	\n");
		fprintf(file, "	.auxsend	resd	1\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kOUT_wrk\n");
		fprintf(file, "	.am			resd	1\n");
		fprintf(file, "	.gm			resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "GO4K_ACC_ID		equ		10\n");
		fprintf(file, "%%macro	GO4K_ACC 1\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define OUTPUT			0\n");
		fprintf(file, "%%define	AUX				8\n");
		fprintf(file, "%%define ACCTYPE(val)	val\n");
		fprintf(file, "struc	go4kACC_val\n");
		fprintf(file, "	.acctype	resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kACC_wrk\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "%%ifdef GO4K_USE_FSTG\n");
		fprintf(file, "GO4K_FSTG_ID	equ		11\n");
		fprintf(file, "%%macro	GO4K_FSTG 2\n");
		fprintf(file, "	db	%%1\n");
		fprintf(file, "	dd	%%2\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "struc	go4kFSTG_val\n");
		fprintf(file, "	.amount		resd	1\n");
		fprintf(file, "	.op1		resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kFSTG_wrk\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "%%endif\n");

		fprintf(file, "%%ifdef GO4K_USE_FLD\n");
		fprintf(file, "GO4K_FLD_ID	equ		12\n");
		fprintf(file, "%%macro	GO4K_FLD 1\n");
		fprintf(file, "	db	%1\n");
		fprintf(file, "%%endmacro\n");
		fprintf(file, "%%define	VALUE(val)	val\n");
		fprintf(file, "struc	go4kFLD_val\n");
		fprintf(file, "	.value		resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "struc	go4kFLD_wrk\n");
		fprintf(file, "	.vm			resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
		fprintf(file, "%%endif\n");

		fprintf(file, "struc	go4k_instrument\n");
		fprintf(file, "	.release	resd	1\n");
		fprintf(file, "	.note		resd	1\n");
		fprintf(file, "	.workspace	resd	256\n");
		fprintf(file, "	.dlloutl	resd	1\n");
		fprintf(file, "	.dlloutr	resd	1\n");
		fprintf(file, "	.outl		resd	1\n");
		fprintf(file, "	.outr		resd	1\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");

		fprintf(file, "struc	go4k_synth\n");
		fprintf(file, "	.instruments	resb	go4k_instrument.size * MAX_INSTRUMENTS * MAX_VOICES\n");
		fprintf(file, "	.global			resb	go4k_instrument.size * MAX_VOICES\n");
		fprintf(file, "	.size\n");
		fprintf(file, "endstruc\n");
#define INDEXED_PATTERN_VALUES__
#ifdef INDEXED_PATTERN_VALUES
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// the pattern values
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		fprintf(file, "; //----------------------------------------------------------------------------------------\n");
		fprintf(file, "; // Pattern Values, sorted by quantity\n");
		fprintf(file, "; //----------------------------------------------------------------------------------------\n");
		fprintf(file, "%%ifdef USE_SECTIONS\n");
		fprintf(file, "section .g4kmuc0 data align=1\n");
		fprintf(file, "%%else\n");
		fprintf(file, "section .data\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "go4k_pattern_values\n");
		std::map<int, int> noteCountMap;
		for (int i = 0; i < ReducedPatterns.size()/PatternSize; i++)
		{
			for (int j = 0; j < PatternSize; j++)
			{
				int note = ReducedPatterns[i*PatternSize+j];
				if (note == -1)
					note = 1;
				if (noteCountMap.find(note) != noteCountMap.end())
				{
					noteCountMap[note] = noteCountMap[note] + 1;
				}
				else
				{
					noteCountMap[note] = 1;
				}
			}
		}
		std::list<std::pair<int, int> > noteIndexList;
		std::map<int, int>::iterator it;
		for (it = noteCountMap.begin(); it != noteCountMap.end(); it++)
		{
			std::pair<int, int> sval(it->second, it->first);
			noteIndexList.push_back(sval);
		}
		noteIndexList.sort();
		std::list<std::pair<int, int> >::reverse_iterator rit;
		fprintf(file, "\tdb\t");
		for (rit = noteIndexList.rbegin(); rit != noteIndexList.rend(); rit++)
		{
			fprintf(file, "%d, ", (*rit).second);
		}
#endif
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// the patterns
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		fprintf(file, "%%ifdef USE_SECTIONS\n");
		fprintf(file, "section .g4kmuc1 data align=1\n");
		fprintf(file, "%%else\n");
		fprintf(file, "section .data align=1\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "go4k_patterns\n");
		for (int i = 0; i < ReducedPatterns.size()/PatternSize; i++)
		{
			fprintf(file, "\tdb\t");
			for (int j = 0; j < PatternSize; j++)
			{
#ifdef INDEXED_PATTERN_VALUES
				int noteindex = 0;
				for (rit = noteIndexList.rbegin(); rit != noteIndexList.rend(); rit++)
				{
					// TODO: check for hold
					if (ReducedPatterns[i*PatternSize+j] == (*rit).second)
						fprintf(file, "%d, ", noteindex);
					noteindex++;
				}
#else
				if (ReducedPatterns[i*PatternSize+j] >= 0)
					fprintf(file, "%d, ", ReducedPatterns[i*PatternSize+j]);
				else
					fprintf(file, "HLD, ");
#endif
			}		
			fprintf(file, "\n");			
		}
		fprintf(file, "go4k_patterns_end\n");
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// the pattern indices
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		fprintf(file, "%%ifdef USE_SECTIONS\n");
		fprintf(file, "section .g4kmuc2 data align=1\n");
		fprintf(file, "%%else\n");
		fprintf(file, "section .data\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "go4k_pattern_lists\n");
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
		{
			if (!InstrumentUsed[i]) continue;
			fprintf(file, "Instrument%dList\t\tdb\t", i);
			for (int j = 0; j < PatternIndices[i].size(); j++)
			{
				fprintf(file, "%d, ", PatternIndices[i][j]);
			}
			fprintf(file, "\n");
		}
		fprintf(file, "go4k_pattern_lists_end\n");
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// the instrument commands
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		fprintf(file, "%%ifdef USE_SECTIONS\n");
		fprintf(file, "section .g4kmuc3 data align=1\n");
		fprintf(file, "%%else\n");
		fprintf(file, "section .data\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "go4k_synth_instructions\n");
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
		{
			if (!InstrumentUsed[i]) continue;
			fprintf(file, "GO4K_BEGIN_CMDDEF(Instrument%d)\n", i);
			for (int u = 0; u < MAX_SLOTS; u++)
			{
				if (SynthObj.InstrumentValues[i][u][0] == M_ENV)
					fprintf(file, "\tdb GO4K_ENV_ID\n");
				if (SynthObj.InstrumentValues[i][u][0] == M_VCO)
					fprintf(file, "\tdb GO4K_VCO_ID\n");
				if (SynthObj.InstrumentValues[i][u][0] == M_VCF)
					fprintf(file, "\tdb GO4K_VCF_ID\n");
				if (SynthObj.InstrumentValues[i][u][0] == M_DST)
					fprintf(file, "\tdb GO4K_DST_ID\n");
				if (SynthObj.InstrumentValues[i][u][0] == M_DLL)
					fprintf(file, "\tdb GO4K_DLL_ID\n");
				if (SynthObj.InstrumentValues[i][u][0] == M_FOP)
					fprintf(file, "\tdb GO4K_FOP_ID\n");
				if (SynthObj.InstrumentValues[i][u][0] == M_FST)
				{
					FST_valP v = (FST_valP)(SynthObj.InstrumentValues[i][u]);
					// local storage
					if (v->dest_stack == -1 || v->dest_stack == i)
						fprintf(file, "\tdb GO4K_FST_ID\n");
					// global storage
					else
						fprintf(file, "\tdb GO4K_FSTG_ID\n");
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_PAN)
					fprintf(file, "\tdb GO4K_PAN_ID\n");
				if (SynthObj.InstrumentValues[i][u][0] == M_OUT)
					fprintf(file, "\tdb GO4K_OUT_ID\n");
				if (SynthObj.InstrumentValues[i][u][0] == M_ACC)
					fprintf(file, "\tdb GO4K_ACC_ID\n");
				if (SynthObj.InstrumentValues[i][u][0] == M_FLD)
					fprintf(file, "\tdb GO4K_FLD_ID\n");
			}
			fprintf(file, "GO4K_END_CMDDEF\n");
		};
		fprintf(file, "GO4K_BEGIN_CMDDEF(Global)\n");
		for (int u = 0; u < MAX_SLOTS; u++)
		{
			if (SynthObj.GlobalValues[u][0] == M_ENV)
				fprintf(file, "\tdb GO4K_ENV_ID\n");
			if (SynthObj.GlobalValues[u][0] == M_VCO)
				fprintf(file, "\tdb GO4K_VCO_ID\n");
			if (SynthObj.GlobalValues[u][0] == M_VCF)
				fprintf(file, "\tdb GO4K_VCF_ID\n");
			if (SynthObj.GlobalValues[u][0] == M_DST)
				fprintf(file, "\tdb GO4K_DST_ID\n");
			if (SynthObj.GlobalValues[u][0] == M_DLL)
				fprintf(file, "\tdb GO4K_DLL_ID\n");
			if (SynthObj.GlobalValues[u][0] == M_FOP)
				fprintf(file, "\tdb GO4K_FOP_ID\n");
			if (SynthObj.GlobalValues[u][0] == M_FST)
			{
				FST_valP v = (FST_valP)(SynthObj.GlobalValues[u]);
				// local storage
				if (v->dest_stack == -1 || v->dest_stack == MAX_INSTRUMENTS)
					fprintf(file, "\tdb GO4K_FST_ID\n");
				// global storage
				else
					fprintf(file, "\tdb GO4K_FSTG_ID\n");
			}
			if (SynthObj.GlobalValues[u][0] == M_PAN)
				fprintf(file, "\tdb GO4K_PAN_ID\n");
			if (SynthObj.GlobalValues[u][0] == M_OUT)
				fprintf(file, "\tdb GO4K_OUT_ID\n");
			if (SynthObj.GlobalValues[u][0] == M_ACC)
				fprintf(file, "\tdb GO4K_ACC_ID\n");
			if (SynthObj.GlobalValues[u][0] == M_FLD)
				fprintf(file, "\tdb GO4K_FLD_ID\n");
		}
		fprintf(file, "GO4K_END_CMDDEF\n");
		fprintf(file, "go4k_synth_instructions_end\n");
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// the instrument data
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		fprintf(file, "%%ifdef USE_SECTIONS\n");
		fprintf(file, "section .g4kmuc4 data align=1\n");
		fprintf(file, "%%else\n");
		fprintf(file, "section .data\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "go4k_synth_parameter_values\n");
		int delayindex = 0;
		for (int i = 0; i < MAX_INSTRUMENTS; i++)
		{
			if (!InstrumentUsed[i]) continue;
			fprintf(file, "GO4K_BEGIN_PARAMDEF(Instrument%d)\n", i);
			for (int u = 0; u < MAX_SLOTS; u++)
			{
				if (SynthObj.InstrumentValues[i][u][0] == M_ENV)
				{
					ENV_valP v = (ENV_valP)(SynthObj.InstrumentValues[i][u]);
					fprintf(file, "\tGO4K_ENV\tATTAC(%d),DECAY(%d),SUSTAIN(%d),RELEASE(%d),GAIN(%d)\n", v->attac, v->decay, v->sustain, v->release, v->gain);
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_VCO)
				{
					VCO_valP v = (VCO_valP)(SynthObj.InstrumentValues[i][u]);
					char type[10]; type[0] = 0;
					char lfo[10]; lfo[0] = 0;
					if (v->flags & VCO_SINE)
						sprintf(type, "SINE");
					if (v->flags & VCO_TRISAW)
						sprintf(type, "TRISAW");
					if (v->flags & VCO_PULSE)
						sprintf(type, "PULSE");
					if (v->flags & VCO_NOISE)
						sprintf(type, "NOISE");
					if (v->flags & VCO_GATE)
						sprintf(type, "GATE");
					if (v->flags & VCO_LFO)
						sprintf(lfo, "|LFO");
					fprintf(file, "\tGO4K_VCO\tTRANSPOSE(%d),DETUNE(%d),PHASE(%d),GATES(%d),COLOR(%d),SHAPE(%d),GAIN(%d),FLAGS(%s%s)\n", v->transpose, v->detune, v->phaseofs, v->gate, v->color, v->shape, v->gain, type, lfo);
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_VCF)
				{
					VCF_valP v = (VCF_valP)(SynthObj.InstrumentValues[i][u]);
					char type[10]; type[0] = 0;
					if (v->type == VCF_LOWPASS)
						sprintf(type, "LOWPASS");
					if (v->type == VCF_HIGHPASS)
						sprintf(type, "HIGHPASS");
					if (v->type == VCF_BANDPASS)
						sprintf(type, "BANDPASS");
					if (v->type == VCF_BANDSTOP)
						sprintf(type, "BANDSTOP");
					if (v->type == VCF_ALLPASS)
						sprintf(type, "ALLPASS");
					if (v->type == VCF_PEAK)
						sprintf(type, "PEAK");
					fprintf(file, "\tGO4K_VCF\tFREQUENCY(%d),RESONANCE(%d),VCFTYPE(%s)\n", v->freq, v->res, type);
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_DST)
				{
					DST_valP v = (DST_valP)(SynthObj.InstrumentValues[i][u]);
					fprintf(file, "\tGO4K_DST\tDRIVE(%d), SNHFREQ(%d)\n", v->drive, v->snhfreq);
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_DLL)
				{
					DLL_valP v = (DLL_valP)(SynthObj.InstrumentValues[i][u]);
					if (v->delay < delay_indices.size())
					{
						fprintf(file, "\tGO4K_DLL\tPREGAIN(%d),DRY(%d),FEEDBACK(%d),DAMP(%d),FREQUENCY(%d),DEPTH(%d),DELAY(%d),COUNT(%d)\n", 
							v->pregain, v->dry, v->feedback, v->damp, v->freq, v->depth, delay_indices[v->delay], v->count);	
					}
					// error handling in case indices are fucked up
					else
					{
						fprintf(file, "\tGO4K_DLL\tPREGAIN(%d),DRY(%d),FEEDBACK(%d),DAMP(%d),FREQUENCY(%d),DEPTH(%d),DELAY(%d),COUNT(%d) ; ERROR\n", 
							v->pregain, v->dry, v->feedback, v->damp, v->freq, v->depth, v->delay, v->count);
					}
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_FOP)
				{
					FOP_valP v = (FOP_valP)(SynthObj.InstrumentValues[i][u]);
					char type[10]; type[0] = 0;
					if (v->flags == FOP_POP)
						sprintf(type, "FOP_POP");
					if (v->flags == FOP_PUSH)
						sprintf(type, "FOP_PUSH");
					if (v->flags == FOP_XCH)
						sprintf(type, "FOP_XCH");
					if (v->flags == FOP_ADD)
						sprintf(type, "FOP_ADD");
					if (v->flags == FOP_ADDP)
						sprintf(type, "FOP_ADDP");
					if (v->flags == FOP_MUL)
						sprintf(type, "FOP_MUL");
					if (v->flags == FOP_MULP)
						sprintf(type, "FOP_MULP");
					if (v->flags == FOP_ADDP2)
						sprintf(type, "FOP_ADDP2");
					if (v->flags == FOP_LOADNOTE)
						sprintf(type, "FOP_LOADNOTE");
					fprintf(file, "\tGO4K_FOP\tOP(%s)\n", type);
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_FST)
				{
					FST_valP v = (FST_valP)(SynthObj.InstrumentValues[i][u]);
					// local storage
					if (v->dest_stack == -1 || v->dest_stack == i)
					{
						// skip empty units on the way to target
						int emptySkip = 0;
						for (int e = 0; e < v->dest_unit; e++)
						{
							if (SynthObj.InstrumentValues[i][e][0] == M_NONE)
								emptySkip++;
						}
						fprintf(file, "\tGO4K_FST\tAMOUNT(%d),DEST(%d*MAX_WORKSPACE_SLOTS+%d)\n", v->amount, v->dest_unit-emptySkip, v->dest_slot );
					}
					// global storage
					else
					{
						int storestack;
						if (v->dest_stack == MAX_INSTRUMENTS)
							storestack = maxinst;
						else
							storestack = InstrumentIndex[v->dest_stack];
						// skip empty units on the way to target
						int emptySkip = 0;
						for (int e = 0; e < v->dest_unit; e++)
						{
							if (v->dest_stack == MAX_INSTRUMENTS)
							{
								if (SynthObj.GlobalValues[e][0] == M_NONE)
									emptySkip++;
							}
							else
							{
								if (SynthObj.InstrumentValues[v->dest_stack][e][0] == M_NONE)
									emptySkip++;
							}
						}
						// invalid store target, possibly due non usage of the target instrument
						if (storestack == -1)
						{
							fprintf(file, "\tGO4K_FSTG\tAMOUNT(0),DEST(7*4+go4k_instrument.workspace)\n");
						}
						else
						{
							fprintf(file, "\tGO4K_FSTG\tAMOUNT(%d),DEST(%d*go4k_instrument.size*MAX_VOICES+%d*MAX_WORKSPACE_SLOTS*4+%d*4+go4k_instrument.workspace)\n", v->amount, storestack, v->dest_unit-emptySkip, v->dest_slot);
						}
					}
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_PAN)
				{
					PAN_valP v = (PAN_valP)(SynthObj.InstrumentValues[i][u]);
					fprintf(file, "\tGO4K_PAN\tPANNING(%d)\n", v->panning);
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_OUT)
				{
					OUT_valP v = (OUT_valP)(SynthObj.InstrumentValues[i][u]);
					fprintf(file, "\tGO4K_OUT\tGAIN(%d), AUXSEND(%d)\n", v->gain, v->auxsend);
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_ACC)
				{
					ACC_valP v = (ACC_valP)(SynthObj.InstrumentValues[i][u]);
					if (v->flags == ACC_OUT)
						fprintf(file, "\tGO4K_ACC\tACCTYPE(OUTPUT)\n");
					else
						fprintf(file, "\tGO4K_ACC\tACCTYPE(AUX)\n");
				}
				if (SynthObj.InstrumentValues[i][u][0] == M_FLD)
				{
					FLD_valP v = (FLD_valP)(SynthObj.InstrumentValues[i][u]);
					fprintf(file, "\tGO4K_FLD\tVALUE(%d)\n", v->value);
				}
			}
			fprintf(file, "GO4K_END_PARAMDEF\n");
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// the global data
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		{
			fprintf(file, "GO4K_BEGIN_PARAMDEF(Global)\n");
			for (int u = 0; u < MAX_SLOTS; u++)
			{
				if (SynthObj.GlobalValues[u][0] == M_ENV)
				{
					ENV_valP v = (ENV_valP)(SynthObj.GlobalValues[u]);
					fprintf(file, "\tGO4K_ENV\tATTAC(%d),DECAY(%d),SUSTAIN(%d),RELEASE(%d),GAIN(%d)\n", v->attac, v->decay, v->sustain, v->release, v->gain);
				}
				if (SynthObj.GlobalValues[u][0] == M_VCO)
				{
					VCO_valP v = (VCO_valP)(SynthObj.GlobalValues[u]);
					char type[10]; type[0] = 0;
					char lfo[10]; lfo[0] = 0;
					if (v->flags & VCO_SINE)
						sprintf(type, "SINE");
					if (v->flags & VCO_TRISAW)
						sprintf(type, "TRISAW");
					if (v->flags & VCO_PULSE)
						sprintf(type, "PULSE");
					if (v->flags & VCO_NOISE)
						sprintf(type, "NOISE");
					if (v->flags & VCO_GATE)
						sprintf(type, "GATE");
					if (v->flags & VCO_LFO)
						sprintf(lfo, "|LFO");
					fprintf(file, "\tGO4K_VCO\tTRANSPOSE(%d),DETUNE(%d),PHASE(%d),GATES(%d),COLOR(%d),SHAPE(%d),GAIN(%d),FLAGS(%s%s)\n", v->transpose, v->detune, v->phaseofs, v->gate, v->color, v->shape, v->gain, type, lfo);
				}
				if (SynthObj.GlobalValues[u][0] == M_VCF)
				{
					VCF_valP v = (VCF_valP)(SynthObj.GlobalValues[u]);
					char type[10]; type[0] = 0;
					if (v->type == VCF_LOWPASS)
						sprintf(type, "LOWPASS");
					if (v->type == VCF_HIGHPASS)
						sprintf(type, "HIGHPASS");
					if (v->type == VCF_BANDPASS)
						sprintf(type, "BANDPASS");
					if (v->type == VCF_BANDSTOP)
						sprintf(type, "BANDSTOP");
					if (v->type == VCF_ALLPASS)
						sprintf(type, "ALLPASS");
					if (v->type == VCF_PEAK)
						sprintf(type, "PEAK");
					fprintf(file, "\tGO4K_VCF\tFREQUENCY(%d),RESONANCE(%d),VCFTYPE(%s)\n", v->freq, v->res, type);
				}
				if (SynthObj.GlobalValues[u][0] == M_DST)
				{
					DST_valP v = (DST_valP)(SynthObj.GlobalValues[u]);
					fprintf(file, "\tGO4K_DST\tDRIVE(%d), SNHFREQ(%d)\n", v->drive, v->snhfreq);
				}
				if (SynthObj.GlobalValues[u][0] == M_DLL)
				{
					DLL_valP v = (DLL_valP)(SynthObj.GlobalValues[u]);
					if (v->delay < delay_indices.size())
					{
						fprintf(file, "\tGO4K_DLL\tPREGAIN(%d),DRY(%d),FEEDBACK(%d),DAMP(%d),FREQUENCY(%d),DEPTH(%d),DELAY(%d),COUNT(%d)\n", 
							v->pregain, v->dry, v->feedback, v->damp, v->freq, v->depth, delay_indices[v->delay], v->count);	
					}
					// error handling in case indices are fucked up
					else
					{
						fprintf(file, "\tGO4K_DLL\tPREGAIN(%d),DRY(%d),FEEDBACK(%d),DAMP(%d),FREQUENCY(%d),DEPTH(%d),DELAY(%d),COUNT(%d) ; ERROR\n", 
							v->pregain, v->dry, v->feedback, v->damp, v->freq, v->depth, v->delay, v->count);
					}
				}
				if (SynthObj.GlobalValues[u][0] == M_FOP)
				{
					FOP_valP v = (FOP_valP)(SynthObj.GlobalValues[u]);
					char type[10]; type[0] = 0;
					if (v->flags == FOP_POP)
						sprintf(type, "FOP_POP");
					if (v->flags == FOP_PUSH)
						sprintf(type, "FOP_PUSH");
					if (v->flags == FOP_XCH)
						sprintf(type, "FOP_XCH");
					if (v->flags == FOP_ADD)
						sprintf(type, "FOP_ADD");
					if (v->flags == FOP_ADDP)
						sprintf(type, "FOP_ADDP");
					if (v->flags == FOP_MUL)
						sprintf(type, "FOP_MUL");
					if (v->flags == FOP_MULP)
						sprintf(type, "FOP_MULP");
					if (v->flags == FOP_ADDP2)
						sprintf(type, "FOP_ADDP2");
					if (v->flags == FOP_LOADNOTE)
						sprintf(type, "FOP_LOADNOTE");
					fprintf(file, "\tGO4K_FOP\tOP(%s)\n", type);
				}
				if (SynthObj.GlobalValues[u][0] == M_FST)
				{
					FST_valP v = (FST_valP)(SynthObj.GlobalValues[u]);
					// local storage
					if (v->dest_stack == -1 || v->dest_stack == MAX_INSTRUMENTS)
					{
						// skip empty units on the way to target
						int emptySkip = 0;
						for (int e = 0; e < v->dest_unit; e++)
						{
							if (SynthObj.GlobalValues[e][0] == M_NONE)
								emptySkip++;
						}
						fprintf(file, "\tGO4K_FST\tAMOUNT(%d),DEST(%d*MAX_WORKSPACE_SLOTS+%d)\n", v->amount, v->dest_unit-emptySkip, v->dest_slot );
					}
					// global storage
					else
					{
						int storestack = InstrumentIndex[v->dest_stack];
						// skip empty units on the way to target
						int emptySkip = 0;
						for (int e = 0; e < v->dest_unit; e++)
						{
							if (SynthObj.InstrumentValues[v->dest_stack][e][0] == M_NONE)
								emptySkip++;
						}
						// invalid store target, possibly due non usage of the target instrument
						if (storestack == -1)
						{
							fprintf(file, "\tGO4K_FSTG\tAMOUNT(0),DEST(7*4+go4k_instrument.workspace)\n");
						}
						else
						{
							fprintf(file, "\tGO4K_FSTG\tAMOUNT(%d),DEST(%d*go4k_instrument.size*MAX_VOICES+%d*MAX_WORKSPACE_SLOTS*4+%d*4+go4k_instrument.workspace)\n", v->amount, storestack, v->dest_unit-emptySkip, v->dest_slot );
						}
					}
				}
				if (SynthObj.GlobalValues[u][0] == M_PAN)
				{
					PAN_valP v = (PAN_valP)(SynthObj.GlobalValues[u]);
					fprintf(file, "\tGO4K_PAN\tPANNING(%d)\n", v->panning);
				}
				if (SynthObj.GlobalValues[u][0] == M_OUT)
				{
					OUT_valP v = (OUT_valP)(SynthObj.GlobalValues[u]);
					fprintf(file, "\tGO4K_OUT\tGAIN(%d), AUXSEND(%d)\n", v->gain, v->auxsend);
				}
				if (SynthObj.GlobalValues[u][0] == M_ACC)
				{
					ACC_valP v = (ACC_valP)(SynthObj.GlobalValues[u]);
					if (v->flags == ACC_OUT)
						fprintf(file, "\tGO4K_ACC\tACCTYPE(OUTPUT)\n");
					else
						fprintf(file, "\tGO4K_ACC\tACCTYPE(AUX)\n");
				}
				if (SynthObj.GlobalValues[u][0] == M_FLD)
				{
					FLD_valP v = (FLD_valP)(SynthObj.GlobalValues[u]);
					fprintf(file, "\tGO4K_FLD\tVALUE(%d)\n", v->value);
				}
			}
			fprintf(file, "GO4K_END_PARAMDEF\n");
		}
		fprintf(file, "go4k_synth_parameter_values_end\n");
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// delay line times
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// TODO: Reduction here
		fprintf(file, "%%ifdef USE_SECTIONS\n");
		fprintf(file, "section .g4kmuc5 data align=1\n");
		fprintf(file, "%%else\n");
		fprintf(file, "section .data\n");
		fprintf(file, "%%endif\n");
		fprintf(file, "%%ifdef GO4K_USE_DLL\n");
		fprintf(file, "global _go4k_delay_times\n");
		fprintf(file, "_go4k_delay_times\n");
		for (int i = 0; i < delay_times.size(); i++)
		{
			fprintf(file, "\tdw %d\n", delay_times[i]);
		}
		fprintf(file, "%%endif\n");

		fclose(file);
	}
#ifdef EXPORT_OBJECT_FILE
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// extract resources and compile song
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::string compfile = path + "c4klang.exe";
	std::string asmfile = path + "4klang.asm";

	// First find and load the nasm compiler resource           
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_COMP_RES), "BINARY");         
	HGLOBAL hFileResource = LoadResource(hInst, hResource);
	// Now open and map this to a disk file           
	unsigned char *pcompres = (unsigned char *)LockResource(hFileResource);           
	DWORD dwSize = SizeofResource(hInst, hResource);            
	// write content
	FILE *fcompfile = fopen(compfile.c_str(), "wb");
	int tindex = 0;
	while (dwSize--)
	{
		unsigned char r = *pcompres++;
		r -= CryptTable[tindex++];
		fwrite(&r, 1, 1, fcompfile);
	}
	fclose(fcompfile);

	// First find and load the asm resource
	hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_ASM_RES), "BINARY");         
	hFileResource = LoadResource(hInst, hResource);
	// Now open and map this to a disk file           
	unsigned char *pasmres = (unsigned char *)LockResource(hFileResource);           
	dwSize = SizeofResource(hInst, hResource);            
	// write content
	FILE *fasmfile = fopen(asmfile.c_str(), "wb");
	tindex = 0;
	while (dwSize--)
	{
		unsigned char r = *pasmres++;
		r -= CryptTable[tindex++];
		fwrite(&r, 1, 1, fasmfile);
	}
	fclose(fasmfile);
	
	// compile object
	char olddir[1024];
	GetCurrentDirectory(1024, olddir);
	SetCurrentDirectory(path.c_str());

	std::string comparg;
	if (objformat == 0) // windows obj
	{
		comparg = "c4klang.exe 4klang.asm -fwin32 -o\"" + fpath + "\"";
	}
	if (objformat == 1) // linux obj
	{
		comparg = "c4klang.exe 4klang.asm -felf32 -o\"" + fpath + "\"";
	}
	if (objformat == 2) // macosx obj
	{
		comparg = "c4klang.exe 4klang.asm -fmacho32 -o\"" + fpath + "\"";
	}
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
    // Start the child process. 
    if( !CreateProcess( NULL,   // No module name (use command line). 
        (LPSTR)comparg.c_str(), // Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        0,                // No creation flags. 
        NULL,             // Use parent's environment block. 
        NULL,             // Use parent's starting directory. 
        &si,              // Pointer to STARTUPINFO structure.
        &pi )             // Pointer to PROCESS_INFORMATION structure.
    ) 
    {
		// delete temp files
		DeleteFile(compfile.c_str());	
		DeleteFile(asmfile.c_str());	
		DeleteFile(incfile.c_str());	
		SetCurrentDirectory(olddir);
		free(CryptTable);
        MessageBox( 0, "Compile failed", "Uuuups", MB_OK | MB_SETFOREGROUND);
        return;
    }
    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );
    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
	// delete temp files
	DeleteFile(compfile.c_str());	
	DeleteFile(asmfile.c_str());	
	DeleteFile(incfile.c_str());
	// restore old directory
	SetCurrentDirectory(olddir);
	free(CryptTable);
#endif
	// write song info file
	std::string infofile;
	if (objformat != 0) // not windows obj
		infofile = fpath.substr(0, fpath.size()-1) + "h";
	else
		infofile = fpath.substr(0, fpath.size()-3) + "h";
	FILE *fnfofile = fopen(infofile.c_str(), "w");
	fprintf(fnfofile, "// some useful song defines for 4klang\n");
	fprintf(fnfofile, "#define SAMPLE_RATE %d\n", 44100);
	fprintf(fnfofile, "#define BPM %f\n", BeatsPerMinute);
	fprintf(fnfofile, "#define MAX_INSTRUMENTS %d\n", maxinst);
	fprintf(fnfofile, "#define MAX_PATTERNS %d\n", PatternIndices[0].size());
	fprintf(fnfofile, "#define PATTERN_SIZE_SHIFT %d\n", GetShift2(PatternSize));
	fprintf(fnfofile, "#define PATTERN_SIZE (1 << PATTERN_SIZE_SHIFT)\n");
	fprintf(fnfofile, "#define MAX_TICKS (MAX_PATTERNS*PATTERN_SIZE)\n");
	fprintf(fnfofile, "#define SAMPLES_PER_TICK %d\n", ((int)(44100.0f*4.0f*60.0f/(BeatsPerMinute*16.0f))));
	fprintf(fnfofile, "#define MAX_SAMPLES (SAMPLES_PER_TICK*MAX_TICKS)\n");
	fprintf(fnfofile, "#define POLYPHONY %d\n", SynthObj.Polyphony);
	if (output16)
	{
		fprintf(fnfofile, "#define INTEGER_16BIT\n");
		fprintf(fnfofile, "#define SAMPLE_TYPE short\n");
	}
	else
	{
		fprintf(fnfofile, "#define FLOAT_32BIT\n");
		fprintf(fnfofile, "#define SAMPLE_TYPE float\n");
	}
	if (objformat == 0)
	{
		fprintf(fnfofile, "\n#define WINDOWS_OBJECT\n\n");
		fprintf(fnfofile, "// declaration of the external synth render function, you'll always need that\n");
		fprintf(fnfofile, "extern \"C\" void  __stdcall	_4klang_render(void*);\n");
		fprintf(fnfofile, "// declaration of the external envelope buffer. access only if you're song was exported with that option\n");
		fprintf(fnfofile, "extern \"C\" float _4klang_envelope_buffer;\n");
		fprintf(fnfofile, "// declaration of the external note buffer. access only if you're song was exported with that option\n");
		fprintf(fnfofile, "extern \"C\" int   _4klang_note_buffer;\n");
	}
	if (objformat == 1)
	{
		fprintf(fnfofile, "\n#define LINUX_OBJECT\n\n");
		fprintf(fnfofile, "// declaration of the external synth render function, you'll always need that\n");
		fprintf(fnfofile, "extern void* __4klang_render(void*);\n");
		fprintf(fnfofile, "// declaration of the external envelope buffer. access only if you're song was exported with that option\n");
		fprintf(fnfofile, "extern float __4klang_envelope_buffer;\n");
		fprintf(fnfofile, "// declaration of the external note buffer. access only if you're song was exported with that option\n");
		fprintf(fnfofile, "extern int   __4klang_note_buffer;\n");
	}
	if (objformat == 2)
	{
		fprintf(fnfofile, "\n#define MACOSX_OBJECT\n\n");
		fprintf(fnfofile, "// declaration of the external synth render function, you'll always need that\n");
		fprintf(fnfofile, "extern void* _4klang_render(void*);\n");
		fprintf(fnfofile, "// declaration of the external envelope buffer. access only if you're song was exported with that option\n");
		fprintf(fnfofile, "extern float _4klang_envelope_buffer;\n");
		fprintf(fnfofile, "// declaration of the external note buffer. access only if you're song was exported with that option\n");
		fprintf(fnfofile, "extern int   _4klang_note_buffer;\n");
	}
	
	fclose(fnfofile);
}

SynthObjectP Go4kVSTi_GetSynthObject()
{
	return &SynthObj;
}