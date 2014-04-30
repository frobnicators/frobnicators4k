in vec2 p; /* screen position (-1, 1) */
in vec3 ro; /* ray origin */
in vec3 rd; /* ray dir */

uniform float syc; /* sync */
uniform float s2;

out vec3 oc;

const float i_heights_at = 50.;

void main() {
	max_dist = 100.;
	Ray r;
	r.po = ro;
	r.d = normalize(rd);
	
	float ref = 2.0;
	int bounce = 0;
	if(time < i_heights_at || time > 87) { ref = 1.5; bounce = 1; }
	if(time >= i_heights_at) max_dist = 500.;
	oc = rym(r, ref, bounce);
}

const vec3 i_field_size = vec3(20);

vec4 scene(vec3 p) {
	if(time < i_heights_at) {
		return vec4(0.7,0,0.7, db(mod(p, i_field_size), vec3(10), vec3(6)) + 0.5*sin(time) * fbm(0.2*(p.xz), 1));
	} else {
		return vec4(0.1, 0.8, 0.3, p.y +50. - fbm(p.xz / 100.0, 6)*30.0);
	}
}

vec3 lpos(Ray r) {
	if( time < i_heights_at) {
		return ro + vec3(0., 0., 20.);
	} else {
		return r.po + vec3(0, 50-syc*20.0, 50);
	}
}

vec3 slight(Ray r, vec4 c) {
	return light(r, c, lpos(r)) + c.rgb * 0.1;
}

vec3 bg(Ray r, vec4 c) {
	if(time < i_heights_at) {
		return vec3(s2, syc, syc);
	} else {
		float frob = clamp(sin(time), 0.,1.);
		return vec3(
		p.x,p.y, 0)*0.5 + vec3(syc*frob, s2, syc*(1-frob));
	}
}