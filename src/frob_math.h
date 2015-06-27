#pragma once
#include "demo.h"

// TODO: Try out simd, see if it reduces the filesize
#ifdef ENABLE_MATH

#include <math.h>

typedef struct {
	float x;
	float y;
} vec2;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} vec4;

typedef struct {
	vec4 c0;
	vec4 c1;
	vec4 c2;
	vec4 c3;
} mat4;

float dot(const vec4 * v1, const vec4 * v2);

float normal(const vec4 * v);
void normalize(vec4 * v);

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
void complex_mul(const complex* c1, const complex* c2, complex* out);

__forceinline void complex_conj(complex* c) { c->y *= -1.f; }

#endif
