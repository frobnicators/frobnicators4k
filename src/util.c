#include "util.h"
#include "winclude.h"

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