#ifndef UTIL_H
#define UTIL_H
	#include <crtdefs.h>

	void update_time(unsigned long * ldt);

	void start_time();

	void * memset(void * s, int c, size_t n);
	int strcmp(const char * s1, const char * s2);
#if _DEBUG
	void checkForGLErrors(const char * msg);
#endif
#endif