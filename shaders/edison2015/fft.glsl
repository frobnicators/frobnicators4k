#version 430

layout(std140, binding = 0) buffer a{
	vec2 T[];
};

layout(std430, binding = 1) buffer b{
	vec2 input[];
};

layout(std430, binding = 2) buffer c{
	vec2 outputb[];
};

unsigned int w;

vec2 cmul(vec2 c1, vec2 c2) {
	return vec2(c1.x*c2.x - c1.y * c2.y, c1.y * c2.x + c1.x * c2.y);
}

layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main() {
	uint gid = gl_GlobalInvocationID.x;
	uint s = gl_LocalInvocationID.y; // 0 or 1

	int p2w = int(pow(2, w));
	int p2w1 = int(pow(2, w+1));

	uint t_offset = -(1 - p2w) + gid%p2w1;

	uint row = gid / p2w;

	uint offset = 2*p2w * row + p2w*s + (gid % p2w);

	vec2 i1 = input[offset];

	uint index2 = s == 0 ? offset + p2w : offset - p2w;

	vec2 i2 = input[index2];

	vec2 res = i1 + cmul(i2, T[t_offset]);

	uint i = gid;// *2 + s;
	outputb[i].x = float(gl_LocalInvocationID.x);// 10.f + float(gid);// float(gid + 10);// res;
	outputb[i].y = float(gl_LocalInvocationID.y);//float(20+s);//
	//outputb[gid*2 + s].x = float(gid);
}