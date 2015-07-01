#version 430

in vec3 n;
in float h;

out vec4 o;

void main() {
	o.rgb = n;
	o.a = 1.f;// h;
}