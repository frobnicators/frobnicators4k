#include "demo.h"
#include "shader.h"
#include "main.h"
#include "util.h"
#include "music.h"

static struct shader_t intro;//, pnh;
static GLuint u_sync, u_sync2;

static float synth = 0.f;

#if _DEBUG
#include <stdio.h>
#endif

void init_demo() {
	load_shader(SHADER_INTRO_GLSL, &intro);
	//load_shader(SHADER_PERLIN_NOISE_HEIGHTS_GLSL, &pnh);
	u_sync = glGetUniformLocation(intro.program, "syc");
	u_sync2 = glGetUniformLocation(intro.program, "s2");
	glClearColor(1.f, 0.f, 1.f, 1.f);
}

void render_demo() {
	//if(time < 33.f) {
	//} else {
		//current = &pnh;
	//}

	glUniform1f(u_sync,  envelope(0, 0, 0) + envelope(0, 1, 0));
	glUniform1f(u_sync2, envelope(2, 0, 0) + envelope(2, 1, 0));

	glClear(GL_COLOR_BUFFER_BIT);
	render(&intro);
}