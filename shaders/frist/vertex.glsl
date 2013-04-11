layout (location=0) in vec2 ip;

out vec2 p;
out vec3 ro;
out vec3 rd;

void cam(out vec3 c[4]) {
	c = vec3[4](
		vec3(0, 0, -5),
		vec3(1, 0, 0),
		vec3(0, 1, 0),
		vec3(0, 0, -1)
	);
}

void main(){
	p = ip;
	gl_Position =  vec4(ip, 0.0, 1.0);
	vec3 r = gl_Position.xyz * vec3(1.7777, 1.0, 0) + vec3(0, 0, -1);
	vec3 c[4];
	cam(c);
	ro = c[0];
	rd.x = dot(r, c[1]);
	rd.y = dot(r, c[2]);
	rd.z = dot(r, c[3]);
}