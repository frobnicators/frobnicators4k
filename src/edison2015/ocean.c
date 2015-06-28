#include "ocean.h"
#include "main.h"
#include "debug.h"
#include "fft.h"
#include <stdlib.h>

static const int ocean_N = 64;
static const float ocean_length = 64.f;

static const float dampening = 0.001f;

vec2 ocean_wind_speed = { 32.f, 32.f };
float ocean_A = 0.0005f;
float ocean_g = 9.81f;

#define USE_FFT 1

typedef struct {
	complex h;
	complex slope_x;
	complex slope_z;
	complex dx;
	complex dz;
} h_tilde_t;

//static h_tilde_t* h_tilde;
static complex* h_tilde;
static complex* h_tilde_slopex;
static complex* h_tilde_slopez;
static complex* h_tilde_dx;
static complex* h_tilde_dz;

typedef struct {
	complex val;
	complex mk_conj;
} h_tilde0_t;

typedef struct {
	vec3 normal;
	float height;
	vec2 displacement;
} ocean_point_t;

static h_tilde0_t* h_tilde0;
static fft_t ocean_fft;

struct shader_t ocean_draw;
static struct shader_t fft_shader;
static struct shader_t ocean_compute;

typedef enum {
	OceanBuffer_Ocean = 0,
	OceanBuffer_Count,
} OceanBuffer;

static GLuint ocean_buffers[OceanBuffer_Count];


__forceinline void ocean_calculate_initial_state();

static void hTilde0(int n, int m, complex* out);
static float phillips(int n, int m);
static float dispersion(int n, int m);
static ocean_point_t calculate_ocean_point(vec2* x);

void ocean_init() {
	const int NxN = ocean_N * ocean_N;
	h_tilde0 = malloc(NxN * sizeof(h_tilde0_t));

	//h_tilde = malloc(NxN * sizeof(h_tilde_t));
	h_tilde = malloc(NxN * sizeof(complex));
	h_tilde_slopex = malloc(NxN * sizeof(complex));
	h_tilde_slopez = malloc(NxN * sizeof(complex));
	h_tilde_dx = malloc(NxN * sizeof(complex));
	h_tilde_dz = malloc(NxN * sizeof(complex));

	fft_init(&ocean_fft, ocean_N);
	ocean_calculate_initial_state();

	load_shader(ShaderType_Visual, SHADER_OCEAN_DRAW_GLSL, &ocean_draw);
	//load_shader(ShaderType_Compute, SHADER_OCEAN_COMPUTE_GLSL, &ocean_compute);
	//load_shader(ShaderType_Compute, SHADER_FFT_GLSL, &fft_shader);

	glGenBuffers(OceanBuffer_Count, ocean_buffers);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_Ocean]);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, NxN * sizeof(ocean_point_t), NULL, GL_DYNAMIC_DRAW); // TODO: Change to COPY
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ocean_buffers[OceanBuffer_Ocean]);
}

void ocean_calculate_initial_state() {
	for (int m = 0; m < ocean_N; ++m) {
		for (int n = 0; n < ocean_N; ++n) {
			h_tilde0_t* ht = h_tilde0 + (m * ocean_N + n);
			hTilde0(n, m, &(ht->val));
			hTilde0(-n, -m, &(ht->mk_conj));
			complex_conj(&(ht->mk_conj));
		}
	}
}


__forceinline float uniformRandomVariable() {
	return (float)rand() / RAND_MAX;
}

static complex gaussianRandomVariable() {
	float x1, x2, w;
	do {
		x1 = 2.f * uniformRandomVariable() - 1.f;
		x2 = 2.f * uniformRandomVariable() - 1.f;
		w = x1 * x1 + x2 * x2;
	} while (w >= 1.f);
	w = sqrt((-2.f * log(w)) / w);
	complex c = { x1 * w, x2 * w };
	return c;
}

static void hTilde0(int n, int m, complex* out) {
	complex r = gaussianRandomVariable();

	float ph = (float)sqrt(phillips(n, m) / 2.f);
	out->x = r.x * ph;
	out->y = r.y * ph;
}

static float phillips(int n, int m) {
	float pi_over_size = M_PI / ocean_length;
	vec2 k = {
		(2 * n - ocean_N) * pi_over_size,
		(2 * m - ocean_N) * pi_over_size
	};
	float k_norm = normal_v2(&k);

	if (k_norm < dampening) {
		return 0.f;
	}

	float k_norm2 = k_norm * k_norm;

	// Normalize k
	k.x /= k_norm;
	k.y /= k_norm;

	float w_norm2 = dotv2(&ocean_wind_speed, &ocean_wind_speed);
	float L2 = (float)pow(w_norm2 / ocean_g, 2.f);

	float w_norm = (float)sqrt(w_norm2);
	vec2 w_unit = { ocean_wind_speed.x / w_norm, ocean_wind_speed.y / w_norm };

	float k_dot_w = dotv2(&k, &w_unit);
	float k_dot_w2 = k_dot_w * k_dot_w;

	float l2 = L2* dampening * dampening;

	return ocean_A * ((float)exp(-1.f / (k_norm2 * L2)) / (k_norm2 * k_norm2)) * k_dot_w2 * (float)exp(-k_norm2 * l2);
}

static float dispersion(int n, int m) {
	float w_0 = 2.f * M_PI / 200.f; // See Tessendorf, Jerry 2001
	float kx = M_PI * (2 * n - ocean_N) / ocean_length;
	float kz = M_PI * (2 * m - ocean_N) / ocean_length;
	return (float)floor(sqrt(ocean_g * sqrt(kx * kx + kz * kz)) / w_0) * w_0;
}

static void hTilde(int n, int m, complex* out) {
	h_tilde0_t* h0 = h_tilde0 + (m * ocean_N + m);

	float omega_t = dispersion(n, m) * time;

	float cosw = (float)cos(omega_t);
	float sinw = (float)sin(omega_t);

	complex c0 = { cosw, sinw };

	complex op1, op2;
	complex_mul(&h_tilde0->val, &c0, &op1);

	complex_conj(&c0);
	complex_mul(&h_tilde0->mk_conj, &c0, &op2);
	
	complex_add(&op1, &op2, out);

}

// This is a slow DFT calculation, will be replaced with FFT later
// This is just to verify that it works at all
static ocean_point_t calculate_ocean_point(vec2* x) {
	ocean_point_t point = { 
		.height = { 0.f },
		.displacement = { 0.f, 0.f },
		.normal = { 0.f, 0.f, 0.f}
	};

	complex h = { 0.f, 0.f };
	complex c, htilde_c;
	vec2 k;
	float k_norm;
	float k_dot_x;
	float m2pi_over_length = 2.f * M_PI / ocean_length;
	for (int m = 0; m < ocean_N; ++m) {
		k.y = (m - ocean_N / 2.f) * m2pi_over_length;
		for (int n = 0; n < ocean_N; ++n) {
			k.x = (n - ocean_N / 2.f) * m2pi_over_length;
			k_norm = normal_v2(&k);
			k_dot_x = dotv2(&k, x);

			c.x = (float)cos(k_dot_x);
			c.y = (float)sin(k_dot_x);
			complex htilde;
			hTilde(n, m, &htilde);
			complex_mul(&htilde, &c, &htilde_c);

			complex_add(&h, &htilde_c, &h);

			// TODO: Should we really only use the imaginary part of htilde_c here?
			point.normal.x -= k.x * htilde_c.y;
			point.normal.z -= k.y * htilde_c.y;

			if (k_norm < dampening) {
				continue;
			}

			point.displacement.x += (k.x / k_norm) * htilde_c.y;
			point.displacement.y += (k.y / k_norm) * htilde_c.y;
		}
	}

	point.normal.x *= -1.f;
	point.normal.y = 1.f;
	point.normal.z *= -1.f;
	normal_v3(&point.normal);
	point.height = h.x;
	return point;
}

void ocean_calculate()
{
	vec4* colors = malloc(sizeof(vec4)* ocean_N * ocean_N);

#if USE_FFT
	vec2 k;
	for (int m = 0; m < ocean_N; ++m) {
		k.y = M_PI * (2.f* m - ocean_N) / ocean_length;
		for (int n = 0; n < ocean_N; ++n) {
			k.x = M_PI * (2.f* n - ocean_N) / ocean_length;
			int index = m * ocean_N + n;
			float k_norm = normal_v2(&k);

			hTilde(n, m, h_tilde + index);

			/*if (n < 16 && m < 16)
				FROB_PRINTF("%d,%d => (%f +%fi)\n", n, m, h_tilde[index].x, h_tilde[index].y);
				*/

			complex tmp = { 0.f, k.x };
			complex_add(h_tilde + index, &tmp, h_tilde_slopex + index);
			tmp.y = k.y;
			complex_add(h_tilde + index, &tmp, h_tilde_slopez + index);
			if (k_norm < dampening*dampening) {
				memset(h_tilde_dx + index, 0, sizeof(complex));
				memset(h_tilde_dz + index, 0, sizeof(complex));
			} else {
				tmp.x = -k.x / k_norm;
				complex_mul(h_tilde + index, &tmp, h_tilde_dx + index);
				tmp.x = -k.y / k_norm;
				complex_mul(h_tilde + index, &tmp, h_tilde_dz + index);
			}
		}
	}

	// Resolve FFT
	for (int m = 0; m < ocean_N; ++m) {
		fft_execute(&ocean_fft, h_tilde, h_tilde, 1, m * ocean_N);
	}

	for (int n = 0; n < ocean_N; ++n) {
		fft_execute(&ocean_fft, h_tilde, h_tilde, ocean_N, n);
	}

	// Resolve signs
	float signs[2] = { 1.f, -1.f };
	for (int m = 0; m < ocean_N; ++m) {
		for (int n = 0; n < ocean_N; ++n) {
			int index = m * ocean_N + n;

			float sgn = signs[(n + m) & 1];

			h_tilde[index].x *= sgn;
			h_tilde[index].y *= sgn;
			h_tilde_slopex[index].x *= sgn;
			h_tilde_slopez[index].y *= sgn;
			h_tilde_dx[index].x *= sgn;
			h_tilde_dz[index].y *= sgn;

			colors[index].x = colors[index].y = colors[index].z = h_tilde[index].x;
			colors[index].w = 1.f;

			/*if (n < 16 && m < 16)
				FROB_PRINTF("%d,%d => %f\n", n, m, h_tilde[index].x);
				*/
				//, (%f, %f) -> (%f, %f, %f)\n", n, m, point.height, point.displacement.x, point.displacement.y, point.normal.x, point.normal.y, point.normal.z);
		}
	}

#else

	for (int m = 0; m < ocean_N; ++m) {
		for (int n = 0; n < ocean_N; ++n) {
			int index = m * ocean_N + n;

			vec2 x = { (float)n, (float)m };
			ocean_point_t point = calculate_ocean_point(&x);

			colors[index].x = colors[index].y = colors[index].z = point.height;
			colors[index].w = 1.f;
		}
	}
#endif

	//glUseProgram(ocean_compute.program);
	//glDispatchCompute(N / 32, N, 1);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_Ocean]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ocean_N * ocean_N * sizeof(vec4), colors, GL_DYNAMIC_DRAW); // TODO: Change to COPY
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	free(colors);
}
