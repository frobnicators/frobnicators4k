in vec2 p; /* screen position (-1, 1) */
in vec3 ro; /* ray origin */
in vec3 rd; /* ray dir */

out vec3 oc;

void main() {
max_dist = 500.;
	Ray r;
	r.po = ro;
	r.d = normalize(rd);
	oc = raymarch(r, t > 87 ? 1.5 : 2.0, t > 87 ? 1 : 0);

	oc = post(oc, 3.0);
}

vec4 scene(vec3 p) {
	float h = fbm(p.xz / 100.0, 6)*30.0;
	float floor_dist = p.y - (h - 50.0);

	return vec4(0.1, 0.8, 0.3, floor_dist);
}

vec3 slight(Ray r, vec4 c) {
	return light(r, c, r.po + vec3(0, 50-s*20.0, 50));
}

vec3 bg(Ray r, vec4 c) {
	float frob = clamp(sin(t), 0.,1.);
	return vec3(p.x,p.y, 0)*0.5 + vec3(s*frob, s2, s*(1-frob));
}