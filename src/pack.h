#ifndef PACK_H
#define PACK_H

#define GLSL_VERSION_LINE "#version 150\n"

struct file_data_t {
		const char * data;
		unsigned long size;
};

struct file_data_t read_data(const char * name);

#endif