#version 150

#extension GL_ARB_explicit_attrib_location: enable

uniform float t; /* time */
uniform float b; /* Base sync */
const int max_steps = 500;
const float max_dist = 1000;
const vec3 bgcolor = vec3(0);

const vec3 light_color = vec3(0.8);
const vec3 ambient_light = vec3(0.2);

const float epsilon = 0.001;

float _db(vec3 eye, vec3 pos, vec3 extends) {
	vec3 delta = abs( pos - eye ) - extends;
	return max( delta.x, max (delta.y, delta.z) );
}

float df(vec3 p, out vec3 color);
vec3 lp(); /* Shall return light position */

vec3 n(vec3 p){ /* Calculate normal*/
	vec3 n;
	vec3 color;
	n.x = df(p + vec3(epsilon,0,0), color) - df(p - vec3(epsilon,0,0), color);
	n.y = df(p + vec3(0,epsilon,0), color) - df(p - vec3(0, epsilon,0), color);
	n.z = df(p + vec3(0,0,epsilon), color) - df(p - vec3(0,0,  epsilon), color);
	return normalize(n);
}

vec3 l(vec3 pos, vec3 color) { /* calculate light */
	vec3 light_dir = pos - lp();
	float dist = length(light_dir);
	light_dir = normalize(light_dir);

	vec3 normal = n(pos);
	float lambert_term = max(dot(-light_dir, normal), 0.0);
	vec3 ocolor = color * lambert_term;
	return ocolor + color*ambient_light;
}

vec3 raymarch(vec3 p, vec2 uv){
	vec3 dir = vec3(0,0,0);
	dir += vec3(1,0,0) * uv.x;
	dir += vec3(0,1,0) * uv.y * 0.5625 /* aspect ratio */;
	dir += -vec3(0,0,1) * 1;
	dir = normalize(dir);

	vec3 color;
	float dist = 0.0;

	for (int i = 0; i < max_steps && dist < max_dist; i++ ) {
		float d = df(p, color);
		if ( d < epsilon ) return l(p, color);
		dist += abs(d);
		p += dir * d;
	}
	return bgcolor;
}