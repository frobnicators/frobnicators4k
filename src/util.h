#ifndef UTIL_H
#define UTIL_H
	#include <crtdefs.h>

	void debug(const char* fmt, ...);

	float get_time(float * dt);

	void start_time();

	void * memset(void * s, int c, size_t n);
	int strcmp(const char * s1, const char * s2);
#endif