#include "shader.h"
#include "klister.h"
#include "main.h"
#include "util.h"
#include "shaders.h"
#include "demo.h"
#include "debug.h"
#include <stdlib.h>

#include <stdio.h>
#if _DEBUG


#include "demo.h"

#else

extern const char * _shaders[];

#endif

static const char * read_shader(SHADER_TYPE name);

static GLuint vbo[2];
static GLuint default_vao;
static const unsigned char indices[] = { 0, 1, 2 , 3 };

extern const shader_stage_t default_vertex_stage = { GL_VERTEX_SHADER, 2, { SHADER_COMMON_GLSL, SHADER_VERTEX_GLSL } };

void init_gl() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
#if ENABLE_TEXTURES
	glEnable(GL_TEXTURE_2D);
#endif
}

static GLuint build_shader(const shader_stage_t* stage) {
	GLuint shader = glCreateShader(stage->type);
	const char** source_parts = malloc(sizeof(char*)*stage->num_parts);
#if SOME_DEBUG
	GLint compile_status;
#endif
	for (unsigned i = 0; i < stage->num_parts; ++i) {
		source_parts[i] = read_shader(stage->parts[i]);
	}

	glShaderSource(shader, stage->num_parts, source_parts, NULL);
	glCompileShader(shader);
#if SOME_DEBUG
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if(compile_status == GL_FALSE) {
		char buffer[2048];

		glGetShaderInfoLog(shader, 2048, NULL, buffer);
		int line=0;
		for (unsigned int i = 0; i < stage->num_parts; ++i) {
#if _DEBUG
			FROB_PRINTF("<< From %s >>\n", stage->parts[stage->num_parts - 1]);
#else
			FROB_PRINTF("<< From shader %d >>", i);
#endif
			char * src = _strdup(source_parts[i]);
			char * split;
			split =  strtok(src, "\n");
			while(split != NULL) {
				++line;
				FROB_PRINTF("%d:\t%s\n", line, split);
				split = strtok(NULL, "\n");
			}
		}
		FROB_ERROR("Shader build error", "Failed to build shader\n%s", buffer);
#if _DEBUG
		terminate();
#endif
	}
#endif

	free((void*)source_parts);
	return shader;
}

void init_shaders() {
	static const float quad[] = {
		-1.f, -1.f,
		-1.f, 1.f,
		1.f, 1.f,
		1.f, -1.f
	};

	/* Create vbos & vao */
	glGenBuffers(2, vbo);
	glGenVertexArrays(1, &default_vao);

	glBindVertexArray(default_vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

/* Loads the shader. This leaves the created shader active */
void load_shader(shader_t* shader, unsigned int num_stages, ...) {
#if SOME_DEBUG
	GLint link_status;
#endif
	va_list ap;
	va_start(ap, num_stages);

	shader->program = glCreateProgram();

	for (unsigned int i = 0; i < num_stages; ++i) {
		shader_stage_t* stage = va_arg(ap, void*);
		glAttachShader(shader->program, build_shader(stage));
	}
	va_end(ap);
	glLinkProgram(shader->program);

#if SOME_DEBUG
	glGetProgramiv(shader->program, GL_LINK_STATUS, &link_status);
	if(!link_status) {
		char buffer[2048];
		glGetProgramInfoLog(shader->program, 2048, NULL, buffer);
		FROB_ERROR("Shader error", "Link error(s): %s\n", buffer);
	}
#endif

	glUseProgram(shader->program);
	shader->time = glGetUniformLocation(shader->program, "time");
	shader->light_dir = glGetUniformLocation(shader->program, "lightd");
	shader->light_color = glGetUniformLocation(shader->program, "lightc");

	CHECK_FOR_GL_ERRORS("load_shader");
}

void render(shader_t * shader) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
	glBindVertexArray(default_vao);
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
		FROB_ERROR("File error", "Could not find file: %s", name);
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
			FROB_PRINTF("Couldn't open file `%s'\n", name);
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
				FROB_PRINTF("Error when reading file `%s': %s\n", name, strerror(errno));
			} else {
				FROB_PRINTF("Error when reading file `%s': read size was not the expected size (read %lu bytes, expected %lu)\n", name, res, size);
			}
		}
		fclose(file);
		add_file(name, data);
		return data;
	}
	FROB_ERROR("File error", "Failed to open file %s", name);
	terminate(); /* Failed to open file */
#else
	return _shaders[name];
#endif
}