in vec2 uv;

out vec3 color;

void main() {
	color = vec3(uv.x, uv.y, mod(time/10.0, 1.0));
}