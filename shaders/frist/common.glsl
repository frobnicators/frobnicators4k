#version 150

#extension GL_ARB_explicit_attrib_location: enable

uniform float t; /* time */
uniform float s; /* sync */

const vec3 i_light_color = vec3(0.8);
const vec3 i_ambient_light = vec3(0.2);
const float i_max_dist = 100.f;

const float i_epsilon = 0.001;

struct Ray {
	vec3 po; /* pos */
	vec3 d;	/* dir */
};

#pragma export(scene,slight, bg)
/* Returns vec4( r, g, b, distance ) */
vec4 scene(vec3 p);
/* 
calculate light for scene
r: original ray
c: collision point (including color)
*/
vec3 slight(Ray r, vec4 c);
vec3 bg(Ray r, vec4 c);

float dbox(vec3 eye, vec3 pos, vec3 extends) {
	vec3 delta = abs( pos - eye ) - extends;
	return max( delta.x, max (delta.y, delta.z) );
}

vec3 rotateY ( vec3 p, float a ) {
	float sinA, cosA;
	sinA = sin(a);
	cosA = cos(a);

	return vec3(p.x *cosA +p.z *sinA, p.y, p.z *cosA -p.x *sinA);
}

vec3 permute(vec3 x) {
  return mod(((x*34.0)+1.0)*x, 289);
}

float snoise(vec2 v) {
  const vec4 C = vec4(0.211, 0.366, -0.577, 0.024);
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = pow(m,vec3(4));

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79 - 0.85 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}


float fbm( in vec2 p, int ioct ) {
	const float i_w = 0.3;
	const float i_s = 2.0;
	float n = 0.;
    for( float i=0; i < ioct ; i++ ) {
        n += pow(i_w, i) * snoise(pow(i_s, i) * p);
    }
	return n;
}

vec4 cast_ray(inout Ray ray, float max_dist) {
	vec3 start = ray.po;
	float traveled_distance = 0;
	for (int i = 0; traveled_distance < max_dist; i++ ) {
		vec4 dist = scene(ray.po);
		if ( dist.w < i_epsilon ) {
			ray.po += ray.d * clamp(dist.w, -10.0, 0); /* go back to the impact point */
			return dist;
		}
		dist.w = max(i_epsilon * traveled_distance, dist.w);
		ray.po += ray.d * dist.w;
		traveled_distance = length(start - ray.po);
	}
	return vec4(-1);
}

vec3 n_at(vec3 p){ /* Calculate normal*/
	vec3 n;
	n.x = scene(p + vec3(i_epsilon,0,0)).w - scene(p - vec3(i_epsilon,0,0)).w;
	n.y = scene(p + vec3(0,i_epsilon,0)).w - scene(p - vec3(0, i_epsilon,0)).w;
	n.z = scene(p + vec3(0,0,i_epsilon)).w - scene(p - vec3(0,0,  i_epsilon)).w;
	return normalize(n);
}


float shadow(vec3 p, vec3 light_dir, float light_dist) {
	p -= light_dir * 0.5;
	float res = 1.0, d = 0;
	float i_factor = 60.0;
	for (int i = 0; i < 50 && d < light_dist; i++ ) {
		vec4 sp = scene(p);
		if ( sp.w < i_epsilon ) return 0.;
		float st = max(i_epsilon * d * 2, sp.w);
		d += st;
		res = min(res, i_factor * sp.w/d);
		p -= light_dir * st;
	}
	return res;
}

vec3 light(Ray r, vec4 c, vec3 light_pos) {
	vec3 light_dir = r.po - light_pos;
	float dist = length(light_dir);
	light_dir = normalize(light_dir);

	vec3 normal = n_at(r.po);
	float lambert_term = max(dot(-light_dir, normal), 0.0) * shadow(r.po, light_dir, dist);
	vec3 ocolor = c.rgb * lambert_term * i_light_color;
	return ocolor + c.rgb*i_ambient_light;
}

vec3 raymarch(Ray r){
	vec4 hit = cast_ray(r, i_max_dist);
	if(hit.r > -0.5) {
		return slight(r, hit);
	} else {
		return bg(r, hit);
	}
}

vec3 post(vec3 color, float exposure) {
	float Y = dot(vec3(0.30, 0.59, 0.11), color);
	return color * Y * exposure;
}