in vec2 pos;

out vec3 color;

void main() {
	color = vec3((pos.x + 1.0)/2.0, (pos.y + 1.0)/2.0, mod(time/10.0, 1.0));
}