#ifndef SHADER_H
#define SHADER_H

#include "klister.h"
#include "shaders.h"

typedef struct shader_t {
	GLuint program;
	GLuint time;
	GLuint light_dir;
	GLuint light_color;
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
// Varargs: *shader_stages
void load_shader(shader_t * shader, unsigned int num_stages, ...);


void render(shader_t * shader);

#endif