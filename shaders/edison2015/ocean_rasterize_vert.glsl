#version 430

layout (location = 0) in vec4 v;
layout(location = 1) in int i; //index into buffers

noperspective out vec3 n;
noperspective out float h;

uniform mat4 M; // projection matrix

layout(std140, binding = 0) buffer a { // ocean data
	vec4 ov[];
};

layout(std140, binding = 1) buffer b { // displacement
	vec2 dp[];
};

void main() {
	vec4 ocean_value = ov[i];
	vec2 d = dp[i];
	vec4 pos = vec4(v.xy + d, -ocean_value.a, 1.f);

	gl_Position =  M*pos;

	n = ocean_value.rgb;
	h = ocean_value.a;
}