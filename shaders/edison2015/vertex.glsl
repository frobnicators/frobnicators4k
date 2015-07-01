layout (location=0) in vec2 ip;

uniform mat4 PV;

out vec2 p;
out vec3 cp; // camera pos
out vec3 cd; // camera dir

void main(){
	p = ip;
	gl_Position =  vec4(ip, 0.0, 1.0);
	
	vec4 tmp = PV * vec4(0.f, 0.f, 0.f, 1.f);
	cp = tmp.xyz / tmp.w;
	tmp = PV * vec4(0.f, 0.f, 1.f, 0.f);
	cd = tmp.xyz / tmp.w;

}