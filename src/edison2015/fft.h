#pragma once

#include "frob_math.h"
#include "shader.h"

typedef struct {
	unsigned int N;
	unsigned int log_2_N;
	unsigned int* reversed;

	shader_t shader;
	GLuint rev_buffer;
	GLuint u_which;
	GLuint u_wi; // w & invert
	GLuint u_os; // offset & stride

	complex **T;
	complex *c[2];

	GLuint* twiddle_buffers;
} fft_t;

void fft_init(fft_t* fft, unsigned int N);

GLuint fft_compute(fft_t* fft, GLuint input, GLuint swap_buffer);

// CPU butterfly combine
void fft_execute(fft_t* fft, complex* input, int stride, int offset);