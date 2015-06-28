#include "ocean.h"
#include "main.h"
#include "debug.h"
#include "fft.h"
#include <stdlib.h>

static const int ocean_N = 128;
static const float ocean_length = 128.f;

// Changing this changes the influence of the wind in the initial state
// The correct value for the formula i 2, but higher values may give nicer results
static const float phillips_kdw_pow = 8.f;

static const float dampening = 0.001f;

static vec2 ocean_wind_speed;
static float ocean_A;
static float ocean_g;

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

	load_shader(ShaderType_Visual, SHADER_OCEAN_DRAW_GLSL, &ocean_draw);
	//load_shader(ShaderType_Compute, SHADER_OCEAN_COMPUTE_GLSL, &ocean_compute);
	//load_shader(ShaderType_Compute, SHADER_FFT_GLSL, &fft_shader);

	glGenBuffers(OceanBuffer_Count, ocean_buffers);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_Ocean]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NxN * sizeof(vec4), NULL, GL_DYNAMIC_DRAW); // TODO: Change to COPY
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ocean_buffers[OceanBuffer_Ocean]);
}

void ocean_seed(vec2* wind, float A, float g) {
	ocean_wind_speed = *wind;
	ocean_A = A;
	ocean_g = g;
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
	float pi_over_length = M_PI / ocean_length;
	vec2 k = {
		(2 * n - ocean_N) * pi_over_length,
		(2 * m - ocean_N) * pi_over_length
	};
	float k_norm = normal_v2(&k);

	float dampening2 = dampening * dampening;

	if (k_norm < dampening2) {
		return 0.f;
	}

	float k_norm2 = k_norm * k_norm;
	float k_norm4 = k_norm2 * k_norm2;

	float w_norm2 = dotv2(&ocean_wind_speed, &ocean_wind_speed);
	float L = w_norm2 / ocean_g;
	float L2 = L*L;

	float w_norm = (float)sqrt(w_norm2);

	float k_dot_w = dotv2(&k, &ocean_wind_speed) / (k_norm * w_norm);
	float k_dot_w_pow = pow(k_dot_w, phillips_kdw_pow);

	float l2 = L2* dampening2;

	return ocean_A * (float)exp(-1.f / (k_norm2 * L2)) / k_norm4 * k_dot_w_pow * (float)exp(-k_norm2 * l2);
}

static float dispersion(int n, int m) {
	float w_0 = 2.f * M_PI / 200.f; // See Tessendorf, Jerry 2001
	float kx = M_PI * (2 * n - ocean_N) / ocean_length;
	float kz = M_PI * (2 * m - ocean_N) / ocean_length;
	return (float)floor(sqrt(ocean_g * sqrt(kx * kx + kz * kz)) / w_0) * w_0;
}

static void hTilde(int n, int m, complex* out) {
	h_tilde0_t* h0 = h_tilde0 + (m * ocean_N + n);

	float omega_t = dispersion(n, m) * time;

	float cosw = (float)cos(omega_t);
	float sinw = (float)sin(omega_t);

	complex c0 = { cosw, sinw };

	complex op1, op2;
	complex_mul(&h0->val, &c0, &op1);

	complex_conj(&c0);
	complex_mul(&h0->mk_conj, &c0, &op2);
	
	complex_add(&op1, &op2, out);

}

void ocean_calculate()
{
	ocean_point_t* points = malloc(sizeof(ocean_point_t)* ocean_N * ocean_N);
	vec4* colors = malloc(sizeof(vec4)* ocean_N * ocean_N);

	vec2 k;
	for (int m = 0; m < ocean_N; ++m) {
		k.y = M_PI * (2.f* m - ocean_N) / ocean_length;
		for (int n = 0; n < ocean_N; ++n) {
			k.x = M_PI * (2.f* n - ocean_N) / ocean_length;
			int index = m * ocean_N + n;
			float k_norm = normal_v2(&k);

			hTilde(n, m, h_tilde + index);

			complex tmp = { 0.f, k.x };
			complex_add(h_tilde + index, &tmp, h_tilde_slopex + index);
			tmp.y = k.y;
			complex_add(h_tilde + index, &tmp, h_tilde_slopez + index);
			if (k_norm < dampening*dampening) {
				memset(h_tilde_dx + index, 0, sizeof(complex));
				memset(h_tilde_dz + index, 0, sizeof(complex));
			} else {
				tmp.y = -k.x / k_norm;
				complex_mul(h_tilde + index, &tmp, h_tilde_dx + index);
				tmp.y = -k.y / k_norm;
				complex_mul(h_tilde + index, &tmp, h_tilde_dz + index);
			}
		}
	}

	// Resolve FFT
	for (int m = 0; m < ocean_N; ++m) {
		fft_execute(&ocean_fft, h_tilde, h_tilde, 1, m * ocean_N);
		fft_execute(&ocean_fft, h_tilde_slopex, h_tilde_slopex, 1, m * ocean_N);
		fft_execute(&ocean_fft, h_tilde_slopez, h_tilde_slopez, 1, m * ocean_N);
		fft_execute(&ocean_fft, h_tilde_dx, h_tilde_dx, 1, m * ocean_N);
		fft_execute(&ocean_fft, h_tilde_dz, h_tilde_dz, 1, m * ocean_N);
	}

	for (int n = 0; n < ocean_N; ++n) {
		fft_execute(&ocean_fft, h_tilde, h_tilde, ocean_N, n);

		fft_execute(&ocean_fft, h_tilde_slopex, h_tilde_slopex, ocean_N, n);
		fft_execute(&ocean_fft, h_tilde_slopez, h_tilde_slopez, ocean_N, n);
		fft_execute(&ocean_fft, h_tilde_dx, h_tilde_dx, ocean_N, n);
		fft_execute(&ocean_fft, h_tilde_dz, h_tilde_dz, ocean_N, n);
	}

	float lambda = -1.f;

	// Resolve
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

			points[index].height = h_tilde[index].x;
			points[index].displacement.x = h_tilde_dx[index].x * lambda;
			points[index].displacement.y = h_tilde_dz[index].x * lambda;

			points[index].normal.x = -h_tilde_slopex[index].x;
			points[index].normal.y = 1.f;
			points[index].normal.z = -h_tilde_slopez[index].x;
			normalize_v3(&points[index].normal);

			memcpy(&colors[index], &points[index].normal, sizeof(vec3));
			colors[index].w = points[index].height;
			colors[index].x = points[index].height;
			colors[index].y = points[index].height;
			colors[index].z = points[index].height;
		}
	}

	//glUseProgram(ocean_compute.program);
	//glDispatchCompute(N / 32, N, 1);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_Ocean]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ocean_N * ocean_N * sizeof(vec4), colors, GL_DYNAMIC_DRAW); // TODO: Change to COPY
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//free(points);
	free(colors);
}
