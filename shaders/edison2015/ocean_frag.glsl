in vec3 p;
in vec3 n;

out vec4 o;

uniform vec3 cp;

const float specular_contribution = 0.15f;

const float n2 = 1.4; // water index

void main() {
	vec3 incident = normalize(p - cp);
	vec3 normal = normalize(n);
	float reflect_dot = dot(incident, normal);
	vec3 rd_out = incident - 2.f * reflect_dot * normal;
	vec3 reflect_color = sky_color(p, rd_out);

	// Fresnel equations:
	vec3 refraction = normalize(refract(incident, normal, 1.f/n2));
	float incident_cos = dot(incident, normal);
	float refract_cos = dot(refraction, normal);
	float reflectance = clamp(pow((incident_cos - n2*refract_cos) / (incident_cos + refract_cos), 2.f), 0.f, 1.f);

	float spec = clamp(reflect_dot, 0.f, 1.f)*2.f;

	vec3 refract_color = vec3(0.f, 0.3, 0.5);

	o.rgb = mix(refract_color, reflect_color, reflectance);

	// Specular:
	float d = clamp(dot(normal, lightd), 0.f, 1.f);
	o.rgb += d * lightc * spec;

	o.a = 1.f;
}