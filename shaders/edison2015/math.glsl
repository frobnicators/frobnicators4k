#version 430

// TODO: Reduce precision to save bytes
#define M_PI 3.1415926535897932384626433832795

// Complex math:

// multiply
vec2 cmul(vec2 c1, vec2 c2) {
	return vec2(c1.x*c2.x - c1.y * c2.y, c1.y * c2.x + c1.x * c2.y);
}

// conjugate
vec2 conj(vec2 c) {
	return vec2(c.x, -1.f * c.y);
}
