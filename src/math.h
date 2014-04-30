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

#endif
