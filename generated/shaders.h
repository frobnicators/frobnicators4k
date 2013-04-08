#ifndef SHADERS_H
#define SHADERS_H

struct shader_entry_t {
	const char * name;
	const char * data;
};

#define NUM_SHADERS 3
extern struct shader_entry_t _shaders[NUM_SHADERS];
#endif
