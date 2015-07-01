in vec3 p;
in vec3 n;
in vec3 cd;

out vec4 o;

const vec3 water_tint = vec3(0.8, 1.0, 1.0);
const float specular_contribution = 0.15f;

void main() {
	vec3 normal = normalize(n);
	vec3 rd_out = reflect(-cd, normal);
	vec3 sky = sky_color(p, rd_out);
	o.rgb = sky*water_tint;
	o.a = 1.f;

	// Specular:
	float d = clamp(dot(normal, lightd), 0.f, 1.f);
	o.rgb += d * lightc * specular_contribution;

	//o.rgb = normal;
}