#ifndef PACK_H
#define PACK_H

struct file_data_t {
		const char * data;
		unsigned long size;
};

struct file_data_t read_data(const char * name);

#endif