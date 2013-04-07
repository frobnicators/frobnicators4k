#include "shader.h"
#include "klister.h"
#include "pack.h"
#include "main.h"
#include "util.h"

#if _DEBUG
#include <stdlib.h>
#include <stdio.h>
#endif

static GLuint vertex_shader, vbo[2];
static const unsigned char indices[] = { 0, 1, 2 , 3 };


static GLuint build_shader(const char * src, GLenum type) {
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
		-1.f, -1.f,
		-1.f, 1.f,
		1.f, 1.f,
		1.f, -1.f
	};
	static struct file_data_t vert_data;
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

	/* Create vbos */
	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
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
	read_data(name, &frag_data);

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
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void render(struct shader_t * shader, float t) {
	glUseProgram(shader->program);
	glUniform1f(shader->time, t);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, 0);
}