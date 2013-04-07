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

GLuint vertex_shader;
float ortho[16];

GLuint build_shader(const char * src, GLenum type) {
	GLuint shader = glCreateShader(type);
	GLint compile_status;

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
#if _DEBUG
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE) {
		char buffer[2048];

		glGetShaderInfoLog(shader, 2048, NULL, buffer);
		printf("Shader compile error. Source:\n%s\n", src);
		printf("Error in shader: %s\n", buffer);
		MessageBox(NULL, "Shader compile error", "Shader error", MB_OK | MB_ICONERROR);
		terminate();
	}
#endif
	return shader;
}

void init_shaders() {
	struct file_data_t vert_data = read_data("shaders/vertex.glsl");
	float w,h;
	int i;
#if _DEBUG
	struct file_data_t common = read_data("shaders/common.glsl");
	const unsigned long len = strlen(common.data) + strlen(vert_data.data) + 3;
	char * src = (char*)malloc(len);
	sprintf(src, "%s\n%s\n", common.data, vert_data.data);
#else
	const char * src = vert_data.data;
#endif

	vertex_shader = build_shader(src, GL_VERTEX_SHADER);
/*	for(i = 0; i<16; ++i) { 
		ortho[i] = 0.f;
	}*/
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

struct shader_t load_shader(const char * name) {
	struct file_data_t frag_data = read_data(name);
	GLuint fragment_shader;
	struct shader_t shader;
#if _DEBUG
	struct file_data_t common = read_data("shaders/common.glsl");
	GLint link_status;
	char * src = (char*)malloc(strlen(common.data) + strlen(frag_data.data) + 3);
	sprintf(src, "%s\n%s\n", common.data, frag_data.data);

#else
	const char * src = frag_data.data;
#endif
	fragment_shader = build_shader(src, GL_FRAGMENT_SHADER);
	shader.program = glCreateProgram();
	glAttachShader(shader.program, vertex_shader);
	glAttachShader(shader.program, fragment_shader);
	glLinkProgram(shader.program);

#if _DEBUG
	glGetProgramiv(shader.program, GL_LINK_STATUS, &link_status);
	if(!link_status) {
		char buffer[2048];
		glGetProgramInfoLog(shader.program, 2048, NULL, buffer);
		printf("Link error in shader %s: %s\n", name, buffer);
		MessageBox(NULL, "Link error in shader.", "Shader error", MB_OK | MB_ICONERROR);
	}
#endif

	shader.matrix = glGetUniformLocation(shader.program, "matrix");
	shader.time = glGetUniformLocation(shader.program, "time");
	
	return shader;
}

void render(struct shader_t * shader, float t) {
	glUseProgram(shader->program);
	glUniform1f(shader->time, t);
	glUniformMatrix4fv(shader->matrix, 1, GL_FALSE, ortho);
}