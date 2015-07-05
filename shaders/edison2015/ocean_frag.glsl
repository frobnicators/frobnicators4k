in vec3 p;
in vec3 n;

out vec4 o;

uniform vec3 cp;

const float specular_contribution = 0.15f;

const float n2 = 1.4; // water index

const vec3 SEA_BASE = vec3(0.1, 0.19, 0.22);
const vec3 SEA_WATER_COLOR = vec3(0.8, 0.9, 0.6);
const vec3 SEA_REFLECT_COLOR = vec3(0.6, 0.7, 0.8);

float diffuse(vec3 normal, vec3 l, float p) {
	return pow(dot(normal, l) * 0.4 + 0.6, p);
}

float specular(vec3 n, vec3 l, vec3 e, float s) {
	float nrm = (s + 8.0) / (3.1415 * 8.0);
	return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

void main() {

	vec3 incident = normalize(p - cp);
	vec3 normal = normalize(n);
	float reflect_dot = dot(incident, normal);
	vec3 rd_out = incident - 2.f * reflect_dot * normal;
	vec3 reflect_color = sky_color(p, rd_out);// *SEA_REFLECT_COLOR;

	// Fresnel equations:
	vec3 refraction = normalize(refract(incident, normal, 1.f/n2));
	float incident_cos = dot(incident, normal);
	float refract_cos = dot(refraction, normal);
	float reflectance = clamp(pow((incident_cos - n2*refract_cos) / (incident_cos + refract_cos), 2.f), 0.f, 1.f);

	float fresnel = 1.0 - max(dot(normal, -incident), 0.0);
	fresnel = pow(fresnel, 3.0) * 0.65;


	vec3 refract_color = SEA_BASE + diffuse(normal, lightd, 80.0) * SEA_WATER_COLOR * 0.12;

	o.rgb = mix(refract_color, reflect_color, fresnel);

	// Specular:
	//float d = clamp(dot(normal, lightd), 0.f, 1.f);
	//o.rgb += d * lightc * spec;

	o.a = 1.f;
}