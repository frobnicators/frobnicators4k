#include "demo.h"
#include "shader.h"
#include "fbo.h"
#include "main.h"
#include "util.h"

static struct shader_t gradient, boxfilter;
static GLuint u_res;
static struct fbo_t fbo;

void init_demo() {
	load_shader("gradient.glsl", &gradient);
	load_shader("boxfilter.glsl", &boxfilter);
	u_res = glGetUniformLocation(boxfilter.program, "res");
	glUniform2i(u_res, width, height);

	create_fbo(width, height, GL_RGB8, &fbo);

	glClearColor(1.f, 0.f, 1.f, 1.f);
}

void update_demo() {

}

void render_demo() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
	glClearColor(1.f, 0.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	render(&gradient);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	render(&boxfilter);
}