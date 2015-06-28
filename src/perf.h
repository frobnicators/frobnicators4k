#pragma once

#if SOME_DEBUG

struct PerfScope {
	const char* name;
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

#define FROB_PERF_BEGIN(name) \
	static struct PerfScope _perf_scope_##name = { #name, 0, 0, 0, 0, 0,NULL }; \
	begin_scope(&_perf_scope_##name)

#define FROB_PERF_END(name) end_scope(&_perf_scope_##name);

#else

#define FROB_PERF_BEGIN(name) {}
#define FROB_PERF_END(name) {}

#endif
