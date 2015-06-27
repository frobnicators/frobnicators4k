#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

static void vasprintf(char **strp, const char *fmt, va_list ap) {
	const int len = _vscprintf(fmt, ap) + 1; /* Include null terminator */
	*strp = (char*)malloc(len);
	vsprintf_s(*strp, len, fmt, ap);
}

void debug_print(const char* fmt, ...) {
	char* str = NULL;
	va_list ap;
	va_start(ap, fmt);
	vasprintf(&str, fmt, ap);
	va_end(ap);
	OutputDebugString(str);
	free(str);
}

void error_msg(const char* title, const char* fmt, ...) {
	char* str = NULL;
	va_list ap;
	va_start(ap, fmt);
	vasprintf(&str, fmt, ap);
	va_end(ap);

	OutputDebugString(str);

	MessageBox(NULL, str, title, MB_OK | MB_ICONEXCLAMATION);
	free(str);
}