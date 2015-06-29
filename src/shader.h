#ifndef SHADER_H
#define SHADER_H

#include "klister.h"
#include "shaders.h"

typedef struct {
	GLuint program;
	GLuint time;
} shader_t;

void init_shaders();
void init_gl();

/*
 * SHADER_TYPE is the type of the defines SHADER_{shader_name} in shaders.h
 */
typedef struct {
	GLenum type; // vertex,fragment etc
	unsigned int num_parts;
	SHADER_TYPE parts[];
} shader_stage_t;

extern const shader_stage_t default_vertex_stage;

// Create a shader.
// @param setup_default_rendering Set to 1 to set parameters used for the standard (fullscreen quad) rendering
// Varargs: *shader_stages
void load_shader(shader_t * shader, char setup_default_rendering, unsigned int num_stages, ...);


void render(shader_t * shader);

#endif