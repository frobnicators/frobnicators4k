#include "demo.h"
#include "shader.h"
#include "main.h"
#include "util.h"
#include "music.h"
#include "frob_math.h"
#include "debug.h"
#include "edison2015/ocean.h"

static float synth = 0.f;

vec2 wind = { 32.f, 16.f };
float A = 0.0005f;
float g = 9.81f;

#if _DEBUG
#include <stdio.h>
#endif

void init_demo() {
	ocean_init();

#ifdef _DEBUG
	checkForGLErrors("postInit");
#endif

	ocean_seed(&wind, A, g);
	
	glClearColor(1.f, 0.f, 1.f, 1.f);
}

void render_demo() {
	ocean_calculate();
#ifdef _DEBUG
	checkForGLErrors("postCalculate");
#endif

	glClear(GL_COLOR_BUFFER_BIT);
	render(&ocean_draw);
#ifdef _DEBUG
	checkForGLErrors("postFrame");
#endif
}