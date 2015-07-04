#include "unit_test_c/includes/unit_test.h"

#include "frob_math.h"
#include "shader.h"
#include "klister.h"
#include "edison2015/fft.h"
#include <stdlib.h>

complex row_results_fft[16][16];

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
		assert_floats_equal(normal(&v1), (float)sqrt(30.f));
		assert_floats_equal(normal(&v2), (float)1.f);
	}
	begin_test("normalize");
	{
		vec4 v1 = { 1.f, 2.f, 3.f, 4.f };
		vec4 v2 = { 1.f, 0.f, 1.f, 0.f };
		normalize(&v1);
		assert_floats_equal(normal(&v1), 1.f);
		normalize(&v2);
		assert_floats_equal(v2.x, 1.f/(float)sqrt(2.f));
		assert_floats_equal(v2.y, 0.f);
		assert_floats_equal(v2.z, 1.f/(float)sqrt(2.f));
		assert_floats_equal(v2.w, 0.f);
	}

	begin_test("cross");
	{
		vec3 v1 = { 3.f, -3.f, 1.f };
		vec3 v2 = { 4.f, 9.f, 2.f };
		vec3 r = cross(&v1, &v2);
		assert_floats_equal(r.x, -15.f);
		assert_floats_equal(r.y, -2.f);
		assert_floats_equal(r.z, 39.f);
	}

	end_test();

	end_context();

	begin_context("complex math");

	begin_test("Multiply");
	{
		complex c1 = { 4.f, 13.f };
		complex c2 = { 25.f, 57.f };
		complex result;
		complex_mul(&c1, &c2, &result);
		assert_floats_equal(result.x, -641.f);
		assert_floats_equal(result.y, 553.f);
	}

	end_context();

	begin_context("fft");

	fft_t fft;

	fft_init(&fft, 16);

	{
		complex* data = malloc(sizeof(complex) * 64*64);
		for (int m = 0; m < 16; ++m) {
			for (int n = 0; n < 64; ++n) {
				data[m * 16 + n].x = 0.0001f * m;
				data[m * 16 + n].y = 0.0001f * n - 0.0005f;
			}
		}

		for (int m = 0; m < 16; ++m) {
			fft_execute(&fft, data, data, 1, m * 16);
		}

		begin_test("fft row resolve");
		for (int m = 0; m < 16; ++m) {
			for (int n = 0; n < 16; ++n) {
				assert_floats_equal(data[m * 16 + n].x, row_results_fft[m][n].x);
				assert_floats_equal(data[m * 16 + n].y, row_results_fft[m][n].y);
			}
		}

	}

	end_context();

	begin_context("fft_compute");
	{

		fft_t fft;

		fft_init(&fft, 16);

		{
			complex* data = malloc(sizeof(complex) * 64 * 64);
			for (int m = 0; m < 16; ++m) {
				for (int n = 0; n < 64; ++n) {
					data[m * 16 + n].x = 0.0001f * m;
					data[m * 16 + n].y = 0.0001f * n - 0.0005f;
				}
			}

			for (int m = 0; m < 16; ++m) {
				fft_compute(&fft, data, 1, m * 16);
			}

			begin_test("fft row resolve");
			for (int m = 0; m < 16; ++m) {
				for (int n = 0; n < 16; ++n) {
					assert_floats_equal(data[m * 16 + n].x, row_results_fft[m][n].x);
					assert_floats_equal(data[m * 16 + n].y, row_results_fft[m][n].y);
				}
			}

		}
	}

	end_context();


	end_test_suite();
}

complex row_results_fft[16][16] = {
{
	{0.000000, 0.004000},
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.001600, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.003200, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.004800, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.006400, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.008000, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.009600, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.011200, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.012800, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.014400, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.016000, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.017600, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.019200, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.020800, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.022400, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
},
{
	{ 0.024000, 0.004000 },
	{ 0.004022, -0.000800 },
	{ 0.001931, -0.000800 },
	{ 0.001197, -0.000800 },
	{ 0.000800, -0.000800 },
	{ 0.000535, -0.000800 },
	{ 0.000331, -0.000800 },
	{ 0.000159, -0.000800 },
	{ 0.000000, -0.000800 },
	{ -0.000159, -0.000800 },
	{ -0.000331, -0.000800 },
	{ -0.000535, -0.000800 },
	{ -0.000800, -0.000800 },
	{ -0.001197, -0.000800 },
	{ -0.001931, -0.000800 },
	{ -0.004022, -0.000800 },
}
};

