#pragma data_seg(push, ".shaders")
const char * _shaders[] = {
	{ "#version 430\nuniform float time; /* time */\nconst int ocean_size = 64;layout(std140, binding = 1) buffer ComputeOutput {\n	vec4 colors[];};" },
	{ "\nlayout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;void main() {\n	uvec2 gid = gl_GlobalInvocationID.xy;	uint index = gid.y * 1024 + gid.x;	colors[index] = vec4(gid.x / 1024.f, 0.f, gid.y / 1024.f, 1.f);}" },
	{ "\nlayout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;void main() {\n	uvec2 gid = gl_GlobalInvocationID.xy;	uint index = gid.y * 1024 + gid.x;	colors[index] = vec4(gid.x / 1024.f, 0.f, gid.y / 1024.f, 1.f);}" },
	{ "in vec2 p; /* screen position (-1, 1) */\nout vec3 oc;void main() {\n	vec2 uv = (p + 1.)/2.;	vec2 x = uv*float(ocean_size);	vec2 index_v;	vec2 blend = modf(x, index_v);	ivec2 index = ivec2(index_v.x,index_v.y);	vec3 color = colors[index.y * ocean_size + index.x].rgb;	oc = color;}" },
	{ "layout (location=0) in vec2 ip;out vec2 p;out vec3 ro;out vec3 rd;void main(){\n	float speed = 1.;	if(time > 33.) speed = 2.;	if(time > 58.) speed = 5.;	if(time > 65.) speed = 7.;	\n	p = ip;	gl_Position =  vec4(ip, 0.0, 1.0);	vec3 r = gl_Position.xyz * vec3(1.78, 1.0, 0) + vec3(0, 0, -1);	ro = vec3(0, 0, -5+time*speed);	rd = vec3(r.x, r.y, -r.z);}" },
};
#pragma data_seg(pop)
