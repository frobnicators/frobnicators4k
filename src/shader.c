#include "shader.h"
#include "klister.h"
#include "main.h"
#include "util.h"
#include "shaders.h"
#include "demo.h"

#include <stdio.h>
#if _DEBUG
#include <stdlib.h>

#include "demo.h"

#else

extern const char * _shaders[];

#endif

#if SOME_DEBUG
static SHADER_TYPE current_shader; /* Current shader compiling */
#endif


static const char * read_shader(SHADER_TYPE name);

static GLuint vertex_shader, vbo[2];
static const char * shader_src[2];
static const unsigned char indices[] = { 0, 1, 2 , 3 };

void init_gl() {
#if ENABLE_TEXTURES
	glEnable(GL_TEXTURE_2D);
#endif
}

static GLuint build_shader(GLenum type) {
	GLuint shader = glCreateShader(type);
#if SOME_DEBUG
	GLint compile_status;
#endif
	glShaderSource(shader, 2, shader_src, NULL);
	glCompileShader(shader);
#if SOME_DEBUG
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE) {
		char buffer[2048];
		char shader_name[32];

		glGetShaderInfoLog(shader, 2048, NULL, buffer);
#if _DEBUG
		memcpy(shader_name, current_shader, strlen(current_shader) + 1);
#else
		sprintf(shader_name, "Shader #%d", current_shader);
#endif
		{
			char * src = _strdup(shader_src[0]);
			char * split;
			int i=0;
			split =  strtok(src, "\n");
			while(split !=  NULL) {
				++i;
				printf("%d\t%s\n", i, split);
				split = strtok(NULL, "\n");
			}
			src = _strdup(shader_src[1]);
			split = strtok(src, "\n");
			while(split !=  NULL) {
				++i;
				printf("%d\t%s\n", i, split);
				split = strtok(NULL, "\n");
			}
		}
		MessageBox(NULL, buffer, shader_name, MB_OK | MB_ICONERROR);
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

	shader_src[0] = read_shader(SHADER_COMMON_GLSL);
	shader_src[1] = read_shader(SHADER_VERTEX_GLSL);

#if SOME_DEBUG
	current_shader = SHADER_VERTEX_GLSL;
#endif

	vertex_shader = build_shader(GL_VERTEX_SHADER);

	/* Create vbos */
	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

/* Loads the shader. This leaves the created shader active */
void load_shader(SHADER_TYPE name, struct shader_t * shader) {
	GLuint fragment_shader;

#if SOME_DEBUG
	GLint link_status;
	current_shader = name;
#endif

	shader_src[1] = read_shader(name);
	fragment_shader = build_shader(GL_FRAGMENT_SHADER);
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
	shader->time = glGetUniformLocation(shader->program, "time");
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void render(struct shader_t * shader) {
	glUseProgram(shader->program);
	glUniform1f(shader->time, time);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, 0);
}

/* Functions for reading shader */

#if _DEBUG
char * find_path(const char * name) {
	char * path;
	const char * fmt_shared = "shaders/shared/%s";
	const char * fmt_special = "shaders/" DEMO_FOLDER "/%s"; 

	int len = _scprintf(fmt_special	, name) + 1; /* Include null terminator */
	DWORD dwAttrib;

	path = (char*) malloc(len);
	sprintf_s(path, len, fmt_special	, name);

	dwAttrib = GetFileAttributes(path);
	if(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
		return path;
	}

	len = _scprintf(fmt_shared, name) + 1; /* Include null terminator */

	path = (char*) malloc(len);
	sprintf_s(path, len, fmt_shared, name);

	dwAttrib = GetFileAttributes(path);
	if(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
		return path;
	} else {
		char buffer[128];
		sprintf(buffer, "Could not find file: %s", name);
		MessageBox(NULL, buffer, "File error", MB_OK | MB_ICONEXCLAMATION);
		terminate();
	}
}
#endif

#if _DEBUG
struct shader_entry_t {
	const char * name;
	const char * data;
};
struct shader_entry_t * shaders = NULL;
int num_shaders = 0;

static void add_file(const char * name, const char * data) {
	struct shader_entry_t entry;
	entry.name = _strdup(name);
	entry.data = data;

	num_shaders++;

	shaders = (struct shader_entry_t *) realloc(shaders, sizeof(struct shader_entry_t) * num_shaders);
	shaders[num_shaders-1] = entry;
}

#endif



const char * read_shader(SHADER_TYPE name) {
#if	_DEBUG
	int i;
	for(i=0; i<num_shaders; ++i) {
		if(strcmp(shaders[i].name, name) == 0) {
			return shaders[i].data;
		}
	}
	{
		char * _name = find_path(name);	
		char * data;
		unsigned long size, res;
		FILE * file = fopen(_name, "rb");
		free(_name);

		if(file == NULL) {
			printf("Couldn't open file `%s'\n", name);
			terminate();
		}

		{
			const long cur = ftell(file);
			fseek (file , 0 , SEEK_END);
			size = ftell(file);
			fseek(file, cur, SEEK_SET);
		}

		data = (char*) malloc(size+1);
		data[size] = 0;
		res = fread(data, 1, size, file);
		if ( res != size ) {
			if ( ferror(file) ){
				printf("Error when reading file `%s': %s\n", name, strerror(errno));
			} else {
				printf("Error when reading file `%s': read size was not the expected size (read %lu bytes, expected %lu)\n", name, res, size);
			}
		}
		fclose(file);
		add_file(name, data);
		return data;
	}
	MessageBox(NULL, "File failure", name, MB_OK | MB_ICONEXCLAMATION);
	terminate(); /* Failed to open file */
#else
	return _shaders[name];
#endif
}