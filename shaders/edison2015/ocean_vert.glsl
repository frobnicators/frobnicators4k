#version 430

layout (location = 0) in vec4 v;
layout(location = 1) in int i; //index into buffers

out vec3 p;
out vec3 n;

uniform mat4 PV; // projection matrix
uniform mat4 M; // model matrix
uniform mat4 V;

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
	//pos.xz += d;

	// TODO: Model matrix
	mat3 normal_matrix = transpose(inverse(mat3(V)));

	p = pos.xyz;

	gl_Position = PV*pos;

	//n = normalize(normal_matrix * ocean_value.rgb);
	n = vec3(0.f, 1.f, 0.f);
}