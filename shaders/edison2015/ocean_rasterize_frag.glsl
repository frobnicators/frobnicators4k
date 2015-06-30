#version 430

noperspective in vec3 n;
noperspective in float h;

out vec4 o;

void main() {
	o.rgb = n;
	o.a = h;
}