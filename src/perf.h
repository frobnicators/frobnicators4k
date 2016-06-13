#pragma once

#if SOME_DEBUG

struct PerfScope {
	const char* name;
	const char* filename;
	int line;
	unsigned long accum;
	unsigned long max_val;
	unsigned long current_start;
	unsigned int hits;
	int registered;
	struct PerfScope* next;
};

void begin_scope(struct PerfScope* scope);
void end_scope(struct PerfScope* scope);
void print_perf_report();

void gpu_scope_push(const char* name, unsigned int color);
void gpu_scope_pop();

#define FROB_PERF_BEGIN(name) \
	static struct PerfScope _perf_scope_##name = { #name, __FILE__, __LINE__, 0, 0, 0, 0, 0,NULL }; \
	begin_scope(&_perf_scope_##name)

#define FROB_PERF_END(name) end_scope(&_perf_scope_##name);

#define GPU_SCOPE_PUSH(name, color) gpu_scope_push(name, color)
#define GPU_SCOPE_POP() gpu_scope_pop()

#define COLOR_RED 0xFFFF0000
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE 0xFF0000FF
#define COLOR_YELLOW 0xFFFFFF00
#define COLOR_MAGENTA 0xFFFF00FF
#define COLOR_CYAN 0xFF00FFF

#else

#define FROB_PERF_BEGIN(name) {}
#define FROB_PERF_END(name) {}
#define GPU_SCOPE_PUSH(name) {}
#define GPU_SCOPE_POP() {}

#endif
