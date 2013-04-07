#include "demo.h"
#include "shader.h"

struct shader_t shader;

void init_demo() {
	shader = load_shader("shaders/gradient.glsl");
}

void update_demo(float dt, float t) {

}

void render_demo(float dt, float t) {
	glClearColor(1.f, 0.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	render(&shader, t);
}