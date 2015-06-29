#ifndef UTIL_H
#define UTIL_H
	#include <crtdefs.h>

	void start_time();

	//void * memset(void * s, int c, size_t n);
#if _DEBUG
	int strcmp(const char * s1, const char * s2);
#endif
#if SOME_DEBUG
	#define CHECK_FOR_GL_ERRORS(context) checkForGLErrors(context)
	void checkForGLErrors(const char * msg);
#else
	#define CHECK_FOR_GL_ERRORS {}
#endif
#endif