#version 430

layout(std430, binding = 0) buffer a{
	vec2 T[];
};

// Bit reversal buffer
layout(std430, binding = 1) buffer b{
	uint R[];
};

layout(std430, binding = 2) buffer c{
	vec2 inputb[];
};

layout(std430, binding = 3) buffer d{
	vec2 outputb[];
};

uniform ivec2 wi; // w & invert
uniform ivec2 os; // offset & stride

vec2 cmul(vec2 c1, vec2 c2) {
	return vec2(c1.x*c2.x - c1.y * c2.y, c1.y * c2.x + c1.x * c2.y);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 2) in;
void main() {
	uint gid = gl_GlobalInvocationID.x;
	uint line = gl_GlobalInvocationID.y;
	uint s = gl_LocalInvocationID.z; // 0 or 1

	int p2w = int(pow(2.f, wi.x));

	uint t_offset = gid%p2w;

	uint row = gid / p2w;

	uint offset = 2 * p2w * row + p2w*s + (gid % p2w);
	uint index1 = offset;
	uint tmp = offset;
	uint index2 = s == 0 ? offset + p2w : offset - p2w;
	if (wi.y == 1) {
		index1 = R[index1];
		index2 = R[index2];
	}
	offset = offset * os.y + os.x*line;
	index1 = index1 * os.y + os.x*line;
	index2 = index2 * os.y + os.x*line;

	vec2 i1 = inputb[index1];
	vec2 i2 = inputb[index2];

	vec2 res;
	if (s == 0) {
		res = i1 + cmul(i2, T[t_offset]);
	} else {
		res = i2 - cmul(i1, T[t_offset]);
	}

	//outputb[offset] = vec2(offset, index2);
//	outputb[offset] = T[t_offset];
	outputb[offset] = res;
	//outputb[offset] = vec2(tmp, R[tmp]);
}