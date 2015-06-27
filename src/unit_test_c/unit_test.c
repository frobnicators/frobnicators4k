#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "includes/unit_test.h"

#ifndef NO_COLOR
#include "sh_colors.h"
#endif

#ifdef WIN32
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <windows.h>
#endif

static int success_tests = 0;
static int total_tests = 0;

static int context_tests = 0;
static int context_success = 0;

static int active_test_suite = 0;

static char current_context[64];
static char current_test[128];

static int test_status = -1;

#ifdef WIN32
static void vasprintf(char **strp, const char *fmt, va_list ap) {
	const int len = _vscprintf(fmt, ap) + 1; /* Include null terminator */
	*strp = (char*)malloc(len);
	vsprintf_s(*strp, len, fmt, ap);
}

void unit_test_printf(const char* fmt, ...) {
	char* str = NULL;
	va_list ap;
	va_start(ap, fmt);
	vasprintf(&str, fmt, ap);
	va_end(ap);
	OutputDebugStringA(str);
	free(str);
}

#else

void unit_test_printf(const char* fmt, ...) {
	va_start(ap, fmt);
	vaprintf(fmt, ap);
	va_end(ap);
}

#endif

static void assert_active_test_suite() {
	if(active_test_suite != 1) {
		fprintf(stderr, "No active test suite! Call begin_test_suite() first!\n");
		assert(0);
	}
}	

static void assert_active_test() {
	if(test_status == -1) {
		fprintf(stderr, "Assert called, but no test was active\n");
		assert(0);
	}
}	

void begin_test_suite() {
	if(active_test_suite == 1) {
		fprintf(stderr, "Test suite already active!\n");
		assert(0);
	} else {
		unit_test_printf("============= BEGIN TEST SUITE ===============\n\n");
	}
	success_tests = 0;
	total_tests = 0;
	active_test_suite = 1;
}

int end_test_suite() {
	const char * status;
	assert_active_test_suite();
	active_test_suite = 0;
	unit_test_printf("============= END TEST SUITE ===============\n");
	if(success_tests == total_tests) {
		status = "succeeded";
#ifndef NO_COLOR
		color(SH_FG_GREEN);
#endif
	} else {
		status = "failed";
#ifndef NO_COLOR
		color(SH_FG_RED);
#endif
	}
	unit_test_printf("Test suite %s. Successive tests: %d/%d\n", status, success_tests, total_tests);
#ifndef NO_COLOR
	reset_color();
#endif
	return !(success_tests == total_tests);
}

void begin_context(const char * context) {
	assert_active_test_suite();
	context_tests = 0;
	context_success = 0;
	test_status = -1;
	strcpy(current_context, context);
	unit_test_printf("==== %s ====\n", context);
}

int end_context() {
	end_test();

	unit_test_printf("==============================\n");
	unit_test_printf("Result for %s: %d/%d successfull tests\n\n", current_context, context_success, context_tests);
	return !(context_success == context_tests);
}

void begin_test(const char * test_name) {
	assert_active_test_suite();

	end_test();

	test_status = 0;

	strcpy(current_test, test_name);
}

int end_test() {
	if(test_status == -1) {
		return 0;
	} else {
		++context_tests;
		++total_tests;
		if(test_status == 0) { //Previous test succeeded
			++context_success;
			++success_tests;
			test_status = -1; //Inactivate
#ifndef NO_COLOR
			color(SH_FG_GREEN);
#endif
			unit_test_printf("Test %s: OK\n", current_test);
#ifndef NO_COLOR
			reset_color();
#endif
			return 0;
		} else {
			test_status = -1;
			return 1;
		}
	}
}

/***
 * Assertions
 */

static void core_assert(int expr, const char * error_msg) {
	assert_active_test();
	if(test_status == 0) {
		if(expr == 0) {
#ifndef NO_COLOR
			color(SH_FG_RED);
#endif
			unit_test_printf("Test %s: FAIL. Expected %s\n", current_test, error_msg);
			test_status=1;
#ifndef NO_COLOR
			reset_color();
#endif
		}
	} 
}

void assert_custom(int expr, const char * msg) {
	core_assert(expr, msg);
}

void assert_true(int expr) {
	core_assert(expr, "expression to be true");
}

void assert_false(int expr) {
	core_assert((expr==0), "expression to be false");
}

void assert_gt(int v1,int  v2) {
	char msg[128];
	sprintf(msg, "%d to be greater than %d", v1, v2);
	core_assert((v1>v2), msg);
}

void assert_lt(int v1, int v2) {
	char msg[128];
	sprintf(msg, "%d to be less than %d", v1, v2); 
	core_assert((v1<v2), msg);
}

void assert_strings_equal(const char * str1, const char * str2) {
	char msg[256];
	if(strlen(str1)+strlen(str2) < 240) {
		sprintf(msg, "\"%s\" to be equal to \"%s\"", str1, str2); 
	} else {
		sprintf(msg, "strings to be equal");
	}
	core_assert((strcmp(str1, str2)==0), msg);
}

void assert_strings_equal_n(const char * str1, const char * str2, int n) {
	char msg[256];
	if(2*n < 240) {
		sprintf(msg, "\"%.*s\" to be equal to \"%.*s\"", n,str1,n, str2); 
	} else {
		sprintf(msg, "strings to be equal");
	}
	core_assert((strncmp(str1, str2, n)==0), msg);
}

void assert_ints_equal(int i1, int i2) {
	char msg[128];
	sprintf(msg, "%d to be equal to %d", i1, i2); 
	core_assert((i1==i2), msg);
}

void assert_floats_equal(float f1, float f2) {
	char msg[128];
	sprintf(msg, "%f to be equal to %f", f1, f2); 
	core_assert(fabs(f1-f2) < 0.001f, msg);
}