#include "demo.h"
#include "shader.h"
#include "main.h"
#include "util.h"
#include "music.h"

#include <math.h>

static struct shader_t intro, pnh;
static GLuint u_sync, u_sync2;

static float synth = 0.f;

#if _DEBUG
#include <stdio.h>
#endif

void init_demo() {
	load_shader(SHADER_INTRO_GLSL, &intro);
	load_shader(SHADER_PERLIN_NOISE_HEIGHTS_GLSL, &pnh);
	u_sync = glGetUniformLocation(intro.program, "s");
	u_sync2 = glGetUniformLocation(intro.program, "s2");
	glClearColor(1.f, 0.f, 1.f, 1.f);
}

void render_demo() {
	struct shader_t * current;
	float env;

	if(time < 48.f) {
		current = &intro;
	} else {
		current = &pnh;
	}

	env = envelope(0, 0, 0) + envelope(0, 1, 0);
	glUseProgram(current->program);
	glUniform1f(u_sync, env);
	env = envelope(2, 0, 0) + envelope(2, 1, 0);
	glUniform1f(u_sync2, env);

	glClear(GL_COLOR_BUFFER_BIT);
	render(current);
}