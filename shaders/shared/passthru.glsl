in vec2 p; /* screen position (-1, 1) */
layout(binding=0) uniform sampler2D t;

out vec3 oc;

void main() {
	vec2 uv = (p + 1.)/2.;

	oc = texture(t, uv).rgb;
}