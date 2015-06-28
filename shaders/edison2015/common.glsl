#version 430

uniform float time; /* time */
layout(binding = 0)  uniform sampler2D texture0;

const int ocean_size = 128;

layout(std140, binding = 1) buffer ComputeOutput {
	vec4 colors[];
};
