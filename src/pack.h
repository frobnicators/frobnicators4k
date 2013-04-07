#ifndef PACK_H
#define PACK_H

struct file_data_t {
		const char * data;
		unsigned long size;
};

void read_data(const char * name, struct file_data_t * data );

#endif