#ifndef DEBUG_H
#define DEBUG_H

#ifdef SOME_DEBUG
#include "winclude.h"

void debug_print(const char* fmt, ...);
void error_msg(const char* title, const char* fmt, ...);

#define FROB_PRINTF(format, ...) debug_print("%s(%d): " format,  __FILE__, __LINE__, __VA_ARGS__)
#define FROB_ERROR(title, format,...) error_msg(title, "%s(%d): " format, __FILE__, __LINE__, __VA_ARGS__)

#else

#define FROB_PRINTF(format, ...) {}
#define FROB_ERROR(title, format,...) {}

#endif

#endif