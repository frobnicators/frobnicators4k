in vec2 p; /* screen position (-1, 1) */

out vec3 oc;

void main() {
	vec2 uv = (p + 1.)/2.;
	vec2 x = uv*float(ocean_size);
	vec2 index_v;
	vec2 blend = modf(x, index_v);
	ivec2 index = ivec2(index_v.x,index_v.y);
	vec3 color = colors[index.y * ocean_size + index.x].rgb;
	oc = color;
}