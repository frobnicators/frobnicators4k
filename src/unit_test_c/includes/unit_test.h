/*
 * This is a simple framework for performing unit test
 *
 * Author: Andreas Tarandi
 */

#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#define NO_COLOR

/**
 * Start test suite. Must be called before anything else
 */
void begin_test_suite();
/**
 * End test suite, must be called last
 */
int end_test_suite();

//Note: Contexts doesn't stack!
void begin_context(const char * context);
int end_context();

void begin_test(const char * test_name);
int end_test();

/*****
 * asserts
 *****/

void assert_true(int expr);
void assert_false(int expr);
void assert_gt(int v1, int v2); //Assert v1 > v2
void assert_lt(int v1, int v2); //Assert v1 < v2
void assert_strings_equal(const char * str1, const char * str2); //Assert strings equal
void assert_strings_equal_n(const char * str1, const char * str2, int n); //Assert strings equal up to char n
void assert_ints_equal(int i1, int i2);
void assert_floats_equal(float f1, float f2); // Uses 0.001 epsilon

//Custom assertion, same as assert_true but with custom msg
void assert_custom(int expr, const char * msg);

#endif
