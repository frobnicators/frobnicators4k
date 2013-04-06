#ifndef UTIL_H
#define UTIL_H
	#include <crtdefs.h>

	void debug(const char* fmt, ...);

	float get_time(float * dt);

	void start_time();

	void * frob_memset(void * s, int c, size_t n);
	int frob_strcmp(const char * s1, const char * s2);
#endif