// h~0
layout(std430, binding = 5) buffer b6 {
	vec4 ht0[]; //xy: h0, zw: h0_mkconj
};

uniform float time;
uniform float oL; // ocean length;
uniform float dmp2; // dampening^2;

uniform float G; // ocean_g

float dispersion(int n, int m) {
	float w_0 = 2.f * M_PI / 200.f; // See Tessendorf, Jerry 2001
	// Todo: Merge with vector math:
	float kx = M_PI * (2 * n - N) / oL;
	float kz = M_PI * (2 * m - N) / oL;
	return floor(sqrt(G * sqrt(kx * kx + kz * kz)) / w_0) * w_0;
}

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main() {
	uvec2 gid = gl_GlobalInvocationID.xy;
	uint index = gid.y * N + gid.x;

	// Todo: Merge with vector math:
	vec2 k = vec2(M_PI * (2.f * gid.x - N) / oL, M_PI * (2.f * gid.y - N) / oL);
	float k_norm = length(k);

	// hTilde

	float omega_t = dispersion(int(gid.x), int(gid.y)) * time;

	vec2 c0 = vec2(cos(omega_t), sin(omega_t));

	vec4 ht0 = ht0[index];

	vec2 hTilde = cmul(ht0.xy, c0) + cmul(ht0.zw, conj(c0));
	ht[index] = hTilde;

	htsx[index] = cmul(hTilde, vec2(0.f, k.x));
	htsz[index] = cmul(hTilde, vec2(0.f, k.y));

	if (k_norm < dmp2) {
		htdx[index] = vec2(0.f);
		htdz[index] = vec2(0.f);
	} else {
		k /= k_norm;
		htdx[index] = cmul(hTilde, vec2(0.f, -k.x));
		htdz[index] = cmul(hTilde, vec2(0.f, -k.y));
	}
}