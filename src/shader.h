#ifndef SHADER_H
#define SHADER_H

#include "klister.h"

struct shader_t {
	GLuint program;
	GLuint time;
};

void init_shaders();
void init_gl();

void load_shader(const char * name, struct shader_t * shader);

void render(struct shader_t * shader);


#endif