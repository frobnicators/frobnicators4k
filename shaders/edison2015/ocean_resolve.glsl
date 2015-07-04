
layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;
void main() {
	uvec2 gid = gl_GlobalInvocationID.xy;
	uint index = gid.y * 1024 + gid.x;
	colors[index] = vec4(gid.x / 1024.f, 0.f, gid.y / 1024.f, 1.f);
}