layout (location=0) in vec2 ip;

out vec2 p;
out vec3 ro;
out vec3 rd;

void main(){
	float speed = 1.;
	if(time > 33.) speed = 2.;
	if(time > 58.) speed = 5.;
	if(time > 65.) speed = 7.;
	
	p = ip;
	gl_Position =  vec4(ip, 0.0, 1.0);
	vec3 r = gl_Position.xyz * vec3(1.78, 1.0, 0) + vec3(0, 0, -1);

	ro = vec3(0, 0, -5+time*speed);
	rd = vec3(r.x, r.y, -r.z);
}