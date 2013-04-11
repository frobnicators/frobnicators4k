in vec2 p; /* screen position (-1, 1) */
in vec3 ro; /* ray origin */
in vec3 rd; /* ray dir */

out vec3 oc;

void main() {
	Ray r;
	r.po = ro;
	r.d = normalize(rd);
	oc = raymarch(r);
	post(oc, 1.0);
}

const vec3 i_field_size = vec3(50);


/*
vec4 scene(vec3 p) {
	/*float h = snoise(vec2(p.x, p.z)/100.0)*10.0
		+ snoise(vec2(p.x, p.z)/50.0)*5.0
		+ snoise(vec2(p.x, p.z)/20.0)*1
		+ snoise(vec2(p.x, p.z)/5.0)*0.1;
	float h = fbm(p.xz / 100.0, 6)*30.0;
	float floor_dist = p.y - (h - 50.0);

	return vec4(0.1, 0.8, 0.3, floor_dist);
}
*/
float pillar(vec3 p) {
	return min(
		dbox(p, vec3(0.0), vec3(1, 5, 1)) + 0.2 * clamp(fbm(p.xz*2.0, 1), 0, 1),
		dbox(p, vec3(0, 4, 0), vec3(1.5, 0.1, 1.5)) + 0.2 * clamp(fbm(p.xz*2.0, 1), 0, 1)
	);
}

vec4 scene(vec3 p) {
	float floor_dist = p.y + 3.0;
	vec4 ret = vec4(0.5);

	
	float column = pillar(rotateY(p, p.y * 0.5));
	if(column < floor_dist) {
		ret.rgb = vec3 (0.8235,0.7059,0.549)+ fbm(p.xy, 1) * 0.2;
	}

	ret.w = min(floor_dist, column);
	return ret;
}

vec3 slight(Ray r, vec4 c) {
	//return light(r, c, r.po + vec3(0, 50, 50));
//	return light(r, c, vec3(0));
	return light(r, c, vec3(sin(t*0.5)*10., 1, 1-cos(t*0.5))*10.);
}

vec3 bg(Ray r, vec4 c) {
	return vec3(0.1, 0.1, 0.8);
}