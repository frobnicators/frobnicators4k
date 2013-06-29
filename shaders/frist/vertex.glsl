layout (location=0) in vec2 ip;

out vec2 p;
out vec3 ro;
out vec3 rd;

mat3 rmat(float angle) {
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	return mat3(
		c, - s,	0,
		s, c, 0,
		0, 0, oc + c
	);
}

void cam(out vec3 c[4]) {
	mat3 r = mat3(1.);
	float speed = 1.;
	if(t > 5.) r = rmat((t-5.)*0.1);
	if(t > 33.) { r = mat3(1.); speed = 2.; }
	if(t > 58.) speed = 5.;
	if(t > 65.) speed = 7.;
	c = vec3[4](
		vec3(0, 0, -5+t*speed),
		r * vec3(1, 0, 0),
		r * vec3(0, 1, 0),
		r * vec3(0, 0, -1)
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