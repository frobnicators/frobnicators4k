#include "demo.h"
#include "shader.h"
#include "main.h"
#include "util.h"
#include "music.h"
#include "math.h"

static struct shader_t intro;
static struct shader_t compute;
static GLuint output;

static float synth = 0.f;

#if _DEBUG
#include <stdio.h>
#endif

void init_demo() {
	static vec4 colors[1024*1024];
	for (int y = 0; y < 1024; ++y) {
		for (int x = 0; x < 1024; ++x) {
			int index = y*1024 + x;
			colors[index].x = x / 1024.f;
			colors[index].y = y / 1024.f;
			colors[index].z = 0.f;
			colors[index].w = 1.f;
		}
	}
	load_shader(ShaderType_Visual, SHADER_INTRO_GLSL, &intro);
	load_shader(ShaderType_Compute, SHADER_COMPUTE_GLSL, &compute);
	//load_shader(SHADER_PERLIN_NOISE_HEIGHTS_GLSL, &pnh);
	//u_sync = glGetUniformLocation(intro.program, "syc");
	//u_sync2 = glGetUniformLocation(intro.program, "s2");

	//glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 64);
	glGenBuffers(1, &output);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, output);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 1024 * 1024 * sizeof(vec4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, output);
	glUseProgram(intro.program);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, output);

	glClearColor(1.f, 0.f, 1.f, 1.f);
}

void render_demo() {
	//if(time < 33.f) {
	//} else {
		//current = &pnh;
	//}

	//glUniform1f(u_sync, envelope(0, 0, 0) + envelope(0, 1, 0));
	//glUniform1f(u_sync2,envelope(2, 0, 0) + envelope(2, 1, 0));

	glUseProgram(compute.program);
	glDispatchCompute(1024 / 32, 1024, 1);

	glClear(GL_COLOR_BUFFER_BIT);
	render(&intro);
}