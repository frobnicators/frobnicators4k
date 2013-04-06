#include "util.h"
#include "winclude.h"
#include <string.h>

#if _DEBUG 
	#include <stdio.h>
	#include <stdarg.h>
#endif

DWORD _start_time, _last_time;

void debug(const char* fmt, ...){
#if _DEBUG
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
#endif
}

void start_time() {
	_start_time = GetTickCount();
	_last_time = _start_time;
}

float get_time(float * dt) {
	DWORD t = GetTickCount();
	*dt = (float)(t - _last_time) / 1000000.f;
	_last_time = t;
	return (t - _start_time) / 1000000.f;
}

void * frob_memset(void * s, int c, size_t n) {
	char * b = (char*) s;
	size_t i;
	for(i = 0; i < n; i++, b++) {
		*b = (char) c;
	}
	return s;
}

int frob_strcmp(const char * s1, const char * s2) {
	size_t i, l1, l2;
	l1 = strlen(s1);
	l2 = strlen(s2);
	for(i = 0; i<l1 && i < l2; ++i) {
		if(s1[i] < s2[i]) return -1;
		else if(s1[i] > s2[i]) return 1;
	}
	return 0;
}