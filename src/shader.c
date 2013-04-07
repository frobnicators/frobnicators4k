#include "shader.h"
#include "klister.h"
#include "pack.h"
#include "main.h"
#include "util.h"

extern DWORD width, height;

#if _DEBUG
#include <stdlib.h>
#include <stdio.h>
#endif

static GLuint vertex_shader;
static float ortho[16];
static const unsigned char index[] = { 0, 1, 2 , 3 };


GLuint build_shader(const char * src, GLenum type) {
	GLuint shader = glCreateShader(type);
#if SOME_DEBUG
	GLint compile_status;
#endif
	

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
#if SOME_DEBUG
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE) {
		char buffer[2048];

		glGetShaderInfoLog(shader, 2048, NULL, buffer);
#if _DEBUG
		printf("Shader compile error. Source:\n%s\n", src);
		printf("Error in shader: %s\n", buffer);
#endif
		MessageBox(NULL, buffer, "Shader compile error", MB_OK | MB_ICONERROR);
		terminate();
	}
#endif
	return shader;
}

void init_shaders() {
	static const float quad[] = {
		0.f, 0.f,	0.f, 1.f,
		0.f, 1.f,	0.f, 0.f,
		1.f, 1.f,	1.f, 0.f,
		1.f, 0.f,	1.f, 1.f,
	};
	struct file_data_t vert_data;
	float w,h;
#if _DEBUG
	struct file_data_t common;
	char * src;

	read_data("shaders/vertex.glsl", &vert_data);
	read_data("shaders/common.glsl", &common);
	src = (char*)malloc(strlen(common.data) + strlen(vert_data.data) + 3);
	sprintf(src, "%s\n%s\n", common.data, vert_data.data);
#else
	const char * src;
	read_data("shaders/vertex.glsl", &vert_data);
	src = vert_data.data;
#endif

	vertex_shader = build_shader(src, GL_VERTEX_SHADER);
	w = (float) width;
	h = (float) height;

	/* Create orthographic projection matrix */
	ortho[0] = 2.f / w;
	ortho[5] = -2.f / h;
	ortho[10] = -1.f;
	ortho[12] = -1.f;
	ortho[13] = 1.f;
	ortho[15] = 1.f;

	/* Set the rest to 0, this is smaller than memset */
	ortho[1] = 0.f;
	ortho[2] = 0.f;
	ortho[3] = 0.f;
	ortho[4] = 0.f;
	ortho[6] = 0.f;
	ortho[7] = 0.f;
	ortho[8] = 0.f;
	ortho[9] = 0.f;
	ortho[11] = 0.f;
	ortho[14] = 0.f;
}

void load_shader(const char * name, struct shader_t * shader) {
	struct file_data_t frag_data;
	GLuint fragment_shader, matrix;
#if SOME_DEBUG
	GLint link_status;
#endif

#if _DEBUG
	struct file_data_t common;
	char * src;

	read_data("shaders/common.glsl", &common);
	src = (char*)malloc(strlen(common.data) + strlen(frag_data.data) + 3);
	sprintf(src, "%s\n%s\n", common.data, frag_data.data);

#else
	const char * src;
	read_data(name, &frag_data);
	src = frag_data.data;
#endif
	fragment_shader = build_shader(src, GL_FRAGMENT_SHADER);
	shader->program = glCreateProgram();
	glAttachShader(shader->program, vertex_shader);
	glAttachShader(shader->program, fragment_shader);
	glLinkProgram(shader->program);

#if SOME_DEBUG
	glGetProgramiv(shader->program, GL_LINK_STATUS, &link_status);
	if(!link_status) {
		char buffer[2048];
		glGetProgramInfoLog(shader->program, 2048, NULL, buffer);
#if _DEBUG
		printf("Link error in shader %s: %s\n", name, buffer);
#endif
		MessageBox(NULL, buffer, "Shader compile error", MB_OK | MB_ICONERROR);
	}
#endif

	glUseProgram(shader->program);
	matrix = glGetUniformLocation(shader->program, "matrix");
	shader->time = glGetUniformLocation(shader->program, "time");
	glUniformMatrix4fv(matrix, 1, GL_FALSE, ortho);
}

void render(struct shader_t * shader, float t) {
	glUseProgram(shader->program);
	glUniform1f(shader->time, t);
}