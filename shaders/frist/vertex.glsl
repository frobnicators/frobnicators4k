layout (location=0) in vec2 ip;

out vec2 p;
out vec3 c; /* camera position*/

void main(){
	p = ip;
	gl_Position =  vec4(ip, 0.0, 1.0);
}