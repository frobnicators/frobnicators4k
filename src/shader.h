#ifndef SHADER_H
#define SHADER_H

#include "klister.h"

struct shader_t {
	GLuint program;
	GLuint matrix;
	GLuint time;
};

void init_shaders();

struct shader_t load_shader(const char * name);

void render(struct shader_t * shader, float t);

#endif