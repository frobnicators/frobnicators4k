#version 430

uniform float time; /* time */

layout(std140, binding = 1) buffer ComputeOutput {
	vec4 colors[];
};
