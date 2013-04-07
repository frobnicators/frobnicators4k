#include "shader.h"
#include "klister.h"
#include "pack.h"
#include "main.h"

#if _DEBUG
#include <stdlib.h>
#include <stdio.h>
#endif

GLuint vertex_shader;

GLuint build_shader(const char * src, GLenum type) {
	GLuint shader = glCreateShader(type);
	GLint compile_status;
	char buffer[2048];

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
#if _DEBUG
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE) {

		glGetShaderInfoLog(shader, 2048, NULL, buffer);
		printf("Shader compile error. Source:\n%s\n", src);
		printf("Error in shader: %s\n", buffer);
		MessageBox(NULL, "Shader compile error", "Shader error", MB_OK | MB_ICONERROR);
		terminate();
	}
#endif

}

void init_shaders() {
	struct file_data_t vert_data = read_data("shaders/vertex.glsl");
#if _DEBUG
	struct file_data_t common = read_data("shaders/common.glsl");
	const unsigned long len = strlen(common.data) + strlen(vert_data.data) + 3;
	char * src = (char*)malloc(len);
	sprintf(src, "%s\n%s\n", common.data, vert_data.data);
#else
	const char * src = vert_data.data;
#endif

	vertex_shader = build_shader(src, GL_VERTEX_SHADER);
}

GLuint load_shader(const char * name) {
	struct file_data_t frag_data = read_data(name);
	GLuint fragment_shader, program;
#if _DEBUG
	struct file_data_t common = read_data("shaders/common.glsl");
	GLint link_status;
	char * src = (char*)malloc(strlen(common.data) + strlen(frag_data.data) + 3);
	sprintf(src, "%s\n%s\n", common.data, frag_data.data);

#else
	const char * src = frag_data.data;
#endif
	fragment_shader = build_shader(src, GL_FRAGMENT_SHADER);
	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

#if _DEBUG
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if(!link_status) {
		char buffer[2048];
		glGetProgramInfoLog(program, 2048, NULL, buffer);
		printf("Link error in shader %s: %s\n", name, buffer);
		MessageBox(NULL, "Link error in shader.", "Shader error", MB_OK | MB_ICONERROR);
	}
#endif

	return 0;
}