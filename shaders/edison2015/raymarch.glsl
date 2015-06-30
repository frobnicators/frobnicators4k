/*
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

vec3 ocean_color(vec3 ro, vec3 rd, vec3 hit, float h){
	vec3 normal = normalize(texture2D(texture0, hit.xz * TEXTURE_SCALE).rgb);
	//return .5f*(normal + 1.f);
	vec3 rd_out = reflect(rd, normal);
	return sky_color(hit, rd_out);
}

*/

vec3 sky_color(vec3 ro, vec3 rd){
	const vec3 sky   = vec3(0.32, 0.58, 0.74);
	const vec3 cloud = vec3(1.0, 0.95, 1.0);
	const vec2 sc = ro.xz + rd.xz*(1000.0 - ro.y) / rd.y;

	vec3 color = mix(sky * (1.0 - rd.y), cloud, 0.5*smoothstep(0.1f, 0.7f, cnoise(sc*0.001)));
	return mix(color, sky, pow(1.0 - max(rd.y, 0.0), 8.0));
}
