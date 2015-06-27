#include "util.h"
#include "main.h"
#include "winclude.h"
#include "music.h"


#if _DEBUG 
	#include <stdio.h>
	#include <stdarg.h>
	#include <GL/GLU.h>
#endif

void * memset(void * s, int c, size_t n) {
	char * b = (char*) s;
	size_t i;
	for(i = 0; i < n; i++, b++) {
		*b = (char) c;
	}
	return s;
}

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

void checkForGLErrors(const char * str) {
	while ( 1 ) {
		GLenum x = glGetError() ;

		if ( x == GL_NO_ERROR )
			return;

		MessageBox(NULL, (LPCSTR) gluErrorString(x), str, MB_OK);
	}
}
#endif