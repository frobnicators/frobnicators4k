in vec2 p; /* screen position (-1, 1) */
in vec3 c; /* camera position*/

out vec3 oc;

void main() {
	oc = raymarch(vec3(0, 0, -10), p) * mix(vec3(1.0), vec3(0.4392,0.2588,0.0784), smoothstep(length(p), 0, 1));
}

const vec3 i_field_size = vec3(50);

float pillar(vec3 p) {
	return min(
		dbox(p, vec3(0.0), vec3(1, 5, 1)) + 0.2 * clamp(fbm(p*2.0), 0, 1),
		dbox(p, vec3(0, 4, 0), vec3(1.5, 0.1, 1.5)) + 0.2 * clamp(fbm(p*2.0), 0, 1)
	);
}

float scene(vec3 p, out vec3 color) {
	float floor_dist = p.y + 3.0;

	color = vec3(0.5);

	float column = pillar(rotateY(p, p.y * 0.5));
	if(column < floor_dist) {
		color = vec3 (0.8235,0.7059,0.549)+ fbm(p*5.0) * 0.2;
	}

	return min(floor_dist, column);
}

vec3 lpos() { /* Shall return light position */
	return vec3(-10);
	//return vec3(sin(t*2.)*2.0, 0, /*t +*/0.0)*10.;
	return vec3(sin(t*2.)*2.0, 0, /*t +*/0.0)*10.;
}