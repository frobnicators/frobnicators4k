#include "ocean.h"
#include "main.h"
#include "debug.h"
#include "fft.h"
#include "perf.h"
#include "util.h"
#include "fbo.h"

#include <stdlib.h>

#define TEXTURE_SIZE 4096

static const int ocean_N = 32;
static const float ocean_length = 32.f;

// Changing this changes the influence of the wind in the initial state
// The correct value for the formula i 2, but higher values may give nicer results
static const float phillips_kdw_pow = 8.f;

static const float dampening = 0.001f;

static vec2 ocean_wind_speed;
static float ocean_A;
static float ocean_g;

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

shader_t ocean_draw;
static shader_t ocean_rasterize;
static shader_t fft_shader;
static shader_t ocean_compute;

static shader_stage_t ocean_draw_frag = { GL_FRAGMENT_SHADER, 2, { SHADER_COMMON_GLSL, SHADER_OCEAN_DRAW_GLSL } };
static shader_stage_t ocean_rasterize_vert = { GL_VERTEX_SHADER, 1, { SHADER_OCEAN_RASTERIZE_VERT_GLSL } };
static shader_stage_t ocean_rasterize_frag = { GL_FRAGMENT_SHADER, 1, { SHADER_OCEAN_RASTERIZE_FRAG_GLSL } };

static GLuint ocean_rasterize_vao;
static mat4 ocean_rasterize_projection;
static fbo_t ocean_rasterize_fbo;

typedef enum {
	OceanBuffer_VertexData,
	OceanBuffer_Indices,
	OceanBuffer_OceanData,
	OceanBuffer_Displacement,
	OceanBuffer_Count,
} OceanBuffer;

typedef struct {
	int x;
	int y;
} ivec2;

typedef struct {
	vec3 pos;
	int index;
} ocean_vertex_data_t;

static GLuint ocean_buffers[OceanBuffer_Count];
static GLuint u_ocean_proj;
static unsigned ocean_num_indices = 0;

__forceinline void ocean_calculate_initial_state();

static void hTilde0(int n, int m, complex* out);
static float phillips(int n, int m);
static float dispersion(int n, int m);

void ocean_init() {
	const int NxN = ocean_N * ocean_N;
	h_tilde0 = malloc(NxN * sizeof(h_tilde0_t));

	h_tilde = malloc(NxN * sizeof(complex));
	h_tilde_slopex = malloc(NxN * sizeof(complex));
	h_tilde_slopez = malloc(NxN * sizeof(complex));
	h_tilde_dx = malloc(NxN * sizeof(complex));
	h_tilde_dz = malloc(NxN * sizeof(complex));

	fft_init(&ocean_fft, ocean_N);

	load_shader(&ocean_draw, 1, 2, &default_vertex_stage, &ocean_draw_frag);
	load_shader(&ocean_rasterize, 0, 2, &ocean_rasterize_vert, &ocean_rasterize_frag);

	// Generate index and vertex buffer
	int Nplus1 = ocean_N + 1;

	ocean_vertex_data_t* vertices = malloc(Nplus1*Nplus1 * sizeof(ocean_vertex_data_t));
	unsigned int* indices = malloc(Nplus1*Nplus1 * sizeof(unsigned int)*2);

	for (int m = 0; m < Nplus1; ++m) {
		for (int n = 0; n < Nplus1; ++n) {
			int index = m * Nplus1 + n;

			vertices[index].pos.x = (n - ocean_N / 2.f)*ocean_length/ocean_N;
			vertices[index].pos.y = (m - ocean_N / 2.f)*ocean_length/ocean_N;
			vertices[index].pos.z = 0.f;

			int ni = 0;
			int mi = 0;
			if (n < ocean_N) {
				ni = n;
			}

			if (m < ocean_N) {
				mi = m;
			}

			vertices[index].index = mi * ocean_N + ni;

			if (m < ocean_N) {
				if (m % 2 == 0) {
					indices[ocean_num_indices++] = index;
					indices[ocean_num_indices++] = index + Nplus1;
				} else if (n < ocean_N) {
					int reverse_index = (m + 1) * Nplus1 - (n + 1);
					indices[ocean_num_indices++] = reverse_index + Nplus1;
					indices[ocean_num_indices++] = reverse_index - 1;
				}
			}
		}
	}
	indices[ocean_num_indices++] = Nplus1 * ocean_N;

	memset(&ocean_rasterize_projection, 0, sizeof(mat4));
	const float near_far = 30.f; // near and far distance
	float two_over_width = 2.f / (float)ocean_length;
	float half_width = (float)ocean_length / 2.f;

	ocean_rasterize_projection.c0.x = two_over_width;
	ocean_rasterize_projection.c1.y = two_over_width;
	ocean_rasterize_projection.c2.z = -1.f / near_far; // -2 / (2 * near_far)

	ocean_rasterize_projection.c3.w = 1.f;

	glGenVertexArrays(1, &ocean_rasterize_vao);
	glBindVertexArray(ocean_rasterize_vao);

	glUseProgram(ocean_rasterize.program);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glEnable(GL_DEPTH_TEST);
	u_ocean_proj = glGetUniformLocation(ocean_rasterize.program, "M");
	glUniformMatrix4fv(u_ocean_proj,1, GL_FALSE, (float*)&ocean_rasterize_projection);

	glGenBuffers(OceanBuffer_Count, ocean_buffers);

	glBindBuffer(GL_ARRAY_BUFFER,  ocean_buffers[OceanBuffer_VertexData]);
	glBufferData(GL_ARRAY_BUFFER, Nplus1*Nplus1 * sizeof(ocean_vertex_data_t), vertices, GL_STATIC_DRAW); // TODO: Change to COPY
	glVertexAttribPointer(0, 3, GL_FLOAT, FALSE, sizeof(ocean_vertex_data_t), 0);
	glVertexAttribIPointer(1, 1, GL_INT, sizeof(ocean_vertex_data_t), (GLvoid*)offsetof(ocean_vertex_data_t, index));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_OceanData]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ocean_N*ocean_N * sizeof(vec4), NULL, GL_DYNAMIC_DRAW); // TODO: Change to COPY when written from GPU
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ocean_buffers[OceanBuffer_OceanData]);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_Displacement]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ocean_N*ocean_N * sizeof(vec2), NULL, GL_DYNAMIC_DRAW); // TODO: Change to COPY when written from GPU
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ocean_buffers[OceanBuffer_Displacement]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  ocean_buffers[OceanBuffer_Indices]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ocean_num_indices*sizeof(unsigned int), indices, GL_STATIC_DRAW);

	create_fbo(TEXTURE_SIZE, TEXTURE_SIZE, GL_RGBA32F, GL_RGBA, GL_FLOAT, 0, &ocean_rasterize_fbo);

	glBindTexture(GL_TEXTURE_2D, ocean_rasterize_fbo.textures[TextureType_Color0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, ocean_rasterize_fbo.textures[TextureType_Color1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindVertexArray(0);

	CHECK_FOR_GL_ERRORS("ocean texture");
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
	w = sqrtf((-2.f * logf(w)) / w);
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
	float k_dot_w_pow = (float)pow(k_dot_w, phillips_kdw_pow);

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
	FROB_PERF_BEGIN(ocean_calculate);
	//ocean_point_t* points = malloc(sizeof(ocean_point_t)* ocean_N * ocean_N);

	FROB_PERF_BEGIN(ocean_hTilde);
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
	FROB_PERF_END(ocean_hTilde);

	FROB_PERF_BEGIN(ocean_fft);
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
	FROB_PERF_END(ocean_fft);

	FROB_PERF_BEGIN(ocean_resolve);

	GLfloat* ocean_data = malloc(ocean_N * ocean_N * 4 * sizeof(GLfloat));
	GLfloat* displacement = malloc(ocean_N * ocean_N * 2 * sizeof(GLfloat));

	float lambda = -1.f;

	// Resolve
	float signs[2] = { 1.f, -1.f };
	for (int m = 0; m < ocean_N; ++m) {
		for (int n = 0; n < ocean_N; ++n) {
			int index = m * ocean_N + n;

			float sgn = signs[(n + m) & 1];

			h_tilde[index].x *= sgn;
			h_tilde_slopex[index].x *= sgn;
			h_tilde_slopez[index].x *= sgn;
			h_tilde_dx[index].x *= sgn;
			h_tilde_dz[index].x *= sgn;

			ocean_point_t point;

			point.height = h_tilde[index].x;
			point.displacement.x = h_tilde_dx[index].x * lambda;
			point.displacement.y = h_tilde_dz[index].x * lambda;

			point.normal.x = -h_tilde_slopex[index].x;
			point.normal.y = 1.f;
			point.normal.z = -h_tilde_slopez[index].x;
			normalize_v3(&point.normal);

			ocean_data[index * 4 + 0] = point.normal.x;
			ocean_data[index * 4 + 1] = point.normal.y;
			ocean_data[index * 4 + 2] = point.normal.z;
			ocean_data[index * 4 + 3] = point.height;

			displacement[index * 2 + 0] = point.displacement.x;
			displacement[index * 2 + 1] = point.displacement.y;
		}
	}
	FROB_PERF_END(ocean_resolve);
	FROB_PERF_BEGIN(ocean_upload);

	//glUseProgram(ocean_compute.program);
	//glDispatchCompute(N / 32, N, 1);


	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ocean_buffers[OceanBuffer_OceanData]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(vec4)*ocean_N*ocean_N, ocean_data);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ocean_buffers[OceanBuffer_Displacement]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(vec2)*ocean_N*ocean_N, displacement);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	free(ocean_data);
	free(displacement);

	FROB_PERF_END(ocean_upload);

	FROB_PERF_BEGIN(ocean_rasterize);

#if !OCEAN_DEBUG
	glBindFramebuffer(GL_FRAMEBUFFER, ocean_rasterize_fbo.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ocean_rasterize_fbo.textures[ocean_rasterize_fbo.back_buffer], 0);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, TEXTURE_SIZE, TEXTURE_SIZE);
#endif

	glBindVertexArray(ocean_rasterize_vao);
	glUseProgram(ocean_rasterize.program);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ocean_buffers[OceanBuffer_Indices]);
	glDrawElements(GL_TRIANGLE_STRIP, ocean_num_indices, GL_UNSIGNED_INT, 0);

#if !OCEAN_DEBUG
	ocean_rasterize_fbo.back_buffer = (ocean_rasterize_fbo.back_buffer + 1) % 2;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, ocean_rasterize_fbo.textures[ocean_rasterize_fbo.back_buffer]);
	glPopAttrib();
#endif
	FROB_PERF_END(ocean_rasterize);

	FROB_PERF_END(ocean_calculate);

	CHECK_FOR_GL_ERRORS("ocean calculate end");
}
