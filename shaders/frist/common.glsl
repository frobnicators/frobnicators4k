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

#pragma export(scene,lpos)
float scene(vec3 p, out vec3 c);
vec3 lpos(); /* Shall return light position */

float dbox(vec3 eye, vec3 pos, vec3 extends) {
	vec3 delta = abs( pos - eye ) - extends;
	return max( delta.x, max (delta.y, delta.z) );
}

vec3 n_at(vec3 p){ /* Calculate normal*/
	vec3 n;
	vec3 color;
	n.x = scene(p + vec3(i_epsilon,0,0), color) - scene(p - vec3(i_epsilon,0,0), color);
	n.y = scene(p + vec3(0,i_epsilon,0), color) - scene(p - vec3(0, i_epsilon,0), color);
	n.z = scene(p + vec3(0,0,i_epsilon), color) - scene(p - vec3(0,0,  i_epsilon), color);
	return normalize(n);
}

float brdr(vec3 pos, vec3 dir, vec3 normal, float d) {
	float border = 0;
	vec3 check_dir = cross(dir, normal);
	vec3 color;
	float weight = 1.0;
	for(int i=0; i<5; ++i) {
		float delta = pow( float (i + 1.0 ), 2.0 ) * 0.07;
		border += abs(scene(pos + check_dir * delta, color) - d) * weight;
		border += abs(scene(pos + check_dir * -delta, color) - d) * weight;
		weight *= 0.5;
	}
	return step(0.1,border);
}

vec3 light(vec3 pos, vec3 dir, float d, vec3 color) { /* calculate light */
	vec3 light_dir = pos - lpos();
	float dist = length(light_dir);
	light_dir = normalize(light_dir);

	vec3 normal = n_at(pos);
	float lambert_term = max(dot(-light_dir, normal), 0.0);

	const int i_num_light_steps = 10;
	const float i_step_size = 1.0/i_num_light_steps;

	for(int i=0;i<i_num_light_steps;++i) {
		if(lambert_term < i_step_size * (i+1)) {
			lambert_term = i_step_size*i;
			break;
		}
	}
	vec3 ocolor = color * lambert_term * i_light_color;
	float border = brdr(pos, dir, normal, d);
	return mix(ocolor + color*i_ambient_light, vec3(0), border);
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
		float d = scene(p, color);
		if ( d < i_epsilon ) return light(p, dir, d, color);
		dist += abs(d);
		p += dir * d;
	}
	return i_bgcolor;
}