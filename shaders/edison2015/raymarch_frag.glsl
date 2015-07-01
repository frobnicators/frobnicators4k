#define NEAR_Z 0.0f
#define FAR_Z 250.0f
#define STEP_SIZE 0.03f

uniform mat4 PV;

in vec2 p; /* screen position (-1, 1) */
in vec3 cp; /* camera position (worldspace) */
in vec3 cd; /* camera direction */

out vec4 oc;

/**
* Return camera origin given current time.
*/
vec3 camera_origin(){
	return vec3(10, 50, 100);
}

/**
* Return camera direction given current time.
*/
vec3 camera_direction(){
	return cd;
	/*
	const float t = time * 0.1;
	const float yaw = 1;
	const float pitch = -0.1;
	return vec3(-cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw));
	*/
}

/**
* Generate ray based on camera direction.
*/
vec3 generate_ray(vec3 dir){
	vec3 u = vec3(0, 1, 0);
	vec3 r = cross(u, dir);
	return normalize(u * p.y + r * p.x + dir);
}

void main() {
	vec2 uv = (p + 1.)/2.;

	vec3 d = camera_direction();                             /* camera direction */
	vec3 ro = camera_origin();                                /* ray origin */
	vec3 rd = generate_ray(d);                                /* ray direction */

	vec3 hit;
	float h;
	//if (terrainmarch(ro, rd, hit, h)){
	//	oc = ocean_color(ro, rd, hit, h);
	//}
	//else {
	//}
	//oc = texture(texture0, uv).aaa;

	oc.rgb = sky_color(ro, rd);
	oc.a = 1.f;
	vec4 pv_hit = PV * vec4(hit, 1.f);
	gl_FragDepth = (pv_hit.z / pv_hit.w);
}