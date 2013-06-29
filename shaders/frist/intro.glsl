in vec2 p; /* screen position (-1, 1) */
in vec3 ro; /* ray origin */
in vec3 rd; /* ray dir */

out vec3 oc;



void main() {
	max_dist = 100.;
	Ray r;
	r.po = ro;
	r.d = normalize(rd);
	oc = raymarch(r, 1.5, 1);
	
	oc = post(oc, 10.0 + s*10.0);
}

const vec3 i_field_size = vec3(20);

vec4 scene(vec3 p) {
	vec4 ret;
	ret.rgb = vec3(0.7,0,0.7);
	ret.w = dbox(mod(p, i_field_size), vec3(10), vec3(6)) + 0.5*sin(t) * fbm(0.2*(p.xz), 1);
	float s = dsphere(p, vec3(sin(t * 1.2) * 10.0, 0., t + 10), 1.f);
	if(s < ret.w) {
		ret.w = s;
		ret.rgb = vec3(0.2, 0.8, 0.0);
	}
	return ret;
}

vec3 lpos() {
	if(t > 20.)	return vec3(sin(t*1.2)*50.0, 0, t + 2.0);
	else return ro + vec3(0., 0., 20.);
}

vec3 slight(Ray r, vec4 c) {
	return light(r, c, lpos()) + c.rgb * 0.1;
}

vec3 bg(Ray r, vec4 c) {
	return vec3(s2, s, s);
}