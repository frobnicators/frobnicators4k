#include "util.h"
#include "main.h"
#include "winclude.h"
#include "music.h"

#if _DEBUG 
	#include <stdio.h>
	#include <stdarg.h>
#endif

#define SAMPLE_RATE_FLT (float)SAMPLE_RATE

static DWORD _last_time = 0;

void update_time(unsigned long * ldt) {
	const DWORD * t = music_time();
	*ldt = (*t - _last_time);
	dt = (float) *ldt / SAMPLE_RATE_FLT;
	_last_time = *t;
	time = *t / SAMPLE_RATE_FLT;
}

void * memset(void * s, int c, size_t n) {
	char * b = (char*) s;
	size_t i;
	for(i = 0; i < n; i++, b++) {
		*b = (char) c;
	}
	return s;
}

int strcmp(const char * s1, const char * s2) {
	size_t i=0;
	while(s1[i] != 0 && s2[i] != 0) {
		if(s1[i] < s2[i]) return -1;
		else if(s1[i] > s2[i]) return 1;
		i++;
	}
	return (s1[i] - s2[i]);
}
