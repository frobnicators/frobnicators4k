#include "shader.h"
#include "klister.h"
#include "pack.h"
#include "main.h"

#if _DEBUG
#include <stdlib.h>
#include <stdio.h>
#endif

GLuint shader_stages[2]; /* First is set to vert shader, second is variated */

GLuint build_shader(const char * src, GLenum type) {
	GLuint shader = glCreateShader(type);
	GLint compile_status;
	char buffer[2048];

	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE) {
#if _DEBUG

		glGetShaderInfoLog(shader, 2048, NULL, buffer);
		printf("Shader compile error. Source:\n%s\n", src);
		printf("Error in shader: %s\n", buffer);
#endif
		MessageBox(NULL, "Shader compile error", "", MB_OK | MB_ICONERROR);
		terminate();
	}

}

void init_shaders() {
	struct file_data_t vert_data = read_data("default.vert");
#if _DEBUG
	struct file_data_t common = read_data("common.glsl");
	const unsigned long len = strlen(GLSL_VERSION_LINE) + strlen(common.data) + strlen(vert_data.data) + 3;
	char * src = (char*)malloc(len);
	sprintf(src, GLSL_VERSION_LINE "%s\n%s\n", common.data, vert_data.data);
#else
	const char * src = vert_data.data;
#endif

	shader_stages[0] = build_shader(src, GL_VERTEX_SHADER);
}

GLuint load_shader(const char * name) {
	struct file_data_t frag_data = read_data(name);
#if _DEBUG
	struct file_data_t common = read_data("common.glsl");
	char * src = (char*)malloc(strlen(GLSL_VERSION_LINE) + strlen(common.data) + strlen(frag_data.data) + 3);
	sprintf(src, GLSL_VERSION_LINE "%s\n%s\n", common.data, frag_data.data);

#else
	const char * src = frag_data.data;
#endif
	//shader_stages[1] = build_shader(src, GL_FRAGMENT_SHADER);
	return 0;
}