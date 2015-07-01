#include "demo.h"
#include "shader.h"
#include "main.h"
#include "util.h"
#include "music.h"
#include "frob_math.h"
#include "debug.h"
#include "perf.h"
#include "edison2015/ocean.h"
#include "camera.h"
#include "fbo.h"

static float synth = 0.f;

vec2 wind = { 32.f, 32.f };
float A = 0.0005f;
float g = 9.81f;

fbo_t main_fbo;

camera_t camera = {
	.fovy = 70.f,
	.znear = 0.1f,
	.zfar = 1000.f,
	.position = { 0.f, 15.f, 0.f },
	.look_direction = { 0.f, -0.1f, 1.f }
};

static shader_t raymarch, passthru;
static shader_stage_t raymarch_frag = { GL_FRAGMENT_SHADER, 4, { SHADER_COMMON_GLSL, SHADER_NOISE_GLSL, SHADER_RAYMARCH_GLSL, SHADER_RAYMARCH_FRAG_GLSL } };
static shader_stage_t passthru_frag = { GL_FRAGMENT_SHADER, 2, { SHADER_COMMON_GLSL, SHADER_PASSTHRU_GLSL } };

#if _DEBUG
#include <stdio.h>
#endif

void init_demo() {
	ocean_init();

	load_shader(&raymarch, 1, 2, &default_vertex_stage, &raymarch_frag);
	load_shader(&passthru, 1, 2, &default_vertex_stage, &passthru_frag);

	camera.aspect = (float)width / height;
	normalize_v3(&camera.look_direction);
	update_camera_matrices(&camera);

	ocean_seed(&wind, A, g);

	create_fbo(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT, 1, &main_fbo);

	glClearColor(0.f, 0.f, 0.f, 0.f);
#ifdef _DEBUG
	checkForGLErrors("postInit");
#endif
}

void render_demo() {
	ocean_calculate();

#ifdef _DEBUG
	checkForGLErrors("postCalculate");
#endif

	{
		FROB_PERF_BEGIN(raymarch);
		glBindFramebuffer(GL_FRAMEBUFFER, main_fbo.fbo);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render(&raymarch);
		FROB_PERF_END(raymarch);

		ocean_render();

		glBindTexture(GL_TEXTURE_2D, main_fbo.textures[TextureType_Color]);
		render(&passthru);
		glBindTexture(GL_TEXTURE_2D, ocean_fbo.textures[TextureType_Color]);
		render(&passthru);
	}
#ifdef _DEBUG
	checkForGLErrors("postFrame");
#endif
}