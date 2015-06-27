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
	ocean_init();

#ifdef _DEBUG
	checkForGLErrors("postInit");
#endif
	
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