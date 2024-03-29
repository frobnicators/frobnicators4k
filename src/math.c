#include "demo.h"
#ifdef ENABLE_MATH

#include "math.h"

float dot(const vec4 * v1, const vec4 * v2) {
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
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

#endif