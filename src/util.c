#include "util.h"
#include "winclude.h"

#if _DEBUG 
	#include <stdio.h>
	#include <stdarg.h>
#endif

unsigned long _start_time, _last_time;

void debug(const char* fmt, ...){
#if _DEBUG
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
#endif
}

static unsigned long util_utime(){
	static int initialized = 0;
	static int dst_warning = 0;
	static long long int divider = 0;
	static LARGE_INTEGER qpc_freq, tmp;

	/* verify timer precision */
	if ( !initialized ){
		QueryPerformanceFrequency(&qpc_freq);
		/* set divider to calculate µs */
		divider = qpc_freq.QuadPart / 1000000;
		initialized = 1;
	}

	/* time query */
	QueryPerformanceCounter(&tmp);
	return (unsigned long)(tmp.QuadPart / divider);
}

void start_time() {
	_start_time = util_utime();
	_last_time = _start_time;
}

float get_time(float * dt) {
	unsigned long t = util_utime();
	*dt = (float)(t - _last_time) / 1000000.f;
	_last_time = t;
	return (t - _start_time) / 1000000.f;
}