#include "demo.h"
#include "shader.h"
#include "main.h"
#include "util.h"
#include "music.h"
#include "frob_math.h"
#include "edison2015/ocean.h"

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

	ocean_init();

	//load_shader(SHADER_PERLIN_NOISE_HEIGHTS_GLSL, &pnh);
	//u_sync = glGetUniformLocation(intro.program, "syc");
	//u_sync2 = glGetUniformLocation(intro.program, "s2");

	//glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 64);

#ifdef _DEBUG
	checkForGLErrors("postInit");
#endif
	
	glClearColor(1.f, 0.f, 1.f, 1.f);
}

void render_demo() {

	//ocean_calculate();

	glClear(GL_COLOR_BUFFER_BIT);
	render(&ocean_draw);
}