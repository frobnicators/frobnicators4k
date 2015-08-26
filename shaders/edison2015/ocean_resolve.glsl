// ocean data
layout(std430, binding = 5) buffer b6{
	vec4 od[];
};

// displacement
layout(std430, binding = 6) buffer b7{
	vec2 dp[];
};

const float lambda = -0.5f;

const vec2 signs = vec2(1.f, -1.f);

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
	uvec2 gid = gl_GlobalInvocationID.xy;
	uint index = gid.y * N + gid.x;

	float sgn = signs[(gid.x + gid.y) & 1];

	vec2 htilde = ht[index] * sgn;
	vec2 htilde_slopex = htsx[index] * sgn;
	vec2 htilde_slopez = htsz[index] * sgn;
	vec2 htilde_dx = htdx[index] * sgn;
	vec2 htilde_dz = htdz[index] * sgn;

	vec3 normal = normalize(vec3(-htilde_slopex.x, 1.f, -htilde_slopez.x));

	od[index].xyz = normal;
	od[index].w = htilde.x;
	
	dp[index].x = htilde_dx.x * lambda;
	dp[index].y = htilde_dz.x * lambda;
}