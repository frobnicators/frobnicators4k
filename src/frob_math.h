#pragma once
#include "demo.h"

// TODO: Try out simd, see if it reduces the filesize
#ifdef ENABLE_MATH

#include <math.h>

typedef __declspec(align(8)) struct {
	float x;
	float y;
} vec2;

typedef __declspec(align(16)) struct {
	float x;
	float y;
	float z;
} vec3;

typedef __declspec(align(16)) struct {
	float x;
	float y;
	float z;
	float w;
} vec4;

typedef __declspec(align(16)) struct {
	vec4 c0;
	vec4 c1;
	vec4 c2;
	vec4 c3;
} mat4;

float dot(const vec4 * v1, const vec4 * v2);
float dotv2(const vec2 * v1, const vec2 * v2);
float dotv3(const vec3 * v1, const vec3 * v2);

float normal(const vec4 * v);
void normalize(vec4 * v);

float normal_v2(const vec2 * v);
void normalize_v2(vec2 * v);

float normal_v3(const vec3 * v);
void normalize_v3(vec3 * v);

vec3 cross(const vec3* v1, const vec3* v2);

vec4 mulvv(const vec4 * v1, const vec4 * v2);
vec4 mulvs(const vec4 * v, const float s);
vec4 addvv(const vec4 * v1, const vec4 * v2);
vec4 subvv(const vec4 * v1, const vec4 * v2);
vec4 addvs(const vec4 * v, const float s);
mat4 mulmm(const mat4* m1, const mat4* m2);

void addvv2(const vec2 * v1, const vec2 * v2, vec2* out);
void subvv2(const vec2 * v1, const vec2 * v2, vec2* out);
void mulvv2(const vec2 * v1, const vec2 * v2, vec2* out);

typedef vec2 complex;

__forceinline void complex_add(const complex* c1, const complex* c2, complex* out) { addvv2(c1, c2, out); }
__forceinline void complex_sub(const complex* c1, const complex* c2, complex* out) { subvv2(c1, c2, out); }

// Note that out must not be c1 or c2
void complex_mul(const complex* c1, const complex* c2, complex* out);

__forceinline void complex_conj(complex* c) { c->y *= -1.f; }

// TODO: Maybe remove a few decimals?
#define M_E 2.71828182845904523536
#define M_LOG2E 1.44269504088896340736
#define M_LOG10E 0.434294481903251827651
#define M_LN2 0.693147180559945309417
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846f
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.785398163397448309616
#define M_1_PI 0.318309886183790671538
#define M_2_PI 0.636619772367581343076
#define M_1_SQRTPI 0.564189583547756286948
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2 1.41421356237309504880
#define M_SQRT_2 0.707106781186547524401

#endif
