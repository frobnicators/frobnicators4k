#include "fft.h"
#include <stdlib.h>

static unsigned int bit_revers(fft_t* fft, unsigned int i);
static void fft_twiddle(unsigned N, unsigned int x, complex* out);

void fft_init(fft_t* fft, unsigned int N) {
	fft->N = N;
	fft->log_2_N = (unsigned int)(log(N) / log(2.f));
	fft->reversed = malloc(N*sizeof(unsigned int));

	for (unsigned i = 0; i < N; ++i) {
		fft->reversed[i] = bit_revers(fft, i);
	}

	unsigned int pow2 = 1;
	fft->T = malloc(sizeof(complex)*fft->log_2_N);
	for (unsigned int i = 0; i < fft->log_2_N; ++i) {
		fft->T[i] = malloc(sizeof(complex) * pow2);
		for (unsigned int j = 0; j < pow2; ++j) {
			fft_twiddle(fft->N, j, fft->T[i] + j);
		}
		pow2 *= 2;
	}

	fft->c[0] = malloc(sizeof(complex)*N);
	fft->c[1] = malloc(sizeof(complex)*N);
}

// Perform FFT Butterfly combine
void fft_execute(fft_t* fft, complex* input, complex* output, int stride, int offset) {
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
				complex_add(c[which ^ 1] + (size * j - half_size + k), &temp, c[which] + (size * j + k));
			}
		}
		loops >>= 1;
		size <<= 1;
		half_size <<= 1;
		++w_;
	}

	// Copy c to output
	for (unsigned int i = 0; i < fft->N; ++i) {
		output[i * stride + offset] = c[which][i];
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

static void fft_twiddle(unsigned int N, unsigned int x, complex* out) {
	float v = M_PI * x / N;
	out->x = (float)cos(v);
	out->y = (float)sin(v);
}
