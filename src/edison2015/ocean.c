#include "ocean.h"
#include "main.h"
#include "debug.h"
#include "fft.h"
#include "perf.h"
#include "util.h"
#include "fbo.h"
#include "camera.h"
#include "shader.h"

#include <stdlib.h>

static const int ocean_N = 128;
static const float ocean_length = 128.f;

// Changing this changes the influence of the wind in the initial state
// The correct value for the formula i 2, but higher values may give nicer results
static const float phillips_kdw_pow = 8.f;

static const float dampening = 0.001f;
static const float lambda = -0.5f;


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

static shader_t ocean_draw;
static shader_t fft_shader;
static shader_t ocean_compute;

static shader_stage_t ocean_vert = { GL_VERTEX_SHADER, 1, { SHADER_OCEAN_VERT_GLSL } };
static shader_stage_t ocean_frag = { GL_FRAGMENT_SHADER, 4, { SHADER_COMMON_GLSL, SHADER_NOISE_GLSL, SHADER_RAYMARCH_GLSL, SHADER_OCEAN_FRAG_GLSL } };

static GLuint ocean_vao;
fbo_t ocean_fbo;

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

static GLuint u_ocean_projview;
static GLuint u_ocean_model;
static GLuint u_ocean_view;
static GLuint u_ocean_camera_pos;

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

	load_shader(&ocean_draw, 2, &ocean_vert, &ocean_frag);

	// Generate index and vertex buffer
	int Nplus1 = ocean_N + 1;

	ocean_vertex_data_t* vertices = malloc(Nplus1*Nplus1 * sizeof(ocean_vertex_data_t));
	unsigned int* indices = malloc(Nplus1*Nplus1 * sizeof(unsigned int)*2);

	for (int m = 0; m < Nplus1; ++m) {
		for (int n = 0; n < Nplus1; ++n) {
			int index = m * Nplus1 + n;

			vertices[index].pos.x = (n - ocean_N / 2.f)*ocean_length/ocean_N;
			vertices[index].pos.y = 0.f;
			vertices[index].pos.z = (m - ocean_N / 2.f)*ocean_length/ocean_N;

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

	glGenVertexArrays(1, &ocean_vao);
	glBindVertexArray(ocean_vao);

	glUseProgram(ocean_draw.program);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	u_ocean_projview = glGetUniformLocation(ocean_draw.program, "PV");
	u_ocean_view = glGetUniformLocation(ocean_draw.program, "V");
	u_ocean_model = glGetUniformLocation(ocean_draw.program, "M");
	u_ocean_camera_pos = glGetUniformLocation(ocean_draw.program, "cp");

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

	create_fbo(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT, 0, &ocean_fbo);

	glBindTexture(GL_TEXTURE_2D, ocean_fbo.textures[TextureType_Color]);

	// Use same depth as main_fbo
	glBindFramebuffer(GL_FRAMEBUFFER, ocean_fbo.fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, main_fbo.textures[TextureType_Depth], 0);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
			complex_mul(h_tilde + index, &tmp, h_tilde_slopex + index);
			tmp.y = k.y;
			complex_mul(h_tilde + index, &tmp, h_tilde_slopez + index);

			if (k_norm < dampening*dampening) {
				memset(h_tilde_dx + index, 0, sizeof(complex));
				memset(h_tilde_dz + index, 0, sizeof(complex));
			}
			else {
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
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ocean_buffers[OceanBuffer_OceanData]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(vec4)*ocean_N*ocean_N, ocean_data);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ocean_buffers[OceanBuffer_Displacement]);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(vec2)*ocean_N*ocean_N, displacement);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	free(ocean_data);
	free(displacement);

	FROB_PERF_END(ocean_upload);

	FROB_PERF_END(ocean_calculate);

	CHECK_FOR_GL_ERRORS("ocean calculate end");
	//glUseProgram(ocean_compute.program);
	//glDispatchCompute(N / 32, N, 1);
}

static void render_internal(int x, int y) {
	// TODO: Model matrix

	glDrawElements(GL_TRIANGLE_STRIP, ocean_num_indices, GL_UNSIGNED_INT, 0);
}

void ocean_render() {

	FROB_PERF_BEGIN(ocean_render);
	glBindFramebuffer(GL_FRAMEBUFFER, ocean_fbo.fbo);
	glBindVertexArray(ocean_vao);
	glUseProgram(ocean_draw.program);

	glUniformMatrix4fv(u_ocean_projview,1, GL_FALSE, (float*)&camera.view_proj_matrix);
	glUniformMatrix4fv(u_ocean_view,1, GL_FALSE, (float*)&camera.view_matrix);
	glUniform3fv(u_ocean_camera_pos, 1, (float*)&camera.position);
	upload_light(&ocean_draw);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ocean_buffers[OceanBuffer_Indices]);

	glBindTexture(GL_TEXTURE_2D, main_fbo.textures[TextureType_Color]);

	glClear(GL_COLOR_BUFFER_BIT);

	render_internal(0, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	FROB_PERF_END(ocean_render);
}
