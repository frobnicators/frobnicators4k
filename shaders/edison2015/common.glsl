#version 430

uniform float time; /* time */

const int ocean_size = 512;


layout(std140, binding = 1) buffer ComputeOutput {
	vec4 colors[];
};