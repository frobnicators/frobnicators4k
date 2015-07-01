#version 430

layout (location = 0) in vec4 v;
layout(location = 1) in int i; //index into buffers

out vec3 p;
out vec3 n;
out vec3 cd; // camera dir

uniform mat4 PV; // projection matrix
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
	vec4 pos = v;
	pos.y += ocean_value.a;
	pos.xz += d;

	p = pos.xyz;

	gl_Position = PV*pos;

	n = ocean_value.rgb;

	vec4 tmp = PV * vec4(0.f, 0.f, 1.f, 0.f);
	cd = tmp.xyz / tmp.w;
}