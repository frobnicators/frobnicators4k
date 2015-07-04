#pragma once

#include "frob_math.h"
#include "shader.h"

typedef struct {
	unsigned int N;
	unsigned int log_2_N;
	unsigned int* reversed;

	shader_t shader;
	GLuint buffers[3];
	GLuint u_w;

	complex **T;
	complex *c[2];
} fft_t;

void fft_init(fft_t* fft, unsigned int N);
void fft_execute(fft_t* fft, complex* input, complex* output, int stride, int offset);
GLuint fft_compute(fft_t* fft, complex* input, int stride, int offset);