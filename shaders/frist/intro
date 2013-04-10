in vec2 p; /* screen position (-1, 1) */
in vec3 c; /* camera position*/

out vec3 oc;

void main() {
	//oc = raymarch(vec3(0, 0, t*10.0), 	p);
	oc = raymarch(vec3(0, 0, t*10), p) * mix(vec3(1.0), vec3(0.4392,0.2588,0.0784), length(p)/length(vec2(1)));
}

const vec3 i_field_size = vec3(20);

float distance_sphere(vec3 eye, vec3 pos, float r){
	return length(eye-pos) - r;
}

float scene(vec3 p, out vec3 color) {
	color = vec3(0.7,0,0.7);
	return dbox(mod(p, i_field_size), vec3(10), vec3(6));
}

vec3 lpos() { /* Shall return light position */
//	return vec3(0);
	return vec3(sin(t*2.)*10.0, 0, t + 2.0)*10.;
}