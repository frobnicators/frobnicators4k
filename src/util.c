#include "util.h"

#if _DEBUG 
	#include <stdio.h>
	#include <stdarg.h>
#endif

void debug(const char* fmt, ...){
#if _DEBUG
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
#endif
}