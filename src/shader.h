#ifndef SHADER_H
#define SHADER_H

#include "klister.h"
#include "shaders.h"

struct shader_t {
	GLuint program;
	GLuint time;
};

void init_shaders();
void init_gl();

/*
 * SHADER_TYPE is the type of the defines SHADER_{shader_name} in shaders.h
 */
void load_shader(SHADER_TYPE name, struct shader_t * shader);

void render(struct shader_t * shader);


#endif