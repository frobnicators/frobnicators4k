#include "ocean.h"
#include "main.h"
#include <stdlib.h>

static const int ocean_N = 512;
static const float ocean_length = 512.f;

vec2 ocean_wind_speed = { 10.f, 1.f };
float ocean_A = 0.0005f;
float ocean_g = 9.81f;

typedef struct {
	complex h;
	complex slope_x;
	complex slope_z;
	complex dx;
	complex dz;
} h_tilde_t;

static h_tilde_t* h_tilde;

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

void ocean_init() {
	const int NxN = ocean_N * ocean_N;
	h_tilde = malloc(NxN * sizeof(h_tilde_t));
	h_tilde0 = malloc(NxN * sizeof(h_tilde0_t));

	ocean_calculate_initial_state();

	load_shader(ShaderType_Visual, SHADER_OCEAN_DRAW_GLSL, &ocean_draw);
	//load_shader(ShaderType_Compute, SHADER_OCEAN_COMPUTE_GLSL, &ocean_compute);
	//load_shader(ShaderType_Compute, SHADER_FFT_GLSL, &fft_shader);

	glGenBuffers(OceanBuffer_Count, ocean_buffers);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_Ocean]);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, NxN * sizeof(ocean_point_t), NULL, GL_DYNAMIC_DRAW); // TODO: Change to COPY
	glBufferData(GL_SHADER_STORAGE_BUFFER, NxN * sizeof(vec4), NULL, GL_DYNAMIC_DRAW); // TODO: Change to COPY
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

static void hTilde0(int n, int m, complex* out) {
	// This should preferably be a Gaussian distribution
	complex r = { (float)rand() / RAND_MAX, (float)rand() / RAND_MAX };

	float ph = (float)sqrt(phillips(n, m) / 2.f);
	out->x = r.x * ph;
	out->y = r.y * ph;
}

static float phillips(int n, int m) {
	const float dampening = 0.001f;
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

void ocean_calculate()
{
	//glUseProgram(ocean_compute.program);
	//glDispatchCompute(N / 32, N, 1);
}
