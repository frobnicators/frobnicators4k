#version 150

#extension GL_ARB_explicit_attrib_location: enable

uniform float t; /* time */
uniform float b; /* Base sync */

const int i_max_steps = 500;
const int i_max_dist = 1000;
const vec3 i_bgcolor = vec3(0);

const vec3 i_light_color = vec3(0.8);
const vec3 i_ambient_light = vec3(0.2);

const float i_epsilon = 0.001;

#pragma export(_df,_lp)
float _df(vec3 p, out vec3 c);
vec3 _lp(); /* Shall return light position */

float _db(vec3 eye, vec3 pos, vec3 extends) {
	vec3 delta = abs( pos - eye ) - extends;
	return max( delta.x, max (delta.y, delta.z) );
}

vec3 _n(vec3 p){ /* Calculate normal*/
	vec3 n;
	vec3 color;
	n.x = _df(p + vec3(i_epsilon,0,0), color) - _df(p - vec3(i_epsilon,0,0), color);
	n.y = _df(p + vec3(0,i_epsilon,0), color) - _df(p - vec3(0, i_epsilon,0), color);
	n.z = _df(p + vec3(0,0,i_epsilon), color) - _df(p - vec3(0,0,  i_epsilon), color);
	return normalize(n);
}

vec3 _l(vec3 pos, vec3 color) { /* calculate light */
	vec3 light_dir = pos - _lp();
	float dist = length(light_dir);
	light_dir = normalize(light_dir);

	vec3 normal = _n(pos);
	float lambert_term = max(dot(-light_dir, normal), 0.0);
	vec3 ocolor = color * lambert_term * i_light_color;
	return ocolor + color*i_ambient_light;
}

vec3 raymarch(vec3 p, vec2 uv){
	vec3 dir = vec3(0,0,0);
	dir += vec3(1,0,0) * uv.x;
	dir += vec3(0,1,0) * uv.y * 0.5625 /* aspect ratio */;
	dir += -vec3(0,0,1) * 1;
	dir = normalize(dir);

	vec3 color;
	float dist = 0.0;

	for (int i = 0; i < i_max_steps && dist < i_max_dist; i++ ) {
		float d = _df(p, color);
		if ( d < i_epsilon ) return _l(p, color);
		dist += abs(d);
		p += dir * d;
	}
	return i_bgcolor;
}