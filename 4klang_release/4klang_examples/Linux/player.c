/*
 * file: player.c
 * date: 08-01-2009
 * description: 4klang & OpenGL
 * author: las/mercury
 */

#include <SDL/SDL.h>
#include <GL/gl.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/signal.h>
#include <math.h>

/* include 4klang track header */
#include "4klang.h"

/* synth defines & definitions */

#define SYNTH_CHANNELS 2 /* 4klang is stereo */
#define SYNTH_OUTPUT_SAMPLE_SIZE sizeof(short) /* we use 16 bit audio */
#define SYNTH_PLAY_BUFFER 4096

static SAMPLE_TYPE synthBuffer[MAX_SAMPLES * SYNTH_CHANNELS];

typedef void* (*threadfunc_t)(void*);

/* */
static volatile int samplePosition = 0;


/* returns the number of available processors */
static int getCores() {
	int cores = 1;
	if ((cores = sysconf(_SC_NPROCESSORS_ONLN)) < 1) {
		cores = 1;
	}
	return cores;
}

#ifdef INTEGER_16BIT
/* callback function for the music output - 16b audio - stereo */
static void synthCallback(void *udata, Uint8 *stream, int len) {
	/* the music is stereo - use 32 bit for left and right channel */
	static int32_t* sample = (int32_t*)synthBuffer;
	int32_t* ptr = (int32_t*) stream;

	udata += 0;
	int i;
	for (i = len / (SYNTH_OUTPUT_SAMPLE_SIZE * SYNTH_CHANNELS); i > 0 ; i--) {
		/* loop the track */
		if (samplePosition >= MAX_SAMPLES) {
			samplePosition = 0;
			sample = udata;
		}
		/* copy left and right channel at the same time */
		*ptr++ = *sample++;
		samplePosition++;
	}
}
#else
/* callback function for the music output - converts 32b float to 16b audio */
static void synthCallback(void *udata, Uint8 *stream, int len) {
	static float* sample = synthBuffer;
	int16_t* ptr = (int16_t*) stream;

	udata += 0;
	int i;
	for (i = len / (SYNTH_OUTPUT_SAMPLE_SIZE * SYNTH_CHANNELS); i > 0 ; i--) {
		/* loop the track */
		if (samplePosition >= MAX_SAMPLES) {
			samplePosition = 0;
			sample = udata;
		}
		*ptr++ = (int16_t)(INT16_MAX * *sample++);
		*ptr++ = (int16_t)(INT16_MAX * *sample++);
		samplePosition++;
	}
}
#endif

/* defines for the particle effect */
#define noise() ((float)rand()/(float)RAND_MAX)
#define scale(X) ((X - 0.5f) * 2.0f)
#define fade(X) (1.0f - fabs(scale(X)))

int main(int argc, char** argv) {
	/* -Wall -Werror -Wmercury */
	argc += 0; argv += 0;

	/* initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		fprintf(stderr, "SDL_Init() failed\n");
		return EXIT_FAILURE;
	}

	SDL_SetVideoMode(640, 480, 32, SDL_OPENGL);

	/* audio specification */
	SDL_AudioSpec audioSpec;
	audioSpec.freq = SAMPLE_RATE;
	audioSpec.format = AUDIO_S16SYS;
	audioSpec.channels = SYNTH_CHANNELS;
	audioSpec.silence = 0;
	audioSpec.samples = SYNTH_PLAY_BUFFER;
	audioSpec.size = 0;
	audioSpec.callback = synthCallback;
	audioSpec.userdata = synthBuffer;

	/* setup audio */
	if (SDL_OpenAudio(&audioSpec, NULL)) {
		fprintf(stderr, "SDL_OpenAudio() failed\n");
		SDL_Quit();
		return EXIT_FAILURE;
	}

	static pthread_t synthRenderThread;
	int isMultiCore = getCores() > 1;
	if (isMultiCore) {
		/* use a thread to render the music */
		if (pthread_create(&synthRenderThread, NULL, (threadfunc_t)__4klang_render, synthBuffer)) {
			fprintf(stderr, "pthread_create() failed\n");
			SDL_Quit();
			return EXIT_FAILURE;
		}
		fprintf(stderr, "Using sound thread\n");
	} else {
		fprintf(stderr, "Using sound precalculation\n");
		/* no multicore - precalculate the music */
		__4klang_render(synthBuffer);
	}

	/* start the music */
	SDL_PauseAudio(0);

	/* OpenGL init */
	glEnable(GL_POINT_SMOOTH);
	glPointSize(8.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	/* minimalistic mainloop - will exit on keydown event*/
	SDL_Event event;
	while (1) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_KEYDOWN) goto _exit;
		}
		glLoadIdentity();
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		/* particle effect */
		glLoadIdentity();
		srand(0);
		glBegin(GL_POINTS);
		float v[2];
		int i;
		for (i = 1023; i >= 0; i--) {
			float f = SDL_GetTicks() / 10000.0f;
			float s = f * 5.0f + noise();
			float t = noise();

			f += noise();
			f -= (int)f;
			s -= (int)s;

			v[0] = scale(t);
			v[1] = scale(f);

			/* access the envelope buffer */
			float bassEnv = (&__4klang_envelope_buffer)[((samplePosition >> 8) << 5) + 2 * 2 + 0]; /* bass is instrument 2 */
			float snareEnv = (&__4klang_envelope_buffer)[((samplePosition >> 8) << 5) + 2 * 6 + 0]; /* snare is instrument 6 */
			
			/* access the note buffer */
			int padNote = (&__4klang_note_buffer)[((samplePosition >> 8) << 5) + 2 * 0 + 0]; /* melody string is instrument 0 */
			v[0] += -0.5f + (float)padNote / 128.0f;
			
			glColor4f(bassEnv, snareEnv, 0.5f, fade(f) * fade(s) * fade(t));
			glVertex2fv(v);
		}
		glEnd();

		SDL_GL_SwapBuffers();
	}

	/* kill the music render thread in case it's still running */
	if (isMultiCore) {
		pthread_kill(synthRenderThread, SIGKILL);
	}

_exit:
	SDL_Quit();
	return EXIT_SUCCESS;
}
