#include "demo.h"
#include "shader.h"
#include "main.h"
#include "util.h"

static struct shader_t intro;
//static GLuint u_res;
//static struct fbo_t fbo;

void init_demo() {
	load_shader("s2", &intro);
//	u_res = glGetUniformLocation(boxfilter.program, "res");
	glClearColor(1.f, 0.f, 1.f, 1.f);
}

void update_demo() {

}

void render_demo() {
	glClear(GL_COLOR_BUFFER_BIT);
	render(&intro);
}