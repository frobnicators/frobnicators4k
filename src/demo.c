#include "demo.h"
#include "shader.h"

struct shader_t shader;

void init_demo() {
	load_shader("gradient.glsl", &shader);
}

void update_demo() {

}

void render_demo() {
	glClearColor(1.f, 0.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	render(&shader);
}