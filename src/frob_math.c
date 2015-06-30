#include "demo.h"
#include "debug.h"
#ifdef ENABLE_MATH

#include "frob_math.h"

float dot(const vec4 * v1, const vec4 * v2) {
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
}

float dotv2(const vec2 * v1, const vec2 * v2) {
	return v1->x * v2->x + v1->y * v2->y;
}

float dotv3(const vec3 * v1, const vec3 * v2) {
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

float normal(const vec4 * v) {
	return (float)sqrt(dot(v, v));
}

void normalize(vec4 * v) {
	float n= normal(v);
	v->x /= n;
	v->y /= n;
	v->z /= n;
	v->w /= n;
}

float normal_v2(const vec2 * v) {
	return (float)sqrt(dotv2(v, v));
}

void normalize_v2(vec2 * v) {
	float n = normal_v2(v);
	v->x /= n;
	v->y /= n;
}

float normal_v3(const vec3 * v) {
	return (float)sqrt(dotv3(v, v));
}

void normalize_v3(vec3 * v) {
	float n= normal_v3(v);
		
	v->x /= n;
	v->y /= n;
	v->z /= n;
}


vec4 mulvv(const vec4 * v1, const vec4 * v2) {
	vec4 ret = {
		v1->x * v2->x,
		v1->y * v2->y,
		v1->z * v2->z,
		v1->w * v2->w
	};
	return ret;
}

vec4 mulvs(const vec4 * v, const float s) {
	vec4 ret = {
		v->x * s,
		v->y * s,
		v->z * s,
		v->w * s 
	};
	return ret;
}

vec4 addvv(const vec4 * v1, const vec4 * v2) {
	vec4 ret = {
		v1->x + v2->x,
		v1->y + v2->y,
		v1->z + v2->z,
		v1->w + v2->w
	};
	return ret;
}

vec4 subvv(const vec4 * v1, const vec4 * v2) {
	vec4 ret = {
		v1->x - v2->x,
		v1->y - v2->y,
		v1->z - v2->z,
		v1->w - v2->w
	};
	return ret;
}

vec4 addvs(const vec4 * v, const float s) {
	vec4 ret = {
		v->x + s,
		v->y + s,
		v->z + s,
		v->w + s 
	};
	return ret;
}

mat4 mulmm(const mat4* m1, const mat4* m2)
{
	mat4 ret;
	int i,j,k;
	const float* fm1 = (float*)m1;
	const float* fm2 = (float*)m2;
	float* fret = (float*)&ret;
	for (j=0; j<4; ++j) {
		for (i=0; i<4; ++i) {
			for(k=0; k<4; ++k) {
				fret[i*4 + j] += fm1[i * 4 + k] * fm2[k * 4 + j];
			}
		}
	}
	return ret;
}

void addvv2(const vec2 * v1, const vec2 * v2, vec2* out) {
	out->x = v1->x + v2->x;
	out->y = v1->y + v2->y;
}

void subvv2(const vec2 * v1, const vec2 * v2, vec2* out) {
	out->x = v1->x - v2->x;
	out->y = v1->y - v2->y;
}

void mulvv2(const vec2 * v1, const vec2 * v2, vec2* out) {
	out->x = v1->x * v2->x;
	out->y = v1->y * v2->y;
}

void complex_mul(const complex* c1, const complex* c2, complex* out) {
	out->x = (c1->x * c2->x) - (c1->y * c2->y);
	out->y = (c1->y * c2->x) + (c1->x * c2->y);
}

#endif