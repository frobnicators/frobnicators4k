#include "fft.h"
#include "debug.h"
#include "shader.h"
#include "util.h"
#include "perf.h"
#include <stdlib.h>

static unsigned int bit_revers(fft_t* fft, unsigned int i);
static void fft_twiddle(unsigned int x, unsigned int N, complex* out);

static shader_stage_t fft_compute_stage = { GL_COMPUTE_SHADER, 1, { SHADER_FFT_GLSL } };


#define TWIDDLE_BINDING 0
#define BIT_REVERSE_BINDING 1
#define INPUT_BINDING 2
#define OUTPUT_BINDING 3

void fft_init(fft_t* fft, unsigned int N) {
	fft->N = N;
	fft->log_2_N = (unsigned int)(log(N) / log(2.f));
	fft->reversed = malloc(N*sizeof(unsigned int));

	for (unsigned i = 0; i < N; ++i) {
		fft->reversed[i] = bit_revers(fft, i);
	}

	unsigned int pow2 = 1;
	fft->T = malloc(sizeof(complex)*fft->log_2_N);

	fft->twiddle_buffers = malloc(sizeof(GLuint)*fft->log_2_N);
	glGenBuffers(fft->log_2_N, fft->twiddle_buffers);

	for (unsigned int i = 0; i < fft->log_2_N; ++i) {

		fft->T[i] = malloc(sizeof(complex) * pow2);
		for (unsigned int j = 0; j < pow2; ++j) {
			fft_twiddle(j, pow2 * 2, fft->T[i] + j);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fft->twiddle_buffers[i]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(complex)*pow2, fft->T[i], GL_STATIC_DRAW);

		pow2 *= 2;
	}

	fft->c[0] = malloc(sizeof(complex)*N);
	fft->c[1] = malloc(sizeof(complex)*N);

	load_shader(&fft->shader, 1, &fft_compute_stage);

	fft->u_wi = glGetUniformLocation(fft->shader.program, "wi");
	fft->u_os = glGetUniformLocation(fft->shader.program, "os");

	glGenBuffers(1, &fft->rev_buffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, fft->rev_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int)*N, fft->reversed, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	/*
	for (int i = 0; i < 2; ++i) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, fft->buffers[i]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(complex)*N*N, NULL, GL_DYNAMIC_DRAW);
	}
	*/

	CHECK_FOR_GL_ERRORS("init fft");
}

static GLuint fft_compute_dispatch(fft_t* fft, GLuint input, GLuint swap_buffer, int stride, int offset_mul) {
	glUniform2i(fft->u_os, offset_mul, stride);
	GLuint buffers[] = { input, swap_buffer };

	int which = 0;
	int invert = 1;
	for (unsigned int i = 0; i < fft->log_2_N; ++i) {
		which ^= 1;
		glUniform2i(fft->u_wi, i, invert);
		if (invert == 1)
			invert = 0;

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TWIDDLE_BINDING, fft->twiddle_buffers[i]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, INPUT_BINDING, buffers[which^1]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, OUTPUT_BINDING, buffers[which]);

		CHECK_FOR_GL_ERRORS("fft bindings");

		glDispatchCompute(fft->N / 2, fft->N, 1);
		CHECK_FOR_GL_ERRORS("fft dispatch");
	}

	/*
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[which]);
	complex* out = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(complex)*fft->N*fft->N, GL_MAP_READ_BIT);

	CHECK_FOR_GL_ERRORS("map");

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	*/

	return buffers[which];
}

GLuint fft_compute(fft_t* fft, GLuint input, GLuint swap_buffer) {
	glUseProgram(fft->shader.program);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BIT_REVERSE_BINDING, fft->rev_buffer);

	GLuint b = fft_compute_dispatch(fft, input, swap_buffer, 1, fft->N);
	GLuint sw = b == input ? swap_buffer : input;
	return fft_compute_dispatch(fft, b, sw, fft->N, 1);

	/*
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[which]);
	complex* out = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(complex)*fft->N, GL_MAP_READ_BIT);

	CHECK_FOR_GL_ERRORS("map");

	// Copy c to output
	for (unsigned int i = 0; i < fft->N; ++i) {
		input[i * stride + offset] = out[i];
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	FROB_PERF_END(map);

	CHECK_FOR_GL_ERRORS("unmap");

	return fft->buffers[which];
	*/
}

// Perform FFT Butterfly combine
void fft_execute(fft_t* fft, complex* input, int stride, int offset) {
	int which = 0;
	complex** c = fft->c;
	// Copy input to c
	for (unsigned int i = 0; i < fft->N; ++i) {
		c[which][i] = input[fft->reversed[i] * stride + offset];
	}

	unsigned int loops = fft->N >> 1; // N / 2
	int size = 2;
	int half_size = 1;
	int w_ = 0;
	complex temp;
	for (unsigned int i = 0; i < fft->log_2_N; ++i) {
		which ^= 1;
		for (unsigned int j = 0; j < loops; ++j) {
			for (int k = 0; k < half_size; ++k) {
				complex_mul(c[which ^ 1] + (size*j + half_size + k), fft->T[w_] + k, &temp);
				complex_add(c[which ^ 1] + (size * j + k), &temp, c[which] + (size * j + k));
			}

			for (int k = half_size; k < size; ++k) {
				complex_mul(c[which ^ 1] + (size*j + k), fft->T[w_] + (k - half_size), &temp);
				complex_sub(c[which ^ 1] + (size * j - half_size + k), &temp, c[which] + (size * j + k));
			}
		}
		loops >>= 1;
		size <<= 1;
		half_size <<= 1;
		++w_;
	}

	// Copy c to output
	for (unsigned int i = 0; i < fft->N; ++i) {
		input[i * stride + offset] = c[which][i];
	}
}

static unsigned int bit_revers(fft_t* fft, unsigned int i) {
	unsigned int res = 0;
	for (unsigned int j = 0; j < fft->log_2_N; ++j) {
		res = (res << 1) + (i & 1);
		i >>= 1;
	}
	return res;
}

static void fft_twiddle(unsigned int x, unsigned int N, complex* out) {
	float v = 2.f * M_PI * x / N;
	out->x = (float)cos(v);
	out->y = (float)sin(v);
}
