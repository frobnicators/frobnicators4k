in vec2 p; /* screen position (-1, 1) */

out vec3 oc;

void main() {
	vec2 uv = (p + 1.)/2.;
	vec2 x = uv*1024.f;
	vec2 index_v;
	vec2 blend = modf(x, index_v);
	ivec2 index = ivec2(index_v.x,index_v.y);
	vec3 color = colors[index.y * 1024 + index.x].rgb;
	oc = color;
}