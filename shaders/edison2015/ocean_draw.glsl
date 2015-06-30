#define NEAR_Z 0.0f
#define FAR_Z 250.0f
#define STEP_SIZE 0.03f

in vec2 p; /* screen position (-1, 1) */

out vec3 oc;

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
	const float t = time * 0.1;
	const float yaw = 1;
	const float pitch = -0.1;
	return vec3(-cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw));
}

/**
* Generate ray based on camera direction.
*/
vec3 generate_ray(vec3 dir){
	vec3 u = vec3(0, 1, 0);
	vec3 r = cross(u, dir);
	return normalize(u * p.y + r * p.x + dir);
}

/**
 * Given a (x,z) coordinate it returns the height of the ocean.
 */
float ocean_height(vec2 p){
	return texture2D(texture0, p * 0.001).a;
}

bool terrainmarch(vec3 ro, vec3 rd, out vec3 hit, out float h){
	for (float t = NEAR_Z; t < FAR_Z; t += STEP_SIZE){
		const vec3 p = ro + rd*t;
		h = ocean_height(p.xz);
		if (p.y < h ){
			hit = p;
			return true;
		}
	}
	return false;
}

vec3 sky_color(vec3 ro, vec3 rd){
	const vec3 sky   = vec3(0.32, 0.58, 0.74);
	const vec3 cloud = vec3(1.0, 0.95, 1.0);
	const vec2 sc = ro.xz + rd.xz*(1000.0 - ro.y) / rd.y;

	vec3 color = mix(sky * (1.0 - rd.y), cloud, 0.5*smoothstep(0.1f, 0.7f, cnoise(sc*0.001)));
	return mix(color, sky, pow(1.0 - max(rd.y, 0.0), 8.0));
}

vec3 ocean_color(vec3 ro, vec3 rd, vec3 hit, float h){
	vec3 normal = normalize(texture2D(texture0, hit.xz * 0.001).rgb);
	vec3 rd_out = reflect(rd, normal);
	return sky_color(hit, rd_out);
}

void main() {
	vec2 uv = (p + 1.)/2.;

	vec3 d = camera_direction();                             /* camera direction */
	vec3 ro = camera_origin();                                /* ray origin */
	vec3 rd = generate_ray(d);                                /* ray direction */

	vec3 hit;
	float h;
	if (terrainmarch(ro, rd, hit, h)){
		oc = ocean_color(ro, rd, hit, h);
	}
	else {
		oc = sky_color(ro, rd);
	}
	//oc = texture(texture0, uv).aaa;
}