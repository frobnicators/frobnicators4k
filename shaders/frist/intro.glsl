in vec2 p; /* screen position (-1, 1) */
in vec3 ro; /* ray origin */
in vec3 rd; /* ray dir */

uniform float syc; /* sync */
uniform float s2;

out vec3 oc;

#define HEIGHTS_AT 50.

void main() {
	max_dist = 100.;
	Ray r;
	r.po = ro;
	r.d = normalize(rd);
	
	float ref = 2.0;
	int bounce = 0;
	if(t < HEIGHTS_AT || t > 87) { ref = 1.5; bounce = 1; }
	if(t >= HEIGHTS_AT) max_dist = 500.;
	oc = rym(r, ref, bounce);
}

const vec3 i_field_size = vec3(20);

vec4 scene(vec3 p) {
	if(t < HEIGHTS_AT) {
		return vec4(0.7,0,0.7, db(mod(p, i_field_size), vec3(10), vec3(6)) + 0.5*sin(t) * fbm(0.2*(p.xz), 1));
	} else {	
		float h = fbm(p.xz / 100.0, 6)*30.0;
		float floor_dist = p.y - (h - 50.0);

		return vec4(0.1, 0.8, 0.3, floor_dist);
	}
}

vec3 lpos(Ray r) {
	if( t< HEIGHTS_AT) {
		return ro + vec3(0., 0., 20.);
	} else {
		return r.po + vec3(0, 50-syc*20.0, 50);
	}
}

vec3 slight(Ray r, vec4 c) {
	return light(r, c, lpos(r)) + c.rgb * 0.1;
}

vec3 bg(Ray r, vec4 c) {
	if(t < HEIGHTS_AT) {
		return vec3(s2, syc, syc);
	} else {
		float frob = clamp(sin(t), 0.,1.);
		return vec3(p.x,p.y, 0)*0.5 + vec3(syc*frob, s2, syc*(1-frob));
	}
}