#include "util.h"
#include "main.h"
#include "winclude.h"
#include "music.h"
#include "debug.h"


#if _DEBUG || SOME_DEBUG
	#include <stdio.h>
	#include <stdarg.h>
	#include <GL/GLU.h>
#endif

#if _DEBUG
int strcmp(const char * s1, const char * s2) {
	size_t i=0;
	while(s1[i] != 0 && s2[i] != 0) {
		if(s1[i] < s2[i]) return -1;
		else if(s1[i] > s2[i]) return 1;
		i++;
	}
	return (s1[i] - s2[i]);
}
#endif

#if _DEBUG || SOME_DEBUG
void checkForGLErrors(const char *s) {
	int errors = 0 ;

	while ( 1 ) {
		GLenum x = glGetError() ;

		if (x == GL_NO_ERROR)
			return;

		FROB_PRINTF("%s: OpenGL error: %s\n", s, gluErrorString(x));
		errors++;
	}
	if (errors > 0) {
		FROB_ERROR("OpenGL", "%d OpenGL Errors in context %s.", errors, s);
	}

}
#endif
