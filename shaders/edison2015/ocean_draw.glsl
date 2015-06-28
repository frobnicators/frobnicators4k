#define NEAR_Z 0.0f
#define FAR_Z 50.0f
#define STEP_SIZE 0.03f

in vec2 p; /* screen position (-1, 1) */

out vec3 oc;

/**
* Return camera origin given current time.
*/
vec3 camera_origin(){
	return vec3(time, 4, 0);
}

/**
* Return camera direction given current time.
*/
vec3 camera_direction(){
	const float t = time * 0.1;
	const float yaw = 0.1;
	const float pitch = -sin(t) * 3.1415;
	return vec3(cos(yaw)*cos(pitch), sin(yaw)*cos(pitch), sin(pitch));;
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

vec3 permute(vec3 x){
	return mod(((x*34.0) + 1.0)*x, 289);
}

float snoise(vec2 v){
	const vec4 C = vec4(0.211, 0.366, -0.577, 0.024);
	// First corner
	vec2 i = floor(v + dot(v, C.yy));
	vec2 x0 = v - i + dot(i, C.xx);

	// Other corners
	vec2 i1;
	i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	vec4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
		+ i.x + vec3(0.0, i1.x, 1.0));

	vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
	m = pow(m, vec3(4));

	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;
	m *= 1.79 - 0.85 * (a0*a0 + h*h);
	vec3 g;
	g.x = a0.x  * x0.x + h.x  * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}

vec3 sky_color(vec3 rd){
	return mix(vec3(0.8, 0.8, 1.0), vec3(0.9, 0.9, 0.9), clamp(snoise(rd.yz * 5), 0, 1));
}

vec3 ocean_color(vec3 ro, vec3 rd, vec3 hit, float h){
	return (texture2D(texture0, hit.xz * 0.001).rgb + 1.f) * 0.5;
	//return (h > 0 ? vec3(0, 0, 1) : vec3(-0.5, -0.5, -0.5)) * h;
}

void main() {
	vec2 uv = (p + 1.)/2.;
	vec3 d  = camera_direction();                             /* camera direction */
	vec3 ro = camera_origin();                                /* ray origin */
	vec3 rd = generate_ray(d);                                /* ray direction */

	vec3 hit;
	float h;
	if (terrainmarch(ro, rd, hit, h)){
		oc = ocean_color(ro, rd, hit, h);
	} else {
		oc = sky_color(rd);
	}

	/*
	vec2 x = uv*float(ocean_size);
	vec2 index_v;
	vec2 blend = modf(x, index_v);
	ivec2 index = ivec2(index_v.x,index_v.y);
	vec3 color = colors[index.y * ocean_size + index.x].rgb;
	oc = color;
	*/
}