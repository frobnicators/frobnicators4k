#ifndef SHADERS_H
#define SHADERS_H

struct shader_entry_t {
	const char * name;
	const char * data;
};

#define NUM_SHADERS 4
extern const struct shader_entry_t const _shaders[NUM_SHADERS];
#endif
