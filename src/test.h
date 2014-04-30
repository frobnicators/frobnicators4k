#include "unit_test_c/includes/unit_test.h"

#include "math.h"

void run_tests() {
	begin_test_suite();

	begin_context("vector math");

	begin_test("dot");
	{
		vec4 v1 = { 1.f, 2.f, 3.f, 4.f };
		vec4 v2 = { 5.f, 6.f, 7.f, 8.f };
		vec4 v3 = { 0.f, };
		assert_floats_equal(dot(&v1, &v2), 70.f);
		assert_floats_equal(dot(&v1, &v3), 0.f);
	}
	begin_test("normal");
	{
		vec4 v1 = { 1.f, 2.f, 3.f, 4.f };
		vec4 v2 = { 1.f, 0.f, 0.f, 0.f };
		assert_floats_equal(normal(&v1), sqrt(30.f));
		assert_floats_equal(normal(&v2), 1.f);
	}
	begin_test("normalize");
	{
		vec4 v1 = { 1.f, 2.f, 3.f, 4.f };
		vec4 v2 = { 1.f, 0.f, 1.f, 0.f };
		normalize(&v1);
		assert_floats_equal(normal(&v1), 1.f);
		normalize(&v2);
		assert_floats_equal(v2.x, 1.f/sqrt(2.f));
		assert_floats_equal(v2.y, 0.f);
		assert_floats_equal(v2.z, 1.f/sqrt(2.f));
		assert_floats_equal(v2.w, 0.f);
	}
	end_test();

	end_context();


	end_test_suite();

	getchar();
}