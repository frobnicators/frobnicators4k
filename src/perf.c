#include "perf.h"
#include "winclude.h"
#include "debug.h"
#include "nvToolsExt.h"

#if SOME_DEBUG

static struct PerfScope* g_firstScope = NULL;

unsigned long util_utime(){
	static int initialized = 0;
	static int dst_warning = 0;
	static long long int divider = 0;
	static LARGE_INTEGER qpc_freq;

	/* verify timer precision */
	if (!initialized){
		QueryPerformanceFrequency(&qpc_freq);
		/* set divider to calculate µs */
		divider = qpc_freq.QuadPart / 1000000;
		initialized = 1;
	}

	/* time query */
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	return (unsigned long)(tmp.QuadPart / divider);
}

void begin_scope(struct PerfScope* scope) {
	scope->current_start = util_utime();
	if (!scope->registered) {
		scope->next = g_firstScope;
		g_firstScope = scope;
		scope->registered = 1;
	}
}

void end_scope(struct PerfScope* scope) {
	unsigned long time = util_utime() - scope->current_start;
	++scope->hits;
	scope->accum += time;
	if (time > scope->max_val) {
		scope->max_val = time;
	}
}

void print_perf_report() {
	debug_print("====== Performance Report =======\n");
	struct PerfScope* next = g_firstScope;
	while (next != NULL) {
		float accum = next->accum / 1000.f;
		debug_print("%s(%d): %s: %f ms (%u hits, max: %f ms)\n", next->filename, next->line, next->name, accum / next->hits, next->hits, next->max_val / 1000.f);
		next = next->next;
	}
	debug_print("=================================\n");
}

void gpu_scope_push(const char* name, unsigned int color) {
    nvtxEventAttributes_t eventAttrib = {0};

    // set the version and the size information
    eventAttrib.version = NVTX_VERSION;
    eventAttrib.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;

    // configure the attributes.  0 is the default for all attributes.
    eventAttrib.colorType = NVTX_COLOR_ARGB;
    eventAttrib.color = color;
    eventAttrib.messageType = NVTX_MESSAGE_TYPE_ASCII;
    eventAttrib.message.ascii = name;

	nvtxRangePushEx(&eventAttrib);
}

void gpu_scope_pop() {
	nvtxRangePop();
}

#endif