#include "ocean.h"
#include "main.h"
#include "debug.h"
#include <stdlib.h>

static const int ocean_N = 512;
static const float ocean_length = 512.f;

static const float dampening = 0.001f;

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
static ocean_point_t calculate_ocean_point(vec2 x);

void ocean_init() {
	const int NxN = ocean_N * ocean_N;
	h_tilde = malloc(NxN * sizeof(h_tilde_t));
	h_tilde0 = malloc(NxN * sizeof(h_tilde0_t));

	vec4* colors = malloc(NxN * sizeof(vec4));
	for (int m = 0; m < ocean_N; ++m) {
		for (int n = 0; n < ocean_N; ++n) {
			int index = m * ocean_N + n;
			colors[index].x = 1.f;
			colors[index].y = 0.f;
			colors[index].z = 0.f;
			colors[index].w = 1.f;
		}

	}

	ocean_calculate_initial_state();

	load_shader(ShaderType_Visual, SHADER_OCEAN_DRAW_GLSL, &ocean_draw);
	//load_shader(ShaderType_Compute, SHADER_OCEAN_COMPUTE_GLSL, &ocean_compute);
	//load_shader(ShaderType_Compute, SHADER_FFT_GLSL, &fft_shader);

	glGenBuffers(OceanBuffer_Count, ocean_buffers);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_Ocean]);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, NxN * sizeof(ocean_point_t), NULL, GL_DYNAMIC_DRAW); // TODO: Change to COPY
	glBufferData(GL_SHADER_STORAGE_BUFFER, NxN * sizeof(vec4), colors, GL_STATIC_DRAW); // TODO: Change to COPY
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

static void hTilde0(int n, int m, complex* out) {
	// This should preferably be a Gaussian distribution
	complex r = { (float)rand() / RAND_MAX, (float)rand() / RAND_MAX };

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
static ocean_point_t calculate_ocean_point(vec2 x) {
	ocean_point_t point = { 
		.height = { 0.f },
		.displacement = { 0.f, 0.f },
		.normal = { 0.f, 0.f, 0.f}
	};

	complex h, c, htilde_c;
	vec2 k;
	float k_norm;
	float k_dot_x;
	float m2pi_over_length = 2.f * M_PI / ocean_length;
	for (int m = 0; m < ocean_N; ++m) {
		k.y = (m - ocean_N / 2.f) * m2pi_over_length;
		for (int n = 0; n < ocean_N; ++n) {
			k.x = (n - ocean_N / 2.f) * m2pi_over_length;
			k_norm = normal_v2(&k);
			k_dot_x = dotv2(&k, &x);

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
	for (int m = 0; m < ocean_N; ++m) {
		for (int n = 0; n < ocean_N; ++n) {
			vec2 x = { (float)n, (float)m };
			ocean_point_t point = calculate_ocean_point(x);
			int index = m * ocean_N + m;
			colors[index].x = colors[index].y = colors[index].z = 0.f;// point.height*10.f;
			colors[index].w = 1.f;
			FROB_PRINTF("%d,%d => %f, (%f, %f) -> (%f, %f, %f)\n", n, m, point.height, point.displacement.x, point.displacement.y, point.normal.x, point.normal.y, point.normal.z);
		}
	}
	//glUseProgram(ocean_compute.program);
	//glDispatchCompute(N / 32, N, 1);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_Ocean]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ocean_N * ocean_N * sizeof(vec4), colors, GL_DYNAMIC_DRAW); // TODO: Change to COPY
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
