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

static const int ocean_N = 512;
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
	vec3 normal;
	float height;
	vec2 displacement;
} ocean_point_t;

static complex* h_tilde0;
static fft_t ocean_fft;

static shader_t ocean_draw;
static shader_t fft_shader;
static shader_t ocean_compute;
static shader_t ocean_resolve;

static shader_stage_t ocean_vert = { GL_VERTEX_SHADER, 1, { SHADER_OCEAN_VERT_GLSL } };
static shader_stage_t ocean_frag = { GL_FRAGMENT_SHADER, 4, { SHADER_COMMON_GLSL, SHADER_NOISE_GLSL, SHADER_RAYMARCH_GLSL, SHADER_OCEAN_FRAG_GLSL } };

static shader_stage_t ocean_resolve_stage = { GL_COMPUTE_SHADER, 3, { SHADER_MATH_GLSL, SHADER_OCEAN_COMPUTE_SHARED_GLSL, SHADER_OCEAN_RESOLVE_GLSL } };
static shader_stage_t ocean_compute_stage = { GL_COMPUTE_SHADER, 3, { SHADER_MATH_GLSL, SHADER_OCEAN_COMPUTE_SHARED_GLSL, SHADER_OCEAN_COMPUTE_GLSL } };

static GLuint h_tilde_buffers[2];
static GLuint h_tilde_slopex_buffers[2];
static GLuint h_tilde_slopez_buffers[2];
static GLuint h_tilde_dx_buffers[2];
static GLuint h_tilde_dz_buffers[2];

static GLuint h_tilde0_buffer;

static GLuint ocean_resolve_N;
static GLuint ocean_compute_N;
static GLuint ocean_compute_ocean_length;
static GLuint ocean_compute_G; //ocean_g
static GLuint ocean_compute_dampening2;

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

static void create_fft_buffers(GLuint* buffers)
{
	const int NxN = ocean_N * ocean_N;
	glGenBuffers(2, buffers);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(complex)*NxN, NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(complex)*NxN, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void ocean_init() {
	const int NxN = ocean_N * ocean_N;
	h_tilde0 = malloc(NxN * sizeof(complex)*2);

	h_tilde = malloc(NxN * sizeof(complex));
	h_tilde_slopex = malloc(NxN * sizeof(complex));
	h_tilde_slopez = malloc(NxN * sizeof(complex));
	h_tilde_dx = malloc(NxN * sizeof(complex));
	h_tilde_dz = malloc(NxN * sizeof(complex));

	create_fft_buffers(h_tilde_buffers);
	create_fft_buffers(h_tilde_slopex_buffers);
	create_fft_buffers(h_tilde_slopez_buffers);
	create_fft_buffers(h_tilde_dx_buffers);
	create_fft_buffers(h_tilde_dz_buffers);

	fft_init(&ocean_fft, ocean_N);

	load_shader(&ocean_draw, 2, &ocean_vert, &ocean_frag);
	load_shader(&ocean_resolve, 1, &ocean_resolve_stage);
	load_shader(&ocean_compute, 1, &ocean_compute_stage);

	ocean_resolve_N = glGetUniformLocation(ocean_resolve.program, "N");
	ocean_compute_N = glGetUniformLocation(ocean_compute.program, "N");
	ocean_compute_G = glGetUniformLocation(ocean_compute.program, "G");
	ocean_compute_ocean_length = glGetUniformLocation(ocean_compute.program, "oL");
	ocean_compute_dampening2 = glGetUniformLocation(ocean_compute.program, "dmp2");

	// Generate h~0 buffer
	glGenBuffers(1, &h_tilde0_buffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, h_tilde0_buffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 2*sizeof(complex)*NxN, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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

	// Generate ocean data render buffers

	glGenBuffers(OceanBuffer_Count, ocean_buffers);

	glBindBuffer(GL_ARRAY_BUFFER,  ocean_buffers[OceanBuffer_VertexData]);
	glBufferData(GL_ARRAY_BUFFER, Nplus1*Nplus1 * sizeof(ocean_vertex_data_t), vertices, GL_STATIC_DRAW); // TODO: Change to COPY
	glVertexAttribPointer(0, 3, GL_FLOAT, FALSE, sizeof(ocean_vertex_data_t), 0);
	glVertexAttribIPointer(1, 1, GL_INT, sizeof(ocean_vertex_data_t), (GLvoid*)offsetof(ocean_vertex_data_t, index));

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_OceanData]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ocean_N*ocean_N * sizeof(vec4), NULL, GL_DYNAMIC_DRAW); // TODO: Change to COPY when written from GPU

	glBindBuffer(GL_SHADER_STORAGE_BUFFER,  ocean_buffers[OceanBuffer_Displacement]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ocean_N*ocean_N * sizeof(vec2), NULL, GL_DYNAMIC_DRAW); // TODO: Change to COPY when written from GPU

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  ocean_buffers[OceanBuffer_Indices]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ocean_num_indices*sizeof(unsigned int), indices, GL_STATIC_DRAW);

	create_fbo(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT, 1, &ocean_fbo);

	glBindTexture(GL_TEXTURE_2D, ocean_fbo.textures[TextureType_Color]);

	// Use same depth as main_fbo
	glBindFramebuffer(GL_FRAMEBUFFER, ocean_fbo.fbo);
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
			complex* ht = h_tilde0 + (m * ocean_N + n) * 2;
			hTilde0(n, m, ht);
			hTilde0(-n, -m, ht + 1);
			complex_conj(ht + 1);
		}
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, h_tilde0_buffer);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * sizeof(complex)*ocean_N * ocean_N, h_tilde0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glUseProgram(ocean_compute.program);
	glUniform1i(ocean_compute_N, ocean_N);
	glUniform1f(ocean_compute_G, ocean_g);
	glUniform1f(ocean_compute_ocean_length, ocean_length);
	glUniform1f(ocean_compute_dampening2, dampening*dampening);
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

static GLuint run_fft(complex* data, GLuint* buffers) {
	GLuint outbuffer = fft_compute(&ocean_fft, buffers[0], buffers[1]);
	return outbuffer;
}

void ocean_calculate()
{
	FROB_PERF_BEGIN(ocean_calculate);

	FROB_PERF_BEGIN(ocean_hTilde);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, h_tilde_buffers[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, h_tilde_slopex_buffers[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, h_tilde_slopez_buffers[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, h_tilde_dx_buffers[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, h_tilde_dz_buffers[0]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, h_tilde0_buffer);

	glUseProgram(ocean_compute.program);

	glUniform1f(ocean_compute.time, time);

	glDispatchCompute(ocean_N / 16, ocean_N / 16, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	FROB_PERF_END(ocean_hTilde);

	FROB_PERF_BEGIN(ocean_fft);

	int buffer_size = sizeof(complex)*ocean_N* ocean_N;

	FROB_PERF_BEGIN(ocean_fft_first); // This scope indicates if the problem is memory sync (which it is...)
	GLuint ht = run_fft(h_tilde, h_tilde_buffers);
	FROB_PERF_END(ocean_fft_first);

	GLuint htsx = run_fft(h_tilde_slopex, h_tilde_slopex_buffers);
	GLuint htsz = run_fft(h_tilde_slopez, h_tilde_slopez_buffers);
	GLuint htdx = run_fft(h_tilde_dx, h_tilde_dx_buffers);
	GLuint htdz = run_fft(h_tilde_dz, h_tilde_dz_buffers);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	FROB_PERF_END(ocean_fft);

	FROB_PERF_BEGIN(ocean_resolve);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ht);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, htsx);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, htsz);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, htdx);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, htdz);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ocean_buffers[OceanBuffer_OceanData]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ocean_buffers[OceanBuffer_Displacement]);

	glUseProgram(ocean_resolve.program);
	glUniform1i(ocean_resolve_N, ocean_N);

	glDispatchCompute(ocean_N / 16, ocean_N / 16, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	FROB_PERF_END(ocean_resolve);

	FROB_PERF_END(ocean_calculate);

	CHECK_FOR_GL_ERRORS("ocean calculate end");
}

static void render_internal(int x, int y) {
	const size_t w = 15;
	const size_t h = 15;
	mat4 model;
	memset(&model, 0, sizeof(mat4));
	model.c0.x = 1;
	model.c1.y = 1;
	model.c2.z = 1;
	model.c3.w = 1;

	for (unsigned int y = 0; y < h; y++){
		for (unsigned int x = 0; x < w; x++){
			model.c3.x = x * ocean_length;
			model.c3.y = 0;
			model.c3.z = y * ocean_length;

			glUniformMatrix4fv(u_ocean_model, 1, GL_FALSE, (float*)&model);
			glDrawElements(GL_TRIANGLE_STRIP, ocean_num_indices, GL_UNSIGNED_INT, 0);
		}
	}
}

void ocean_render() {

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ocean_buffers[OceanBuffer_OceanData]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ocean_buffers[OceanBuffer_Displacement]);

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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	render_internal(0, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	FROB_PERF_END(ocean_render);
}
